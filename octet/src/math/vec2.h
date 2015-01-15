////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Vector class
//

namespace octet { namespace math {
  class mat4t;
  class vec4;

  class vec2 {
    static const char *Copyright() { return "Copyright(C) Andy Thomason 2011-2013"; }
    #if OCTET_SSE
      union {
        __m128 m;
        float v[4];
      };

      vec2(__m128 m) {
        this->m = m;
      }
    #else
      float v[2];
    #endif
  public:
    // default constructor: note does not initialize!
    vec2() {
    }

    // construct from four scalars
    vec2(float x, float y) {
      v[0] = x; v[1] = y;
      #if OCTET_SSE
        v[2] = v[3] = 0;
      #endif
    };

    vec2(float xyz) {
      v[0] = v[1] = xyz;
      #if OCTET_SSE
        v[2] = v[3] = 0;
      #endif
    }

    // index accessor [0] [1] ...
    float &operator[](int i) { return v[i]; }

    // constant index accessor
    const float &operator[](int i) const {
      return v[i];
    }

    // vector - scalar operators
    vec2 operator+(float r) const {
      return *this + vec2(r);
    }
    vec2 operator-(float r) const {
      return *this - vec2(r);
    }
    vec2 operator*(float r) const {
      return *this * vec2(r);
    }
    vec2 operator/(float r) const {
      return *this * vec2(recip(r));
    }

    // in-place vector operators
    vec2 &operator+=(const vec2 &r) {
      *this = *this + r;
      return *this;
    }

    vec2 &operator-=(const vec2 &r) {
      *this = *this - r;
      return *this;
    }

    vec2 &operator*=(const vec2 &r) {
      *this = *this * r;
      return *this;
    }

    // dot product
    float dot(const vec2 &r) const {
      return (*this * r).sum();
    }

    // make the length equal to 1
    vec2 normalize() const {
      return *this * lengthRecip();
    }

    // euclidean length of a vector
    float length() const {
      return sqrt(dot(*this));
    }

    // one over the euclidean length of a vector
    float lengthRecip() const {
      return rsqrt(dot(*this));
    }

    // length squared
    float squared() const {
      return dot(*this);
    }

    // access the floating point numbers
    float *get() { return &v[0]; }

    // get the floating point numbers
    const float *get() const { return &v[0]; }

    ////////////////////////////////////
    //
    // vec2 specific
    //

    vec2 operator+(const vec2 &r) const {
      #if OCTET_SSE
        return vec2(_mm_add_ps(m, r.m));
      #else
        return vec2(v[0]+r.v[0], v[1]+r.v[1]);
      #endif
    }

    vec2 operator-(const vec2 &r) const {
      #if OCTET_SSE
        return vec2(_mm_sub_ps(m, r.m));
      #else
        return vec2(v[0]-r.v[0], v[1]-r.v[1]);
      #endif
    }

    vec2 operator*(const vec2 &r) const {
      #if OCTET_SSE
        return vec2(_mm_mul_ps(m, r.m));
      #else
        return vec2(v[0]*r.v[0], v[1]*r.v[1]);
      #endif
    }

    vec2 operator/(const vec2 &r) const {
      #if OCTET_SSE
        return vec2(_mm_div_ps(m, r.m));
      #else
        return vec2(v[0]*r.v[0], v[1]*r.v[1]);
      #endif
    }

    vec2 operator-() const {
      #if OCTET_SSE
        return vec2(_mm_sub_ps(_mm_setzero_ps(), m));
      #else
        return vec2(-v[0], -v[1]);
      #endif
    }

    // minumum of two vectors
    vec2 min(const vec2 &r) const {
      #if OCTET_SSE
        return vec2(_mm_min_ps(m, r.m));
      #else
        return vec2(v[0] < r[0] ? v[0] : r[0], v[1] < r[1] ? v[1] : r[1]);
      #endif
    }

    // maximum of two vectors
    vec2 max(const vec2 &r) const {
      #if OCTET_SSE
        return vec2(_mm_max_ps(m, r.m));
      #else
        return vec2(v[0] >= r[0] ? v[0] : r[0], v[1] >= r[1] ? v[1] : r[1]);
      #endif
    }

    // make all values positive.
    vec2 abs() const {
      return vec2(octet::abs(v[0]), octet::abs(v[1]));
    }

    // access x
    float &x() {
      return v[0];
    }

    // access y
    float &y() {
      return v[1];
    }

    // get x
    float x() const {
      return v[0];
    }

    // get y
    float y() const {
      return v[1];
    }

    // sum of terms
    float sum() const {
      return v[0] + v[1];
    }

    // convert to a string (up to 4 strings can be included at a time)
    const char *toString(char *dest, size_t size) const
    {
      snprintf(dest, size, "[%f, %f]", v[0], v[1]);
      return dest;
    }
  };

  // dot product
  inline float dot(const vec2 &lhs, const vec2 &rhs) {
     return lhs.dot(rhs); 
  }

  // add terms
  inline float sum(const vec2 &lhs) {
     return lhs.sum(); 
  }

  // make length = 1
  inline vec2 normalize(const vec2 &lhs) {
     return lhs.normalize(); 
  }

  // comonent-wise min
  inline vec2 min(const vec2 &lhs, const vec2 &r) {
     return lhs.min(r); 
  }

  // comonent-wise max
  inline vec2 max(const vec2 &lhs, const vec2 &r) {
     return lhs.max(r); 
  }

  // euclidean length
  inline float length(const vec2 &lhs) {
     return lhs.length(); 
  }

  // 1/length
  inline float lengthRecip(const vec2 &lhs) {
     return lhs.lengthRecip(); 
  }

  // length squared
  inline float squared(const vec2 &lhs) {
     return lhs.squared(); 
  }

  // component-wise abs
  inline vec2 abs(const vec2 &lhs) {
     return lhs.abs(); 
  }

  inline vec2 operator+(float lhs, const vec2 &rhs) {
    return rhs + lhs;
  }

  inline vec2 operator-(float lhs, const vec2 &rhs) {
    return rhs - lhs;
  }

  inline vec2 operator*(float lhs, const vec2 &rhs) {
    return rhs * lhs;
  }

  inline vec2 operator/(float lhs, const vec2 &rhs) {
    return rhs / lhs;
  }

  // vec2p is a packed vec2
  #if OCTET_SSE
    class vec2p {
      float v[2];
    public:
      vec2p(const vec2 &in) { v[0] = in[0]; v[1] = in[1]; }
      vec2p(float x=0, float y=0) { v[0] = x; v[1] = y; }
      operator vec2() { return vec2(v[0], v[1]); }
      operator const vec2() const { return vec2(v[0], v[1]); }
    };
  #else
    typedef vec2 vec2p;
  #endif

  std::ostream &operator <<(std::ostream &os, const vec2 &rhs) {
    char tmp[256];
    os << rhs.toString(tmp, sizeof(tmp));
    return os;
  }
} }

