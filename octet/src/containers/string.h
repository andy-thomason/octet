////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
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

namespace octet { namespace containers {
  /// The string class is used to hold persistant text strings.
  ///
  /// Only use this class as a data member in another class. Do not pass strings as parameters
  /// but instead use const char * to pass strings around locally.
  ///
  /// This string class has the ability to perform a few common operations such as formatting
  /// and url encode/decode.
  ///
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
    /// Default constructor: empty string.
    string() { data_ = null_string(); }

    /// Copy a UTF8 C string
    string(const char *value) { data_ = null_string(); *this = value; }
    
    /// Copy of a UFT16 C string
    string(const wchar_t *value) { data_ = null_string(); *this = value; }
    
    /// Copy of another string
    string(const string& rhs) { data_ = null_string(); *this = rhs.c_str(); }
    
    /// Copy of a substring
    string(const char *value, unsigned size) { data_ = null_string(); set(value, size); }

    /// Free up memory used by the string.
    ~string() { release(); }

    /// Format a string using sprintf.
    ///
    /// Example
    ///
    ///     string my_path;
    ///     my_path.format("%s/%s.dat", path, filename);
    string &format(const char *fmt, ...) {
      release();
      va_list v;
      va_start(v, fmt);
      vformat(fmt, v);
      va_end(v);
      return *this;
    }

    string &printf(const char *fmt, ...) {
      va_list v;
      va_start(v, fmt);
      vformat(fmt, v);
      va_end(v);
      return *this;
    }

    void vformat(const char *fmt, va_list v) {
      #ifdef WIN32
        size_t cur_len = strlen(data_);
        int len = _vscprintf(fmt, v);
        if (len) {
          if (cur_len) {
            data_ = (char*)allocator::realloc(data_, cur_len+1, cur_len + len + 1);
            vsprintf_s(data_ + cur_len, len+1, fmt, v);
          } else {
            data_ = (char*)allocator::malloc(len+1);
            vsprintf_s(data_, len+1, fmt, v);
          }
        }
      #else
        char tmp[1024];
        vsnprintf(tmp, sizeof(tmp)-1, fmt, v);
        *this += tmp;
      #endif
    }

    /// Decode url strings - to turn them into filenames, for example.
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

    /// encode url strings - to turn them into URLs, for example
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

    // copy a utf8 string - unix, mac and the web.
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

    // copy utf16 unicode strings - microsoft & java
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

    /// copy another string
    string &operator=(const string& rhs) { data_ = null_string(); *this = rhs.c_str(); return *this; }

    /// copy a substring
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

    /// shorten a string to a new length
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

    /// compare two strings
    bool operator==(const char *rhs) const { return strcmp(data_, rhs) == 0; }
    /// compare two strings
    bool operator!=(const char *rhs) const { return strcmp(data_, rhs) != 0; }
    /// compare two strings
    bool operator<(const char *rhs) const { return strcmp(data_, rhs) < 0; }
    /// compare two strings
    bool operator>(const char *rhs) const { return strcmp(data_, rhs) > 0; }

    /// Append to a string. Note: it is generally better to use format.
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

    /// Insert a substring.
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

    /// Find a substring.
    int find(const char *rhs) const {
      char *d = strstr(data_, rhs);
      if (d) {
        return (int)(d - data_);
      }
      return -1;
    }

    /// Find the position of the extension in a file path.
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

    /// Find the position of a filename in a file path
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

    /// Number of bytes in a string. Note: this is not the number of characters.
    int size() { return (int)strlen(data_); }

    /// Get a C string from this string.
    const char *c_str() const { return data_; }
    /// Get a C string from this string.
    operator const char *() { return data_; }

    /// raw data access
    char *data() const {
      return data_;
    }

    /// Get/set a byte from the string.
    char &operator[](int index) { return data_[index]; }
    
    /// Get a byte from the string.
    char operator[](int index) const { return data_[index]; }

    /// python-style string split.
    ///
    /// Example.
    ///
    ///     dynarray<string> parts;
    ///     string my_csv = "100,fred,bert,harry";
    ///     my_csv.split(parts, ",")
    ///     // parts now contains four strings: "100", "fred", "bert", "harry"
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

    /// return true if the string is empty.
    bool empty() {
      return size() == 0;
    }
  };
} }
