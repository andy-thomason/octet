////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// visitor for writing xml files.
//

namespace octet { namespace resources {
  class binary_writer : public visitor {
    enum { debug = true };
    hash_map<void *, int> refs;
    int next_id;
    FILE *file;

    void write(const uint8_t *src, unsigned bytes) {
      //if (debug) log("%*swrite %08x bytes\n", get_depth()*2, "", bytes);
      fwrite(src, 1, bytes, file);
    }

    void write_int(int value) {
      if (debug) log("%*swrite %08x\n", get_depth()*2, "", value);
      uint8_t b[4] = { (uint8_t)value, (uint8_t)(value >> 8), (uint8_t)(value >> 16), (uint8_t)(value >> 24) };
      write(b, 4);
    }

    void write_atom(atom_t value) {
      if (debug) log("%*swrite %08x (%s)\n", get_depth()*2, "", value, app_utils::get_atom_name((atom_t)value));
      uint8_t b[4] = { (uint8_t)value, (uint8_t)(value >> 8), (uint8_t)(value >> 16), (uint8_t)(value >> 24) };
      write(b, 4);
    }

    void write_string(const char *value) {
      if (debug) log("%*swrite %s\n", get_depth()*2, "", value);
      write((const uint8_t*)value, (int)strlen(value)+1);
    }

  public:
    binary_writer(FILE *file) {
      if (debug) log("%*sbinary_writer\n", get_depth()*2, "");
      next_id = 1;
      this->file = file;

      fwrite("octet\r\n\x1a", 1, 8, file);
    }

    ~binary_writer() {
    }

    // dictionary entry
    bool begin_ref(void *ref, const char *sid, atom_t type) {
      if (debug) log("%*sbegin_ref %p %s %s\n", get_depth()*2, "", ref, sid, app_utils::get_atom_name(type));
      if (ref == NULL) {
        write_atom(atom_);
        write_string(sid);
        write_int(0);
        return false;
      } else {
        int &id = refs[ref];
        bool is_new = id == 0;

        if (is_new) {
          id = next_id++;
        }

        write_atom(type);
        write_string(sid);
        write_int(id);

        return is_new;
      }
    }

    // ordinary ref
    bool begin_ref(void *ref, atom_t sid, atom_t type) {
      if (debug) log("%*sbegin_ref %p %s %s\n", get_depth()*2, "", ref, app_utils::get_atom_name(sid), app_utils::get_atom_name(type));
      if (ref == NULL) {
        write_atom(atom_);
        write_atom(sid);
        write_int(0);
        return false;
      } else {
        int &id = refs[ref];
        bool is_new = id == 0;

        if (is_new) {
          id = next_id++;
        }

        write_atom(type);
        write_atom(sid);
        write_int(id);

        return is_new;
      }
    }

    // array entry
    bool begin_ref(void *ref, int index, atom_t type) {
      if (debug) log("%*sbegin_ref %p %d %s\n", get_depth()*2, "", ref, index, app_utils::get_atom_name(type));
      if (ref == NULL) {
        write_atom(atom_);
        write_int(0);
        return false;
      } else {
        int &id = refs[ref];
        bool is_new = id == 0;

        if (is_new) {
          id = next_id++;
        }

        write_atom(type);
        write_int(id);
        return is_new;
      }
    }

    void end_ref() {
      if (debug) log("%*send_ref\n", get_depth()*2, "");
      write_atom(atom_end_ref);
    }

    bool begin_agg(void *ref, atom_t sid, atom_t type) {
      write_atom(type);
      write_atom(sid);
      return true;
    }

    void end_agg() {
    }

    bool begin_refs(atom_t sid, int &size, bool is_dict) {
      if (debug) log("%*sbegin_refs sid=%s size=%d is_dict=%d\n", get_depth()*2, "", app_utils::get_atom_name(sid), size, is_dict);
      write_atom(sid);
      write_atom(atom_begin_refs);
      write_int(size);
      return true;
    }

    void end_refs(bool is_dict) {
      if (debug) log("%*send_refs\n", get_depth()*2, "");
      //write_atom(atom_end_refs);
    }

    void visit_bin(void *value, unsigned size, atom_t sid, atom_t type) {
      write_atom(type);
      write_atom(sid);
      write_int(size);
      write((const uint8_t*)value, size);
    }

    void visit_string(string &value, atom_t sid) {
      write_atom(atom_string);
      write_atom(sid);
      write_string(value);
    }

    void visit(int &value, atom_t sid) {
      write_int(sid);
      write_int(sizeof(value));
      write((const uint8_t*)value, sizeof(value));
    }

    void visit(unsigned &value, atom_t sid) {
      write_int(sid);
      write_int(sizeof(value));
      write((const uint8_t*)value, sizeof(value));
    }

    void visit(atom_t &value, atom_t sid) {
      write_int(sid);
      write_int(sizeof(value));
      write((const uint8_t*)value, sizeof(value));
    }

    void visit(vec4 &value, atom_t sid) {
      write_int(sid);
      write_int(sizeof(value));
      write((const uint8_t*)&value, sizeof(value));
    }

    void visit(mat4t &value, atom_t sid) {
      write_int(sid);
      write_int(sizeof(value));
      write((const uint8_t*)&value, sizeof(value));
    }
  };
} }

