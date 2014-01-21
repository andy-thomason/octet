////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// visitor for finding objects by name in the octet graph
//
// work in progress

namespace octet { namespace resources {
  /// Work in progress: Visitor to find objects by name.
  class url_finder : public visitor {
    dynarray<string> &url;
    int depth;
    int max_depth;
  public:
    url_finder(int depth_, int max_depth_, dynarray<string> &url_, dynarray<string> &response_) : url(url_), response(response_) {
      depth = depth_;
      max_depth = max_depth_;
      response.resize(0);
    }

    bool begin_ref(void *ref, const char *sid, atom_t type) {
      if (depth == max_depth) {
        return false;
      } else if (url[depth] == sid) {
        depth++;
        return true;
      } else {
        return false;
      }
    }

    bool begin_ref(void *ref, atom_t sid, atom_t type) {
      return begin_ref(ref, app_utils::get_atom_name(sid), type);
    }

    bool begin_ref(void *ref, int index, atom_t type) {
      if (depth == max_depth) {
        return false;
      } else if (atoi(url[depth]) == index) {
        depth++;
        return true;
      } else {
        return false;
      }
    }

    void end_ref() {
      depth--;
    }

    bool begin_refs(atom_t sid, int &size, bool is_dict) {
      if (depth == max_depth) {
        return false;
      } else if (url[depth] == app_utils::get_atom_name(sid)) {
        depth++;
        return true;
      } else {
        return false;
      }
    }

    void end_refs(bool is_dict) {
      depth--;
    }

    void visit_bin(void *value, size_t size, atom_t sid, atom_t type) {
      if (depth == max_depth) {
      }
    }
  };
}

