////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// visitor for fetching parts of the game world to a web browser
//

namespace octet {
  class http_writer : public visitor {
    char hex_digit(unsigned i) {
      return i < 10 ? i + '0' : i + 'a' - 10;
    }

    const char *to_hex(void *data, size_t size) {
      static char tmp[256 + 1];
      assert(size*2 < sizeof(tmp));
      for (size_t i = 0; i != size; ++i) {
        tmp[i*2+0 ] = hex_digit(((uint8_t*)data)[i] >> 4);
        tmp[i*2+1 ] = hex_digit(((uint8_t*)data)[i] & 0x0f);
      }
      tmp[size*2] = 0;
      return tmp;
    }

    dynarray<string> &url;
    dynarray<string> &response;
    int depth;
    int max_depth;

    string &next() {
      response.resize(response.size()+1);
      return response.back();
    }
  public:
    http_writer(int depth_, int max_depth_, dynarray<string> &url_, dynarray<string> &response_) : url(url_), response(response_) {
      depth = depth_;
      max_depth = max_depth_;
      response.resize(0);
    }

    bool begin_ref(void *ref, const char *sid, atom_t type) {
      if (depth == max_depth) {
        next().format("%s,ref,%s\n", sid, app_utils::get_atom_name(type));
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
      char tmp[32];
      sprintf(tmp, "%d", index);
      return begin_ref(ref, tmp, type);
    }

    void end_ref() {
      depth--;
    }

    bool begin_refs(atom_t sid, int number) {
      if (depth == max_depth) {
        next().format("%s,array\n", app_utils::get_atom_name(sid));
        return false;
      } else if (url[depth] == app_utils::get_atom_name(sid)) {
        depth++;
        return true;
      } else {
        return false;
      }
    }

    void end_refs() {
      depth--;
    }

    void visit_bin(void *value, size_t size, atom_t sid, atom_t type) {
      if (depth == max_depth) {
        switch (type) {
          case atom_int8: next().format("%s,int,%d\n", app_utils::get_atom_name(sid), *(int8_t*)value); break;
          case atom_int16: next().format("%s,int,%d\n", app_utils::get_atom_name(sid), *(int16_t*)value); break;
          case atom_int32: next().format("%s,int,%d\n", app_utils::get_atom_name(sid), *(int32_t*)value); break;
          case atom_uint8: next().format("%s,unsigned,%d\n", app_utils::get_atom_name(sid), *(uint8_t*)value); break;
          case atom_uint16: next().format("%s,unsigned,%d\n", app_utils::get_atom_name(sid), *(uint16_t*)value); break;
          case atom_uint32: next().format("%s,unsigned,%u\n", app_utils::get_atom_name(sid), *(uint32_t*)value); break;
          case atom_mat4t: next().format("%s,mat4t,%s\n", app_utils::get_atom_name(sid), ((mat4t*)value)->toString()); break;
          case atom_vec4: next().format("%s,vec4,%s\n", app_utils::get_atom_name(sid), ((vec4*)value)->toString()); break;
          case atom_atom: next().format("%s,atom,%s\n", app_utils::get_atom_name(sid), app_utils::get_atom_name(*(atom_t*)value)); break;
          default: {
            if (size <= 128) {
              next().format("%s,bin,%s\n", app_utils::get_atom_name(sid), to_hex(value, size));
            } else {
              next().format("%s,bin+,%d\n", app_utils::get_atom_name(sid), size);
            }
          } break;
        }
      }
    }
  };
}

