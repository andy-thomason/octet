////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
//
// HTTP server for debugging game code and building game editors.

namespace octet { namespace helpers {
  /// Class for exposing game object to web browsers.
  class http_server {
    enum { port = 8888 };
    int listen_socket;

    struct session {
      int client_socket;
    };

    // The information we are serving. ie. the game data.
    ref<resource_dict> dict;

    // client sessions active
    dynarray<session> sessions;

    // temporary buffer used for send/recieve
    dynarray<char> buf;

    void set_non_blocking(int socket) {
      unsigned long mode = 1;
      ioctlsocket(socket, FIONBIO, &mode);
    }

    void parse_http_request(session &s, char *p) {
      string header(p);

      dynarray<string> lines;
      lines.reserve(32);
      header.split(lines, "\n");
      if (lines.size() == 0) return;

      dynarray<string> line0;
      lines[0].split(line0, " ");
      if (line0.size() < 3) return;
      if (line0[0] != "GET") return;

      log("http get from: %s\n", line0[1].c_str());

      // /graph?operation=get_children&id=1
      dynarray<string> url;
      line0[1].split(url, "?");
      if (url.size() < 2) return;

      dynarray<string> ops;
      url[1].split(ops, "&");
      string id;
      string callback;
      bool get_children = false;
      for (unsigned i = 0; i != ops.size(); ++i) {
        dynarray<string> lhsrhs;
        ops[i].split(lhsrhs, "=");
        if (lhsrhs[0] == "operation") {
          get_children = lhsrhs[1] == "get_children";
        } else if (lhsrhs[0] == "id") {
          id = lhsrhs[1];
        } else if (lhsrhs[0] == "callback") {
          callback = lhsrhs[1];
        }
        //log("%s = %s\n", lhsrhs[0].c_str(), lhsrhs[1].c_str());
      }

      if (!get_children) return;

      //dynarray<string> id_parts;
      //id.split(id_parts, ".");

      dynarray<string> response;
      response.reserve(64);
      int max_depth = 5;
      http_writer writer(0, max_depth, response);
      response.resize(response.size()+1);
      response.back().format("%s([\n", callback.c_str());
      dict->visit(writer);
      response.resize(response.size()+1);
      response.back().format("])\n");

      // With HTTP 1.1 we can keep the connection open and respond to more
      // feeds without the overhead of a new connection.
      unsigned num_bytes = 0;
      for (unsigned i = 0; i != response.size(); ++i) {
        num_bytes += response[i].size();
      }

      string response_header;
      response_header.format(
        "HTTP/1.1 200 OK\n"
        "Content-Type: application/json; charset=UTF-8\n"
        "Content-Length: %d\n"
        "\n",
        num_bytes
      );

      send(s.client_socket, response_header.c_str(), response_header.size(), 0);

      for (unsigned i = 0; i != response.size(); ++i) {
        log("send: %s", response[i].c_str());
        send(s.client_socket, response[i].c_str(), response[i].size(), 0);
      }
    }

  public:
    void init(resource_dict *dict_) {
      dict = dict_;

      // create a socket to listen for connections
      listen_socket = (int)socket(AF_INET, SOCK_STREAM, 0);

      // bind the socket to a specific port
      sockaddr_in addr;
      memset(&addr, 0, sizeof(addr));
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = htonl(INADDR_ANY);
      addr.sin_port = htons(port);
      bind(listen_socket, (sockaddr *)&addr, sizeof(addr));

      // start listening for connections (only one connection at a time)
      listen(listen_socket, 1);

      // set the socket to non-blocking so we don't need to use a thread
      set_non_blocking(listen_socket);

      // 64k buffer
      buf.resize(0x10000);

      printf("connect a web browser to webui/index.html\n");
    }

    // called once per frame
    void update() {
      // establish new sessions
      int client_socket = (int)accept(listen_socket, 0, 0);
      if (client_socket >= 0) {
        log("http: new connection socket %d\n", client_socket);
        set_non_blocking(client_socket);
        session s;
        s.client_socket = client_socket;
        sessions.push_back(s);
      }

      // poll the sessions
      for (unsigned i = 0; i < sessions.size(); ++i) {
        session &s = sessions[i];
        int bytes = (int)recv(s.client_socket, &buf[0], (size_t)buf.size()-1, 0);
        if (bytes > 0) {
          log("http: recieved from %d\n", s.client_socket);
          buf[bytes] = 0;
          parse_http_request(s, &buf[0]);
        } else if (bytes == 0) {
          log("http: close connection %d\n", s.client_socket);
          closesocket(s.client_socket); 
          sessions.erase(i);
        }
      }
    }
  };
}}

