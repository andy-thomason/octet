////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// dynarray class
//

// this is a dummy class used to customise the placement new and delete
struct dynarray_dummy_t {};

// placement new operator, allows construction in-place at "place"
void *operator new(size_t size, void *place, dynarray_dummy_t x) { return place; }

// dummy placement delete operator, allows destruction at "place"
void operator delete(void *ptr, void *place, dynarray_dummy_t x) {}

// dynamic array class similar to std::vector
template <class item_t, class allocator_t=allocator> class dynarray {
  item_t *data_;
  typedef unsigned int_size_t;
  int_size_t size_;
  int_size_t capacity_;
  enum { min_capacity = 8 };
  void reset() {
    for (int_size_t i = 0; i != size_; ++i) {
      data_[i].~item_t();
    }
    if (data_) {
      allocator_t::free(data_, capacity_ * sizeof(item_t));
    }
    data_ = 0;
    size_ = 0;
    capacity_ = 0;
  }
public:
  dynarray() {
    data_ = 0;
    size_ = 0;
    capacity_ = 0;
  }

  dynarray(int_size_t size) {
    data_ = (item_t*)allocator::malloc(size * sizeof(item_t));
    size_ = capacity_ = size;
  }

  ~dynarray() {
    reset();
  }

  class iterator {
    int_size_t elem;
    dynarray *vec;
    friend class dynarray;
  public:
    iterator(dynarray *vec_, int_size_t elem_) : vec(vec_), elem(elem_) {}
    item_t *operator ->() { return &(*vec)[elem]; }
    item_t &operator *() { return (*vec)[elem]; }
    bool operator != (const iterator &rhs) const { return elem != rhs.elem; }
    void operator++() { elem++; }
    void operator--() { elem--; }
  };

  iterator begin() {
    return iterator(this, 0);
  }

  iterator end() {
    return iterator(this, size_);
  }
  
  iterator insert(iterator it, const item_t &new_item) {
    int_size_t old_length = size_;
    resize(size_+1);
    for (int_size_t i = old_length; i != it.elem; --i) {
      data_[i] = data_[i-1];
    }
    data_[it.elem] = new_item;
    return it;
  }

  iterator erase(iterator it) {
    for (int_size_t i = it.elem; i < size_-1; ++i) {
      data_[i] = data_[i+1];
    }
    resize(size_-1);
    return it;
  }
  
  void push_back(const item_t &new_item) {
    int_size_t old_length = size_;
    resize(size_+1);
    data_[old_length] = new_item;
  }
  
  item_t &operator[](int_size_t elem) { return data_[elem]; }
  const item_t &operator[](int_size_t elem) const { return data_[elem]; }
  
  int_size_t size() { return size_; }

  int_size_t capacity() { return capacity_; }

  void *data() { return data_; }
  
  void resize(int_size_t new_length) {
    bool trace = false; // hack this for detailed traces
    dynarray_dummy_t x;
    if (new_length >= size_ && new_length <= capacity_) {
      if (trace) printf("case 1: growing dynarray up to capacity_\n");
      int_size_t len = size_; // avoid aliases
      while (len < new_length) {
        new (data_ + len, x)item_t;
        ++len;
      }
      size_ = len;
    } else if (new_length > capacity_) {
      if (trace) printf("case 2: growing dynarray beyond capacity_\n");
      int_size_t new_capacity = capacity_ == 0 ? min_capacity : capacity_ * 2;
      while (new_capacity < new_length) new_capacity *= 2;

      reserve(new_capacity);

      // initialize the rest to default
      for (int_size_t i = size_; i < new_length; ++i) {
        new (data_ + i, x) item_t;
      }

      size_ = new_length;
    } else {
      if (trace) printf("case 3: shrinking dynarray\n");
      int_size_t len = size_; // avoid aliases
      while (len > new_length) {
        --len;
        data_[len].~item_t();
      }
      size_ = new_length;
      if (size_ == 0) reset();
    }
  }

  void reserve(int_size_t new_capacity) {
    if (new_capacity >= size_) {
      dynarray_dummy_t x;
      item_t *new_data = (item_t *)allocator_t::malloc(sizeof(item_t) * new_capacity);
      
      // initialize new data_ elements from old ones
      for (int_size_t i = 0; i != size_; ++i) {
        new (new_data + i, x) item_t(data_[i]);
        data_[i].~item_t();
      }

      // free up data_
      if (data_) {
        allocator_t::free(data_, capacity_ * sizeof(item_t));
      }

      data_ = new_data;
      capacity_ = new_capacity;
    }
  }

  item_t &back() {
    //assert(size_ != 0);
    return data_[size_ - 1];
  }

  void pop_back() {
    //assert(size_ != 0);
    size_--;
  }
};
