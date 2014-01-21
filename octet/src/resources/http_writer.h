////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// visitor for fetching the game world to a web browser
// This visitor writes the JSON format required by jquery.jstree.js

namespace octet { namespace resources {
  /// Visitor to serialize game data to JSON format for use by web browsers.
  class http_writer : public visitor {
    hash_map<void *, int> refs;
    int next_id;

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

    dynarray<string> &response;
    int depth;
    int max_depth;

    string &next() {
      response.resize(response.size()+1);
      return response.back();
    }
  public:
    /// Use as a visitor to generate response text for game data
    http_writer(int depth_, int max_depth_, dynarray<string> &response_) : response(response_) {
      depth = depth_;
      max_depth = max_depth_;
      response.resize(0);
    }

    bool begin_ref(void *ref, const char *sid, atom_t type) {
      if (depth == max_depth) {
        next().format("%*s{ \"data\": \"%s\" },\n", depth*2, "", sid);
        return false;
      } else {
        next().format("%*s{ \"data\": \"%s\", children: [\n", depth*2, "", sid);
        depth++;
        return true;
      }
    }

    bool begin_ref(void *ref, atom_t sid, atom_t type) {
      return begin_ref(ref, app_utils::get_atom_name(sid), type);
    }

    bool begin_ref(void *ref, int index, atom_t type) {
      if (depth == max_depth) {
        next().format("%*s{ \"data\": \"%d\",\n", depth*2, "", index);
        return false;
      } else {
        next().format("%*s{ \"data\": \"%d\", children: [\n", depth*2, "", index);
        depth++;
        return true;
      }
    }

    void end_ref() {
      depth--;
      next().format("%*s]},\n", depth*2, "");
    }

    bool begin_refs(atom_t sid, int &size, bool is_dict) {
      if (depth == max_depth) {
        next().format("%*s{ \"data\": \"%s\" },\n", depth*2, "", app_utils::get_atom_name(sid));
        return false;
      } else {
        next().format("%*s{ \"data\": \"%s\", children: [\n", depth*2, "", app_utils::get_atom_name(sid));
        depth++;
        return true;
      }
    }

    void end_refs(bool is_dict) {
      depth--;
      next().format("%*s]},\n", depth*2, "");
    }

    void visit_bin(void *value, size_t size, atom_t sid, atom_t type) {
      string data;
      char tmp[256];
      switch (type) {
        case atom_int8: data.format("%d", *(int8_t*)value); break;
        case atom_int16: data.format("%d", *(int16_t*)value); break;
        case atom_int32: data.format("%d", *(int32_t*)value); break;
        case atom_uint8: data.format("%d", *(uint8_t*)value); break;
        case atom_uint16: data.format("%d", *(uint16_t*)value); break;
        case atom_uint32: data.format("%u", *(uint32_t*)value); break;
        case atom_mat4t: data.format("%s", ((mat4t*)value)->toString(tmp, sizeof(tmp))); break;
        case atom_vec4: data.format("%s", ((vec4*)value)->toString(tmp, sizeof(tmp))); break;
        case atom_atom: data.format("%s", app_utils::get_atom_name(*(atom_t*)value)); break;
        default: {
          if (size <= 128) {
            data.format("%s", to_hex(value, size));
          } else {
            data.format("blob");
          }
        } break;
      }
      next().format("%*s{ \"data\": \"%s\", children: [\"%s\"] },\n", depth*2, "", app_utils::get_atom_name(sid), data.c_str());
    }
  };
} }

