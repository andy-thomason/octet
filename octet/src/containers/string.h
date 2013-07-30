////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// no-frills string class: holds an asciiz string
//
// example:
//
//   string my_string = "hello world";
//   printf("%s\n", my_string.c_str());
//

namespace octet {
  class string {
    char *data_;

    static char *null_string() { static char c; return &c; }

    void release() {
      if (data_ != null_string()) {
        allocator::free((void*)data_, size() + 1);
        data_ = null_string();
      }
    }
  public:
    string() { data_ = null_string(); }

    string(const char *value) { data_ = null_string(); *this = value; }
    string(const string& rhs) { data_ = null_string(); *this = rhs.c_str(); }

    ~string() { release(); }

    string &format(const char *fmt, ...) {
      release();
      va_list v;
      va_start(v, fmt);
      #ifdef WIN32
        int len = _vscprintf(fmt, v);
        if (len) {
          data_ = (char*)allocator::malloc(len+1);
          vsprintf_s(data_, len+1, fmt, v);
        }
      #else
        char tmp[1024];
        vsnprintf(tmp, sizeof(tmp)-1, fmt, v);
        *this = tmp;
      #endif
      return *this;
    }

    string &operator=(const char *value) {
      release();
      if (value) {
        size_t size = strlen(value);
        if (size) {
          data_ = (char*)allocator::malloc(size+1);
          memcpy((char*)data_, value, size+1);
        }
      }
      return *this;
    }

    string &set(const char *value, unsigned size) {
      release();
      if (value) {
        if (size) {
          data_ = (char*)allocator::malloc(size+1);
          memcpy((char*)data_, value, size);
          data_[size] = 0;
        }
      }
      return *this;
    }

    string &truncate(int new_len) {
      int size = (int)strlen(data_);
      if (new_len < size) {
        if (data_ == null_string()) {
          data_ = (char*)allocator::malloc(new_len+1);
        } else {
          data_ = (char*)allocator::realloc((void*)data_, size+1, new_len+1);
        }
        data_[new_len] = 0;
      }
      return *this;
    }

    bool operator==(const char *rhs) const { return strcmp(data_, rhs) == 0; }
    bool operator!=(const char *rhs) const { return strcmp(data_, rhs) != 0; }
    bool operator<(const char *rhs) const { return strcmp(data_, rhs) < 0; }
    bool operator>(const char *rhs) const { return strcmp(data_, rhs) > 0; }

    string &operator+=(const char *rhs) {
      if (rhs) {
        size_t data_size = strlen(data_);
        size_t rhs_size = strlen(rhs);
        if (data_ == null_string()) {
          data_ = (char*)allocator::malloc(data_size+rhs_size+1);
        } else {
          data_ = (char*)allocator::realloc(data_, data_size + 1, data_size+rhs_size+1);
        }
        memcpy(data_ + data_size, rhs, rhs_size+1);
      }
      return *this;
    }

    string &insert(unsigned pos, const char *rhs) {
      if (rhs) {
        size_t data_size = strlen(data_);
        size_t rhs_size = strlen(rhs);
        char *new_data = (char*)allocator::malloc(data_size+rhs_size+1);
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
        return (int)(d - data_);
      }
      return -1;
    }

    int extension_pos() const {
      int res = -1;
      for (const char *p = data_; *p; ++p) {
        char chr = *p;
        if (chr == '/' || chr == '\\') {
          res = -1;  // note  /usr/fred.jim/harry   has no extension
        } else if (chr == '.') {
          res = (int)(p - data_);
        }
      }
      return res;
    }

    int filename_pos() const  {
      int res = 0;
      for (const char *p = data_; *p; ++p) {
        char chr = *p;
        if (chr == '/' || chr == '\\') {
          res = (int)(p - data_ + 1);
        }
      }
      return res;
    }

    int size() { return (int)strlen(data_); }

    const char *c_str() const { return data_; }
    operator const char *() { return data_; }

    // python-style string split
    void split(dynarray<string> &result, const char *delimiter) {
      result.resize(0);
      char *cur = data_;
      unsigned delim_len = strlen(delimiter);
      for(;;) {
        char *next = strstr(cur, delimiter);
        if (!next) break;
        result.push_back(string());
        result.back().set(cur, next - cur);
        cur = next + delim_len;
      }
      result.push_back(string());
      result.back() = cur;
    }
  };
}
