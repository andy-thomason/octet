////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// no-frills string class: holds an asciz string
//

template <typename allocator_t> class chars {
  const char *data_;

  static const char *null_string() { return ""; }

  void release() {
    if (data_ != null_string()) {
      allocator_t::free((void*)data_, size() + 1);
      data_ = null_string();
    }
  }
public:
  chars() { data_ = null_string(); }

  chars(const char *value) { data_ = null_string(); *this = value; }

  ~chars() { release(); }

  chars &operator=(const char *value) {
    release();
    size_t size = strlen(value);
    if (size) {
      data_ = (char*)allocator_t::malloc(size+1);
      memcpy((char*)data_, value, size+1);
    }
    return *this;
  }

  size_t size() { return strlen(data_); }

  const char *c_str() { return data_; }
};

