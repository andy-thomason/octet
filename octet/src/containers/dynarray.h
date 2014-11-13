////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//


namespace octet { namespace containers {
  /// Dynamic array class similar to std::vector.
  ///
  /// Example
  ///
  ///     dynarray<int> my_array;
  ///     my_array.push_back(1);
  ///     my_array.push_back(2);
  ///     my_array.push_back(3);
  ///
  ///     // now treat the array like an ordinary array.
  ///     printf("%d\n", my_array[1]);
  ///
  /// Note: try to avoid making arrays of class types.
  ///
  ///     dynarray<int> ints;          // ok. int is well-behaved.
  ///     dynarray<mesh> meshes;       // bad! mesh contains other arrays.
  ///     dynarray<ref<mesh> > meshes; // ok. managed pointers to meshes.
  template <class item_t, class allocator_t=allocator, bool use_new_delete=true> class dynarray {
    item_t *data_;
    typedef unsigned int_size_t;

    // note we don't use size_t for these as we don't expect to use arrays > 4G and we care about performance!
    int_size_t size_;
    int_size_t capacity_;
    enum { min_capacity = 8 };

  public:
    /// Create a new, empty, dynamic array
    dynarray() {
      data_ = 0;
      size_ = 0;
      capacity_ = 0;
    }

    /// Create a new dynamic array of a certain size.
    dynarray(int_size_t size) {
      data_ = (item_t*)allocator::malloc(size * sizeof(item_t));
      size_ = capacity_ = size;
      if (use_new_delete) {
        dynarray_dummy_t x;
        for (int_size_t i = 0; i != size; ++i) {
          new (data_ + i, x)item_t;
        }
      }
    }

    /// Create a copy of a dynamic array.
    ///
    /// Note: this is very slow and will happen frequently in naive code.
    dynarray(const dynarray &rhs) {
      data_ = (item_t*)allocator::malloc(rhs.size_ * sizeof(item_t));
      size_ = capacity_ = rhs.size_;
      if (use_new_delete) {
        dynarray_dummy_t x;
        for (int_size_t i = 0; i != size_; ++i) {
          new (data_ + i, x)item_t(rhs.data_[i]);
        }
      } else {
        memcpy(data_, rhs.data_, rhs.size_ * sizeof(item_t));
      }
    }

    /// Destroy the array and its contents.
    ~dynarray() {
      reset();
    }

    /// iterator class for use with this dynamic array.
    ///
    /// Note: this is for STL compatibility. We recommend that you use code like this instead:
    ///
    ///     for (unsigned i = 0; i != array.size(); ++i) {
    ///       // access array[i]
    ///     }
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
      void operator++(int) { elem++; }
      void operator--(int) { elem--; }
    };

    /// iterator start for STL compatibility
    iterator begin() {
      return iterator(this, 0);
    }

    /// iterator end for STL compatibility
    iterator end() {
      return iterator(this, size_);
    }
  
    /// iterator insert for STL compatibility
    iterator insert(iterator it, const item_t &new_item) {
      int_size_t old_length = size_;
      resize(size_+1);
      for (int_size_t i = old_length; i != it.elem; --i) {
        data_[i] = data_[i-1];
      }
      data_[it.elem] = new_item;
      return it;
    }

    /// iterator erase for STL compatibility
    iterator erase(iterator it) {
      for (int_size_t i = it.elem; i < size_-1; ++i) {
        data_[i] = data_[i+1];
      }
      resize(size_-1);
      return it;
    }
  
    /// Erase an item; move subsequent items down to fill the gap.
    void erase(unsigned elem) {
      for (int_size_t i = elem; i < size_-1; ++i) {
        data_[i] = data_[i+1];
      }
      resize(size_-1);
    }

    /// Add an item at the back of the array.
    void push_back(const item_t &new_item) {
      int_size_t old_length = size_;
      resize(size_+1);
      data_[old_length] = new_item;
    }

    /// Get the last element in the array.
    item_t &back() const {
      assert(size_);
      return data_[size_-1];
    }

    /// Return true if the array is empty.
    bool empty() const {
      return size_ == 0;
    }
  
    /// Access an element in the array.
    item_t &operator[](size_t elem) { return data_[elem]; }

    /// Read an element in the array.
    const item_t &operator[](size_t elem) const { return data_[elem]; }
  
    /// Return number of elements in the array
    int_size_t size() const { return size_; }

    /// Return the number of elements in the array before we have to reallocate the memory
    int_size_t capacity() const { return capacity_; }

    /// Get a constant pointer to the first element of the array.
    const item_t *data() const { return data_; }

    /// Get a pointer to the first element of the array.
    item_t *data() { return data_; }
  
    /// Resize the array to make it bigger or smaller.
    void resize(size_t new_length) {
      bool trace = false; // hack this for detailed traces
      dynarray_dummy_t x;
      if (new_length >= size_ && new_length <= capacity_) {
        if (trace) printf("case 1: growing dynarray up to capacity_\n");
        if (use_new_delete) {
          int_size_t len = size_; // avoid aliases
          while (len < new_length) {
            new (data_ + len, x)item_t;
            ++len;
          }
        }
        size_ = (int_size_t)new_length;
      } else if (new_length > capacity_) {
        if (trace) printf("case 2: growing dynarray beyond capacity_\n");
        int_size_t new_capacity = ((int_size_t)new_length < size_ ? size_ : (int_size_t)new_length);

        if (new_length == size_ + 1) {
          // growing array by 1: round up to power of two.
          new_capacity = capacity_ == 0 ? min_capacity : capacity_ * 2;
          while (new_capacity < new_length) new_capacity *= 2;
        }

        reserve(new_capacity);

        if (use_new_delete) {
          // initialize the rest to default
          for (int_size_t i = size_; i < new_length; ++i) {
            new (data_ + i, x) item_t;
          }
        }

        size_ = (int_size_t)new_length;
      } else {
        if (trace) printf("case 3: shrinking dynarray\n");

        if (use_new_delete) {
          int_size_t len = size_; // avoid aliases
          while (len > new_length) {
            --len;
            data_[len].~item_t();
          }
        }
        size_ = (int_size_t)new_length;
        //if (size_ == 0) reset();
      }
    }

    /// Reserve an amount of memory to use with this array.
    /// Use this before you start a loop with push_back calls, for example.
    void reserve(int_size_t new_capacity) {
      if (new_capacity >= size_) {
        dynarray_dummy_t x;
        item_t *new_data = (item_t *)allocator_t::malloc(sizeof(item_t) * new_capacity);
      
        if (use_new_delete) {
          // initialize new data_ elements from old ones
          for (int_size_t i = 0; i != size_; ++i) {
            new (new_data + i, x) item_t(data_[i]);
            data_[i].~item_t();
          }
        }

        // free up data_
        if (data_) {
          allocator_t::free(data_, capacity_ * sizeof(item_t));
        }

        data_ = new_data;
        capacity_ = new_capacity;
      }
    }

    /// Shrink the size of the array by one.
    void pop_back() {
      assert(size_ != 0);
      size_--;
    }

    /// Reset the array to zero size, freeing up the data.
    /// This is not the same as resize(0)
    void reset() {
      if (use_new_delete) {
        for (int_size_t i = 0; i != size_; ++i) {
          data_[i].~item_t();
        }
      }
      if (data_) {
        allocator_t::free(data_, capacity_ * sizeof(item_t));
      }
      data_ = 0;
      size_ = 0;
      capacity_ = 0;
    }
  };

  inline void vformat(dynarray <char> &ary, const char *fmt, va_list v) {
    unsigned old_size = ary.size();
    #ifdef WIN32
      int len = _vscprintf(fmt, v);
      if (len) {
        if (old_size) {
          ary.resize(old_size + len);
          vsprintf_s(&ary[old_size-1], len+1, fmt, v);
        } else {
          ary.resize(len + 1);
          vsprintf_s(&ary[0], len+1, fmt, v);
        }
      }
    #else
      char tmp[1024];
      size_t len = vsnprintf(tmp, sizeof(tmp)-1, fmt, v);
      if (len) {
        if (old_size) {
          ary.resize((int)(old_size + len));
          strcpy(&ary[old_size-1], tmp);
        } else {
          ary.resize((int)len + 1);
          strcpy(&ary[0], tmp);
        }
      }
    #endif
  }

  inline void format(dynarray <char> &ary, const char *fmt, ...) {
    va_list v;
    va_start(v, fmt);
    vformat(ary, fmt, v);
    va_end(v);
  }

} }

