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

    // When dealing with windows or java, we will come across the less popular
    // utf16 encoding scheme. All other sources of text will likely be in UTF8, ANSI or shift-JIS
    // We use UTF8 internally as it is compact and popular.
    unsigned utf16_to_utf8(char *dest, const wchar_t *value) {
      unsigned num_bytes = 0;
      for (unsigned i = 0; value[i]; ++i) {
        unsigned unicode = value[i];

        // codes in the region 0x100000 to 0x10ffff are represented as two wchar_t values
        if ( (unicode & 0xfc00 ) == 0xd800 && (value[i+1] & 0xfc00 ) == 0xdc00 ) {
          unicode = 0x100000 | ( (unicode & 0x3ff) << 10 ) | (value[i+1] & 0x3ff);
          i++;
        }

        unsigned size =
          (unicode < 0x80) ? 1 :        // mostly english
          (unicode < 0x800) ? 2 :       // mostly european
          (unicode < 0x10000) ? 3 :     // mostly asian
          (unicode < 0x200000) ? 4 :    // extended unicode
          (unicode < 0x4000000) ? 5 : 6
        ;

        if (dest) {
          if (size == 1) {
            *dest++ = (char)unicode;
          } else {
            // todo: write a unit test for this
            unsigned bits_to_go = size * 6 - 6;
            unsigned top_bits = unicode >> bits_to_go;
            *dest++ = top_bits | ( 0xfc << ( 6 - size ));
            while (bits_to_go != 0) {
              unsigned bits = unicode >> bits_to_go;
              *dest++ = ( bits & 0x3f ) | 0x80;
              bits_to_go -= 6;
            }
          }
        } else {
          num_bytes += size;
        }
      }
      if (dest) {
        *dest = 0;
      }
      return num_bytes;
    }

    // return true if this is a hex digit.
    bool is_hex(char c, unsigned &digit) {
      unsigned d09 = (unsigned)c - '0';
      unsigned daf = (unsigned)(c & ~0x20) - 'A';
      if (d09 < 10) {
        digit = d09;
        return true;
      } else if (daf < 6) {
        digit = daf + 10;
        return true;
      } else {
        return false;
      }
    }

    char to_hex(unsigned digit) {
      return digit < 10 ? '0' + digit : 'A' - 10 + digit;
    }

    // return true if this is a hex digit.
    unsigned urldecode_impl(char *dest, const char *src) {
      unsigned num_bytes = 0;
      for (unsigned i = 0; src[i]; ++i) {
        unsigned digit0, digit1;
        char chr = src[i];
        if (chr == '%' && is_hex(src[i+1], digit0) && is_hex(src[i+2], digit1)) {
          chr = (char)(digit0 * 16 + digit1);
          i += 2;
        }
        if (dest) {
          dest[num_bytes] = chr;
        }
        num_bytes++;
      }
      if (dest) {
        dest[num_bytes] = 0;
      }
      return num_bytes;
    }

    // return true if this is a hex digit.
    unsigned urlencode_impl(char *dest, const char *src) {
      unsigned num_bytes = 0;
      for (unsigned i = 0; src[i]; ++i) {
        char chr = src[i];
        if (chr == ' ' || chr == '&' || chr == '<' ||  chr == '>') {
          if (dest) {
            dest[num_bytes+0] = '%';
            dest[num_bytes+1] = to_hex((chr >> 4) & 0x0f);
            dest[num_bytes+2] = to_hex((chr >> 0) & 0x0f);
          }
          num_bytes += 3;
        } else {
          if (dest) {
            dest[num_bytes] = chr;
          }
          num_bytes++;
        }
      }
      if (dest) {
        dest[num_bytes] = 0;
      }
      return num_bytes;
    }
  public:
    string() { data_ = null_string(); }

    string(const char *value) { data_ = null_string(); *this = value; }
    string(const wchar_t *value) { data_ = null_string(); *this = value; }
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

    // decode url strings - to turn them into filenames, for example
    string &urldecode(const char *value) {
      release();
      if (value) {
        unsigned size = urldecode_impl(0, value);
        if (size) {
          data_ = (char*)allocator::malloc(size+1);
          urldecode_impl(data_, value);
        }
      }
      return *this;
    }

    // encode url strings - to turn them into filenames, for example
    string &urlencode(const char *value) {
      release();
      if (value) {
        unsigned size = urlencode_impl(0, value);
        if (size) {
          data_ = (char*)allocator::malloc(size+1);
          urlencode_impl(data_, value);
        }
      }
      return *this;
    }

    // utf8 strings - unix, mac and the web
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

    // utf16 unicode strings - microsoft & java
    string &operator=(const wchar_t *value) {
      release();
      if (value) {
        unsigned size = utf16_to_utf8(0, value);
        if (size) {
          data_ = (char*)allocator::malloc(size+1);
          utf16_to_utf8(data_, value);
        }
      }
      return *this;
    }

    string &operator=(const string& rhs) { data_ = null_string(); *this = rhs.c_str(); return *this; }

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
      unsigned delim_len = (unsigned)strlen(delimiter);
      for(;;) {
        char *next = strstr(cur, delimiter);
        if (!next) break;
        result.push_back(string());
        result.back().set(cur, (int)(next - cur));
        cur = next + delim_len;
      }
      result.push_back(string());
      result.back() = cur;
    }
  };
}
