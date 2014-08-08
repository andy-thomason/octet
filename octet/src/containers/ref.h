////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Sharing "smart" pointer for reference counting classes
//
// These should only be used in long-lived containers, never on the stack.
//

namespace octet { namespace containers {
  /// The ref class is used to keep reference counter pointers to object.
  ///
  /// It should only be used as a data member in a class. Do not use ref as
  /// a parameter and only rarely as a stack variable.
  ///
  /// In all other cases just use a regular C++ pointer.
  ///
  /// Anything using a ref must implement the add_ref() and release() methods.
  /// These allow the ref to keep track of the number of uses of an object.
  ///
  /// Examples.
  ///
  ///     {
  ///       ref<mesh> my_mesh = new mesh(); // create a new mesh.
  ///       ...
  ///       my_mesh->get_num_vertices();    // use ref like a pointer
  ///       mesh *local_ptr = my_mesh;      // convert to an unmanaged pointer
  ///       ...
  ///     } // the mesh object will be freed here.
  
  template <class item_t, class allocator_t=allocator> class ref {
    // wrapped pointer to the object
    item_t *item;

  public:
    /// default initializer - empty pointer
    ref() {
      item = 0;
    }

    /// copy constructor - avoid using this if possible.
    ref(const ref &rhs) {
      item = rhs.item;
      if (item) item->add_ref();
    }

    /// initialize with new item - pointer then "owns" object
    ref(item_t *new_item) {
      if (new_item) new_item->add_ref();
      item = new_item;
    }

    /// replace item with new one - frees any old object
    const ref &operator=(const ref &rhs) {
      item_t *new_item = rhs.item;
      if (new_item) new_item->add_ref();
      if (item) item->release();
      item = new_item;
      return rhs;
    }

    /// replace item with new one - frees any old object
    item_t *operator=(item_t *new_item) {
      if (new_item) new_item->add_ref();
      if (item) item->release();
      item = new_item;
      return new_item;
    }

    /// get a pointer to the constant item.
    operator const item_t *() const { return item; }

    /// get a pointer to the item. Beware! do not store this pointer as the object may be destroyed.
    operator item_t *() { return item; }

    /// get a pointer to the item from a constant reference.
    operator item_t *() const { return item; }

    /// get a pointer to the item.
    item_t * operator ->() const { return item; }

    /// return true if the pointer is not NULL.
    operator bool() const { return item != 0; }

    /// destructor - remove a reference rom the object, may free the object.
    ~ref() {
      if (item) item->release();
      item = 0;
    }
  };
} }
