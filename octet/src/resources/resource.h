////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// generalised resource: materials, scenes, cameras, meshes etc.
//

typedef int atom_t;

class resource {
  atom_t name;
  int ref_count;
public:
  resource() {
    name = 0;
    ref_count = 0;
  }

  void add_ref() {
    ref_count++;
  }

  void release() {
    if (--ref_count) {
      delete this;
    }
  }

  void set_name(atom_t value) {
    name = value;
  }

  atom_t get_name() {
    return name;
  }

  // get a unique int for a string.
  static atom_t atom(const char *name) {
    static dictionary<int> *dict;
    static atom_t num_atoms = 1;
    // avoid C++ static thread lock
    if (!dict) dict = new dictionary<int>();
    if (dict->contains(name)) {
      return (*dict)[name];
    } else {
      return (*dict)[name] = num_atoms++;
    }
  }
};

