////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// visitor for writing xml files.
//

namespace octet {
  class binary_reader : public visitor {
    enum { debug = true };
    hash_map<void *, int> refs;
    dynarray<void *> id_to_ref;
    FILE *file;
    char tmp[256];

    void read(uint8_t *src, unsigned bytes) {
      //if (debug) app_utils::log("read %08x bytes\n", bytes);
      fread(src, 1, bytes, file);
    }

    int read_int() {
      uint8_t b[4];
      read(b, 4);
      int value = b[0] + (b[1] << 8) + (b[2] << 16) + (b[3] << 24);
      if (debug) app_utils::log("%*sread %08x\n", get_depth()*2, "", value);
      return value;
    }

    atom_t read_atom() {
      uint8_t b[4];
      read(b, 4);
      int value = b[0] + (b[1] << 8) + (b[2] << 16) + (b[3] << 24);
      if (debug) app_utils::log("%*sread %08x (%s)\n", get_depth()*2, "", value, app_utils::get_atom_name((atom_t)value));
      return (atom_t)value;
    }

    const char *read_string() {
      int nchars = 0;
      for(;;) {
        int c = fgetc(file);
        tmp[nchars] = c;
        if (c == 0) break;
        nchars += nchars != sizeof(tmp)-1;
      }
      if (debug) app_utils::log("%*sread %s\n", get_depth()*2, "", tmp);
      return tmp;
    }

    bool check_atom(atom_t sid) {
      if (!get_error()) {
        atom_t test = read_atom();
        app_utils::log("%*scheck_atom %s\n", get_depth()*2, "", app_utils::get_atom_name(sid));
        if (test != sid) {
          app_utils::log("error: expected %s\n", app_utils::get_atom_name(sid));
          set_error(true);
        }
      }
      return get_error();
    }

    bool check_size(unsigned size) {
      if (!get_error()) {
        int test = read_int();
        app_utils::log("%*scheck_size %d\n", get_depth()*2, "", size);
        if (test != (int)size) {
          app_utils::log("error: expected %d bytes\n", size);
          set_error(true);
        }
      }
      return get_error();
    }

    void *get_ref(int id) {
      app_utils::log("%*sget_ref %d/%d\n", get_depth()*2, "", id, id_to_ref.size());
      if (id == (int)id_to_ref.size()) {
        return NULL;
      } else if (id > (int)id_to_ref.size()) {
        app_utils::log("error: id overflow\n");
        set_error(true);
        return NULL;
      } else {
        return id_to_ref[id];
      }
    }

  public:
    binary_reader(FILE *file) {
      if (debug) app_utils::log("binary_reader\n");
      id_to_ref.reserve(256);
      id_to_ref.push_back(NULL);

      this->file = file;
      char tmp[8];
      fread(tmp, 1, sizeof(tmp), file);
      if (memcmp(tmp, "octet", 5)) {
        set_error(true);
      }
    }

    ~binary_reader() {
    }

    // this is a reader
    bool is_reader() {
      return true;
    }

    // register a reference after creating a new object
    void add_new_ref(void *ref) {
      id_to_ref.push_back(ref);
    }

    // aggregate read
    bool begin_ref(void *ref, atom_t sid, atom_t type) {
      if (check_atom(type) || check_atom(sid)) {
        return false;
      }
      return true;
    }

    bool begin_ref(void *ref, int index, atom_t type) { return false; }
    bool begin_ref(void *ref, const char *sid, atom_t type) { return false; }

    // read a regular reference
    bool begin_read_ref(void *&ref, atom_t &sid, atom_t &type) {
      type = read_atom();
      sid = read_atom();
      int id = read_int();
      ref = get_ref(id);
      if (debug) app_utils::log("%*sbegin_read_ref %p %s %s %d\n", get_depth()*2, "", ref, app_utils::get_atom_name(sid), app_utils::get_atom_name(type), id);
      return !get_error();
    }

    // read an array reference
    bool begin_read_ref(void *&ref, int index, atom_t &type) {
      if (debug) app_utils::log("begin_read_ref\n");
      type = read_atom();
      int id = read_int();
      ref = get_ref(id);
      if (debug) app_utils::log("%*sbegin_read_ref %p %d %s\n", get_depth()*2, "", ref, index, app_utils::get_atom_name(type), id);
      return !get_error();
    }

    // read a dictionary reference
    bool begin_read_ref(void *&ref, const char *&sid, atom_t &type) {
      type = read_atom();
      sid = read_string();
      if (debug) app_utils::log("%*sbegin_read_ref %s\n", get_depth()*2, "", sid);
      int id = read_int();
      ref = get_ref(id);
      return !get_error();
    }

    // read an aggregate
    bool begin_agg(void *ref, atom_t sid, atom_t type) {
      if (!check_atom(type) && !check_atom(sid)) {
        return true;
      }
      return false;
    }

    // finish reading an aggregate
    void end_agg() {
    }

    // begin reading a dynarray
    unsigned begin_read_dynarray(unsigned elem_size, atom_t &sid) {
      if (!check_atom(atom_dynarray) && !check_atom(sid)) {
        return (unsigned)read_int() / elem_size;
      }
      return 0;
    }

    // finish reading a dynarray
    void end_read_dynarray(void *ptr, unsigned bytes) {
      read((uint8_t*)ptr, bytes);
    }

    // called after visiting a new object
    void end_ref() {
      if (debug) app_utils::log("%*send_ref\n", get_depth()*2, "");
      check_atom(atom_end_ref);
    }

    // called before reading an array or dictionary
    bool begin_refs(atom_t sid, int &size, bool is_dict) {
      if (debug) app_utils::log("%*sbegin_refs %s\n", get_depth()*2, "", app_utils::get_atom_name(sid));
      if (!check_atom(sid) && !check_atom(atom_begin_refs)) {
        size = read_int();
        return true;
      }
      return false;
    }

    // called after reading an array or dictionary
    void end_refs(bool is_dict) {
      if (debug) app_utils::log("%*send_refs\n", get_depth()*2, "");
      //check_atom(atom_end_refs);
    }

    void visit_bin(void *value, unsigned size, atom_t sid, atom_t type) {
      if (debug) app_utils::log("%*svisit_bin %s %d\n", get_depth()*2, "", app_utils::get_atom_name(sid), size);
      if (!check_atom(type) && !check_atom(sid) && !check_size(size)) {
        read((uint8_t*)value, size);
      }
    }

    void visit_string(string &value, atom_t sid) {
      if (!check_atom(atom_string) && !check_atom(sid)) {
        value = read_string();
      }
    }

    void visit(int &value, atom_t sid) {
      if (!check_atom(sid) && !check_size(sizeof(value))) {
        value = read_int();
      }
    }

    void visit(unsigned &value, atom_t sid) {
      if (!check_atom(sid) && !check_size(sizeof(value))) {
        value = read_int();
      }
    }

    void visit(atom_t &value, atom_t sid) {
      if (!check_atom(sid) && !check_size(sizeof(value))) {
        value = read_atom();
      }
    }

    void visit(vec4 &value, atom_t sid) {
      if (!check_atom(sid) && !check_size(sizeof(value))) {
        read((uint8_t*)&value, sizeof(value));
      }
    }

    void visit(mat4t &value, atom_t sid) {
      if (!check_atom(sid) && !check_size(sizeof(value))) {
        read((uint8_t*)&value, sizeof(value));
      }
    }
  };
}

