////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// self deleting "smart" pointer (non-shared)
//

// This is a non-sharing "smart" pointer class
//
// If you have to use "new" you can use it to guarantee a "delete" when
// the smart pointer goes out of scope or is deleted.
//
// Smart pointers "own" objects which have been allocated with new:
// example:
//
// {
//   ptr<my_class> fred(new my_class());
//   fred->my_method();
// } <-- object gets deleted here
//
namespace octet {
  template <class item_t, class allocator_t=allocator> class ptr {
    // wrapped pointer to the object
    item_t *item;

    // prevent copying of the pointer
    ptr(ptr &) {}

  public:
    // default initializer - empty pointer
    ptr() {
      item = 0;
    }

    // initialize with new item - pointer then "owns" object
    ptr(item_t *new_item) {
      if (item) delete item;
      item = new_item;
    }

    // replace item with new one - frees any old object
    item_t *operator=(item_t *new_item) {
      if (item) delete item;
      item = new_item;
      return new_item;
    }

    // get a pointer to the constant item
    operator const item_t *() const { return item; }

    // get a pointer to the item. Beware! do not store or pass this pointer.
    operator item_t *() const { return item; }

    // get a pointer to the item
    item_t * operator ->() const { return item; }

    // destructor - frees object
    ~ptr() {
      if (item) delete item;
    }
  };
}
