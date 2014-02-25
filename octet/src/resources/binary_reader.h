////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// visitor for writing xml files.
//

namespace octet { namespace resources {
  /// The binary reader is a visitor that is used to load a binary file.
  /// The binary reader will use a factory to create new classes, providied the class is in classes.h
  class binary_reader : public visitor {
    enum { debug = true };
    hash_map<void *, int> refs;
    dynarray<void *> id_to_ref;
    FILE *file;
    char tmp[256];

    void read(uint8_t *src, size_t bytes) {
      //if (debug) log("read %08x bytes\n", bytes);
      fread(src, 1, bytes, file);
    }

    int read_int() {
      uint8_t b[4];
      read(b, 4);
      int value = b[0] + (b[1] << 8) + (b[2] << 16) + (b[3] << 24);
      if (debug) log("%*sread %08x\n", get_depth()*2, "", value);
      return value;
    }

    atom_t read_atom() {
      uint8_t b[4];
      read(b, 4);
      int value = b[0] + (b[1] << 8) + (b[2] << 16) + (b[3] << 24);
      if (debug) log("%*sread %08x (%s)\n", get_depth()*2, "", value, app_utils::get_atom_name((atom_t)value));
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
      if (debug) log("%*sread %s\n", get_depth()*2, "", tmp);
      return tmp;
    }

    bool check_atom(atom_t sid) {
      if (!get_error()) {
        atom_t test = read_atom();
        log("%*scheck_atom %s\n", get_depth()*2, "", app_utils::get_atom_name(sid));
        if (test != sid) {
          log("error: expected %s\n", app_utils::get_atom_name(sid));
          set_error(true);
        }
      }
      return get_error();
    }

    bool check_size(size_t size) {
      if (!get_error()) {
        int test = read_int();
        log("%*scheck_size %d\n", get_depth()*2, "", size);
        if (test != (int)size) {
          log("error: expected %d bytes\n", size);
          set_error(true);
        }
      }
      return get_error();
    }

    void *get_ref(int id) {
      log("%*sget_ref %d/%d\n", get_depth()*2, "", id, id_to_ref.size());
      if (id == (int)id_to_ref.size()) {
        return NULL;
      } else if (id > (int)id_to_ref.size()) {
        log("error: id overflow\n");
        set_error(true);
        return NULL;
      } else {
        return id_to_ref[id];
      }
    }

  public:
    /// Construct a binary reader for a file.
    binary_reader(FILE *file) {
      if (debug) log("binary_reader\n");
      id_to_ref.reserve(256);
      id_to_ref.push_back(NULL);

      this->file = file;
      char tmp[8];
      fread(tmp, 1, sizeof(tmp), file);
      if (memcmp(tmp, "octet", 5)) {
        set_error(true);
      }
    }

    /// Destroy the reader
    ~binary_reader() {
    }

    /// This function returns true to indicate that this is a reader
    /// The visitor will behave differently for readers and writers
    bool is_reader() {
      return true;
    }

    /// register a reference after creating a new object
    void add_new_ref(void *ref) {
      id_to_ref.push_back(ref);
    }

    /// Read an aggregate object such as a struct or array.
    bool begin_ref(void *ref, atom_t sid, atom_t type) {
      if (check_atom(type) || check_atom(sid)) {
        return false;
      }
      return true;
    }

    /// When loading a reference in an array, call this function
    bool begin_ref(void *ref, int index, atom_t type) { return false; }

    /// When loading a reference in a dictionary, call this function
    bool begin_ref(void *ref, const char *sid, atom_t type) { return false; }

    /// Read a regular reference embeded in a class.
    bool begin_read_ref(void *&ref, atom_t &sid, atom_t &type) {
      type = read_atom();
      sid = read_atom();
      int id = read_int();
      ref = get_ref(id);
      if (debug) log("%*sbegin_read_ref %p %s %s %d\n", get_depth()*2, "", ref, app_utils::get_atom_name(sid), app_utils::get_atom_name(type), id);
      return !get_error();
    }

    /// Read an array reference
    bool begin_read_ref(void *&ref, int index, atom_t &type) {
      if (debug) log("begin_read_ref\n");
      type = read_atom();
      int id = read_int();
      ref = get_ref(id);
      if (debug) log("%*sbegin_read_ref %p %d %s\n", get_depth()*2, "", ref, index, app_utils::get_atom_name(type), id);
      return !get_error();
    }

    /// Read a dictionary reference
    bool begin_read_ref(void *&ref, const char *&sid, atom_t &type) {
      type = read_atom();
      sid = read_string();
      if (debug) log("%*sbegin_read_ref %s\n", get_depth()*2, "", sid);
      int id = read_int();
      ref = get_ref(id);
      return !get_error();
    }

    /// Read an aggregate such as an array or struct.
    bool begin_agg(void *ref, atom_t sid, atom_t type) {
      if (!check_atom(type) && !check_atom(sid)) {
        return true;
      }
      return false;
    }

    /// finish reading an aggregate
    void end_agg() {
    }

    /// Begin reading a dynarray
    unsigned begin_read_dynarray(unsigned elem_size, atom_t &sid) {
      if (!check_atom(atom_dynarray) && !check_atom(sid)) {
        return (unsigned)read_int() / elem_size;
      }
      return 0;
    }

    /// finish reading a dynarray
    void end_read_dynarray(void *ptr, unsigned bytes) {
      read((uint8_t*)ptr, bytes);
    }

    /// called after visiting a new object
    void end_ref() {
      if (debug) log("%*send_ref\n", get_depth()*2, "");
      check_atom(atom_end_ref);
    }

    /// called before reading an array or dictionary
    bool begin_refs(atom_t sid, int &size, bool is_dict) {
      if (debug) log("%*sbegin_refs %s\n", get_depth()*2, "", app_utils::get_atom_name(sid));
      if (!check_atom(sid) && !check_atom(atom_begin_refs)) {
        size = read_int();
        return true;
      }
      return false;
    }

    /// called after reading an array or dictionary
    void end_refs(bool is_dict) {
      if (debug) log("%*send_refs\n", get_depth()*2, "");
      //check_atom(atom_end_refs);
    }

    /// Read a binary object. The contents are opaque.
    void visit_bin(void *value, size_t size, atom_t sid, atom_t type) {
      if (debug) log("%*svisit_bin %s %d\n", get_depth()*2, "", app_utils::get_atom_name(sid), size);
      if (!check_atom(type) && !check_atom(sid) && !check_size(size)) {
        read((uint8_t*)value, size);
      }
    }

    /// Read a string object.
    void visit_string(string &value, atom_t sid) {
      if (!check_atom(atom_string) && !check_atom(sid)) {
        value = read_string();
      }
    }

  };
} }

