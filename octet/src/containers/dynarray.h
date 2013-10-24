////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Dynamic array (like std::vector)
//
// example:
//
//   dynarray<int> my_array;
//   my_array.push_back(1);
//   my_array.push_back(2);
//   my_array.push_back(3);
//
//   // now treat the array like an ordinary array.
//   printf("%d\n", my_array[1]);


// dynamic array class similar to std::vector
namespace octet {
  template <class item_t, class allocator_t=allocator, bool use_new_delete=true> class dynarray {
    item_t *data_;
    typedef unsigned int_size_t;
    int_size_t size_;
    int_size_t capacity_;
    enum { min_capacity = 8 };

    dynarray(const dynarray &rhs) {
      // you can't do this at the moment!
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
      void operator++(int) { elem++; }
      void operator--(int) { elem--; }
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
  
    void erase(unsigned elem) {
      for (int_size_t i = elem; i < size_-1; ++i) {
        data_[i] = data_[i+1];
      }
      resize(size_-1);
    }
  
    void push_back(const item_t &new_item) {
      int_size_t old_length = size_;
      resize(size_+1);
      data_[old_length] = new_item;
    }

    item_t &back() const {
      return data_[size_-1];
    }

    bool is_empty() const {
      return size_ == 0;
    }
  
    item_t &operator[](int_size_t elem) { return data_[elem]; }
    const item_t &operator[](int_size_t elem) const { return data_[elem]; }
  
    int_size_t size() const { return size_; }

    int_size_t capacity() const { return capacity_; }

    void *data() const { return data_; }
  
    void resize(int_size_t new_length) {
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
        size_ = new_length;
      } else if (new_length > capacity_) {
        if (trace) printf("case 2: growing dynarray beyond capacity_\n");
        int_size_t new_capacity = new_length < size_ ? size_ : new_length;

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

        size_ = new_length;
      } else {
        if (trace) printf("case 3: shrinking dynarray\n");

        if (use_new_delete) {
          int_size_t len = size_; // avoid aliases
          while (len > new_length) {
            --len;
            data_[len].~item_t();
          }
        }
        size_ = new_length;
        //if (size_ == 0) reset();
      }
    }

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

    void pop_back() {
      //assert(size_ != 0);
      size_--;
    }

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

  // dumbarray:
  //   high performance vector does not use new and delete
  /*template <class item_t, class allocator_t=allocator> class dumbarray : public dynarray<item_t, allocator_t, false> {
  };*/
}

