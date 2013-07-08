////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// no-frills string class: holds an asciiz string
//
// example:
//
//   chars my_string = "hello world";
//   printf("%s\n", my_string.c_str());
//

template <typename allocator_t=allocator> class chars {
  char *data_;

  static char *null_string() { static char c; return &c; }

  void release() {
    if (data_ != null_string()) {
      allocator_t::free((void*)data_, size() + 1);
      data_ = null_string();
    }
  }
public:
  chars() { data_ = null_string(); }

  chars(const char *value) { data_ = null_string(); *this = value; }
  chars(const chars& rhs) { data_ = null_string(); *this = rhs.c_str(); }

  ~chars() { release(); }

  chars &operator=(const char *value) {
    release();
    if (value) {
      size_t size = strlen(value);
      if (size) {
        data_ = (char*)allocator_t::malloc(size+1);
        memcpy((char*)data_, value, size+1);
      }
    }
    return *this;
  }

  chars &truncate(int new_len) {
    int size = (int)strlen(data_);
    if (new_len < size) {
      if (data_ == null_string()) {
        data_ = (char*)allocator_t::malloc(new_len+1);
      } else {
        data_ = (char*)allocator_t::realloc((void*)data_, size+1, new_len+1);
      }
      data_[new_len] = 0;
    }
    return *this;
  }

  bool operator==(const char *rhs) const { return strcmp(data_, rhs) == 0; }
  bool operator!=(const char *rhs) const { return strcmp(data_, rhs) != 0; }
  bool operator<(const char *rhs) const { return strcmp(data_, rhs) < 0; }
  bool operator>(const char *rhs) const { return strcmp(data_, rhs) > 0; }

  chars &operator+=(const char *rhs) {
    if (rhs) {
      size_t data_size = strlen(data_);
      size_t rhs_size = strlen(rhs);
      if (data_ == null_string()) {
        data_ = (char*)allocator_t::malloc(data_size+rhs_size+1);
      } else {
        data_ = (char*)allocator_t::realloc(data_, data_size + 1, data_size+rhs_size+1);
      }
      memcpy(data_ + data_size, rhs, rhs_size+1);
    }
    return *this;
  }

  chars &insert(unsigned pos, const char *rhs) {
    if (rhs) {
      size_t data_size = strlen(data_);
      size_t rhs_size = strlen(rhs);
      char *new_data = (char*)allocator_t::malloc(data_, data_size+rhs_size+1);
      memcpy(new_data, data_, pos);
      memcpy(new_data + pos, rhs, rhs_size);
      memcpy(new_data + pos + rhs_size, data_, data_size - pos + 1);
      release();
      data_ = new_data;
    }
    return *this;
  }

  int find(const char *rhs) const {
    char *d = strstr(data_, rhs);
    if (d) {
      return d - data_;
    }
  }

  int extension_pos() const {
    int res = -1;
    for (const char *p = data_; *p; ++p) {
      char chr = *p;
      if (chr == '/' || chr == '\\') {
        res = -1;  // note  /usr/fred.jim/harry   has no extension
      } else if (chr == '.') {
        res = p - data_;
      }
    }
    return res;
  }

  int filename_pos() const  {
    int res = 0;
    for (const char *p = data_; *p; ++p) {
      char chr = *p;
      if (chr == '/' || chr == '\\') {
        res = p - data_ + 1;
      }
    }
    return res;
  }

  int size() { return strlen(data_); }

  const char *c_str() const { return data_; }
  operator const char *() { return data_; }
};

typedef chars<allocator> string;
