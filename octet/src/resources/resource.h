////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// generalised resource: materials, scenes, cameras, meshes etc.
//

namespace octet {
  #define OCTET_CLASS(X) class X;
  #include "classes.h"
  #undef OCTET_CLASS

  // this macro implements standard functions for each class
  #define RESOURCE_META(classname) \
    classname *get_##classname() { return this; } \
    atom_t get_type() { return atom_##classname; } \
    static atom_t get_type_static() { return atom_##classname; }

  class resource {
    int ref_count;

  public:
    resource() {
      ref_count = 0;
    }

    static resource *new_type(atom_t type);

    // called by an animation, script or RPC.
    virtual void set_value(atom_t sid, atom_t sub_target, atom_t component, float *value) {
    }

    virtual void visit(visitor &v) {
    }

    virtual atom_t get_type() {
      return atom_;
    }

    virtual ~resource() {
    }

    void add_ref() {
      ref_count++;
    }

    void release() {
      if (--ref_count == 0) {
        delete this;
      }
    }

    void *operator new (size_t size) { return allocator::malloc(size); }
    void operator delete (void *ptr, size_t size) { return allocator::free(ptr, size); }

    // casting and aggregation
    #define OCTET_CLASS(X) virtual X *get_##X() { return 0; }
    #include "classes.h"
    #undef OCTET_CLASS
  };
}

