////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Generic skeleton for platform development.
//
// Use this as a template for any new platforms.
//


#pragma warning(disable : 4996)
#pragma warning(disable : 4345)
#pragma warning(disable : 4530)

#undef WIN32
#define _WINDOWS_

#define OCTET_HOT

typedef float float_t;

#include "gl_skeleton.h"
#include "al_defs.h"

// include cross platform app helpers, such as texture loaders
#include "video_capture.h"
#include "app_common.h"

#define FIONBIO 1

inline void ioctlsocket(int socket, unsigned kind, unsigned long *up) {
}

#define AF_INET 0
#define SOCK_STREAM 0
#define INADDR_ANY 0

struct sockaddr {
};

struct sockaddr_in {
  uint16_t sin_family;
  uint16_t sin_port;
  struct {
    long s_addr;
  } sin_addr;
};

inline int socket(int domain, int type, int protocol) {
  return 0;
}

inline int accept(int s, void *addr, int *addrlen) {
  return 0;
}

inline int bind(int s, sockaddr *addr, size_t addrlen) {
  return 0;
}

inline long htonl(long i) {
  return i;
}

inline short htons(short i) {
  return i;
}

inline int send(int s, const void *data, size_t size, int flags) {
  return 0;
}

inline int recv(int s, void *data, size_t size, int flags) {
  return 0;
}

inline int listen(int s, int) {
  return 0;
}

inline int closesocket(int s) {
  return 0;
}


namespace octet {
  // this is the class that all apps are derived from.
  class app : public app_common {
    static app *get_the_app(app *the_app_init=0) {
      app *the_app;
      if (the_app_init) the_app = the_app_init;
      return the_app;
    }
  public:
    // constructor
    app(int argc, char **argv) {
      get_the_app(this);
    }

    // initialiser (it is nice to keep the two separate for aggregate memory allocation)
    void init() {
      set_viewport_size(512, 512);
      app_init();
    }

    void render() {
      //printf("render %d\n", glutGetWindow());
      int vx, vy;
      get_viewport_size(vx, vy);
      draw_world(0, 0, vx, vy);
      inc_frame_number();
    }

    ~app() {
    }

    static void init_all(int argc, char **argv) {
      gl_ctxt(new gl_context());
      //get_the_app()->init();
    }


    static void run_all_apps() {
      //get_the_app()->render();
    }

    static void error(const char *msg) {
      printf("%s - exiting\n", msg);
      exit(1);
    }
  };

}
