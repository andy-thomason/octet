////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// generalised resource: materials, scenes, cameras, meshes etc.
//
// This class is a base class for most of the allocatable objects in the engine.
// It provides automation for casts, saving and loading.

// this macro implements standard functions for each class
#define RESOURCE_META(classname) \
  classname *get_##classname() { return this; } \
  atom_t get_type() { return atom_##classname; } \
  static atom_t get_type_static() { return atom_##classname; }

namespace octet { namespace resources {
  /// Base class for resources; provides aligned allocation and reference counting.
  class resource {
    // how many lives do we have?
    int ref_count;

  public:
    /// Make a new resource with no lives.
    /// Adding it to a ref<> will give it a life.
    resource() {
      ref_count = 0;
    }

    /// factory for making new resources of various kinds
    /// used by readers
    static resource *new_type(atom_t type);

    /// Animation hook; called by an animation, script or RPC.
    virtual void set_value(atom_t sid, atom_t sub_target, atom_t component, float *value) {
    }

    /// visit the resource for saving, loading and script access
    virtual void visit(visitor &v) {
    }

    /// update the resource if anything has changed
    virtual void update() {
    }

    /// what kind of resource are we?
    virtual atom_t get_type() {
      return atom_;
    }

    /// destructors must be virtual or they may not get called!
    virtual ~resource() {
    }

    /// Give this resource an extra life; see the %ref class.
    void add_ref() {
      ref_count++;
    }

    /// Remove a life from this resource and delete it if it is dead; see the %ref class.
    void release() {
      if (--ref_count == 0) {
        delete this;
      }
    }

    /// use the allocator to allocate this resource and its child classes
    void *operator new (size_t size) {
      return allocator::malloc(size);
    }

    /// use the allocator to free this resource and its child classes
    void operator delete (void *ptr, size_t size) {
      return allocator::free(ptr, size);
    }

    // casting and aggregation: make a get_* function for each class
    // the RESOURCE_META macro overrides each of these once for every class.
    // use the get_* function for casting and checking types.
    #define OCTET_CLASS(N, X) virtual N::X *get_##X() { return 0; }
    //#pragma message("resource.h 2")
    #include "classes.h"
    #undef OCTET_CLASS
  };
} }

