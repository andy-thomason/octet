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
  class vec3;
  class vec4;
  class ivec3;
  class ivec4;

  // vec3: vector of 3 floats - optimial for computation, but not storage.
  class vec3 {
    static const char *Copyright() { return "Copyright(C) Andy Thomason 2011-2013"; }

    #if OCTET_SSE
      union {
        __m128 m;
        float v[4];
        int i[4];
      };
    #else
      float v[3];
    #endif
  public:
    vec3() {
      #if OCTET_SSE
        m = _mm_setzero_ps();
      #else
        v[0] = v[1] = v[2] = 0;
      #endif
    }

    // construct from four scalars
    explicit vec3(float x, float y, float z) {
      #if OCTET_SSE
        m = _mm_setr_ps(x, y, z, z);
      #else
        v[0] = x; v[1] = y; v[2] = z;
      #endif
    };

    // construct from four scalars
    /*explicit vec3(int x, int y, int z) {
      #if OCTET_SSE
        __m64 lo = _mm_set_pi32(y, x);
        __m64 hi = _m_from_int(z);
        m = _mm_cvtpi32x2_ps(lo, hi);
      #else
        v[0] = x; v[1] = y; v[2] = z;
      #endif
    };*/

    vec3(float xyz) {
      #if OCTET_SSE
        m = _mm_set_ps1(xyz);
      #else
        v[0] = v[1] = v[2] = xyz;
      #endif
    }

    #if OCTET_SSE
      vec3(__m128 m) {
        this->m = m;
      }

      vec3(const vec3 &rhs) {
        this->m = rhs.m;
      }
    #endif

    vec3(const ivec3 &rhs);

    // index accessor [0] [1] ...
    float &operator[](int i) { return v[i]; }

    // constant index accessor
    const float &operator[](int i) const {
      return v[i];
    }

    // vector - scalar operators
    vec3 operator+(float r) const {
      return *this + vec3(r);
    }
    vec3 operator-(float r) const {
      return *this - vec3(r);
    }
    vec3 operator*(float r) const {
      return *this * vec3(r);
    }
    vec3 operator/(float r) const {
      return *this * vec3(recip(r));
    }

    // in-place vector operators
    vec3 &operator+=(const vec3 &r) {
      *this = *this + r;
      return *this;
    }

    vec3 &operator-=(const vec3 &r) {
      *this = *this - r;
      return *this;
    }

    vec3 &operator*=(const vec3 &r) {
      *this = *this * r;
      return *this;
    }

    // dot product
    float dot(const vec3 &r) const {
      return (*this * r).sum();
    }

    // make the length equal to 1
    vec3 normalize() const {
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

    // cross product
    vec3 cross(const vec3 &r) const {
      #if OCTET_SSE
        __m128 lshuf = _mm_shuffle_ps(r.m, r.m, _MM_SHUFFLE(0, 0, 2, 1));
        __m128 rshuf = _mm_shuffle_ps(m, m, _MM_SHUFFLE(0, 0, 2, 1));
        __m128 lprod = _mm_mul_ps(m, lshuf);
        __m128 rprod = _mm_mul_ps(r.m, rshuf);
        __m128 sum = _mm_sub_ps(lprod, rprod);
        return vec3(_mm_shuffle_ps(sum, sum, _MM_SHUFFLE(0, 0, 2, 1)));
      #else
        return vec3(
          v[1] * r.v[2] - v[2] * r.v[1],
	        v[2] * r.v[0] - v[0] * r.v[2],
	        v[0] * r.v[1] - v[1] * r.v[0]
	      );
      #endif
    }

    // positive cross product (for box tests)
    vec3 abs_cross(const vec3 &r) const {
      #if OCTET_SSE
        __m128 lshuf = _mm_shuffle_ps(r.m, r.m, _MM_SHUFFLE(0, 0, 2, 1));
        __m128 rshuf = _mm_shuffle_ps(m, m, _MM_SHUFFLE(0, 0, 2, 1));
        __m128 lprod = _mm_mul_ps(m, lshuf);
        __m128 rprod = _mm_mul_ps(r.m, rshuf);
        __m128 sum = _mm_add_ps(lprod, rprod);
        return vec3(_mm_shuffle_ps(sum, sum, _MM_SHUFFLE(0, 0, 2, 1)));
      #else
        return vec3(
          v[1] * r.v[2] + v[2] * r.v[1],
	        v[2] * r.v[0] + v[0] * r.v[2],
	        v[0] * r.v[1] + v[1] * r.v[0]
	      );
      #endif
    }

    // access the floating point numbers
    float *get() { return &v[0]; }

    // get the floating point numbers
    const float *get() const { return &v[0]; }

    ////////////////////////////////////
    //
    // vec3 specific
    //

    vec3 operator+(const vec3 &r) const {
      #if OCTET_SSE
        return vec3(_mm_add_ps(m, r.m));
      #else
        return vec3(v[0]+r.v[0], v[1]+r.v[1], v[2]+r.v[2]);
      #endif
    }

    vec3 operator-(const vec3 &r) const {
      #if OCTET_SSE
        return vec3(_mm_sub_ps(m, r.m));
      #else
        return vec3(v[0]-r.v[0], v[1]-r.v[1], v[2]-r.v[2]);
      #endif
    }

    vec3 operator*(const vec3 &r) const {
      #if OCTET_SSE
        return vec3(_mm_mul_ps(m, r.m));
      #else
        return vec3(v[0]*r.v[0], v[1]*r.v[1], v[2]*r.v[2]);
      #endif
    }

    vec3 operator/(const vec3 &r) const {
      #if OCTET_SSE
        return vec3(_mm_div_ps(m, r.m));
      #else
        return vec3(v[0]*r.v[0], v[1]*r.v[1], v[2]*r.v[2]);
      #endif
    }

    vec3 operator-() const {
      #if OCTET_SSE
        return vec3(_mm_sub_ps(_mm_setzero_ps(), m));
      #else
        return vec3(-v[0], -v[1], -v[2]);
      #endif
    }

    // minumum of two vectors
    vec3 min(const vec3 &r) const {
      #if OCTET_SSE
        return vec3(_mm_min_ps(m, r.m));
      #else
        return vec3(v[0] < r[0] ? v[0] : r[0], v[1] < r[1] ? v[1] : r[1], v[2] < r[2] ? v[2] : r[2]);
      #endif
    }

    // maximum of two vectors
    vec3 max(const vec3 &r) const {
      #if OCTET_SSE
        return vec3(_mm_max_ps(m, r.m));
      #else
        return vec3(v[0] >= r[0] ? v[0] : r[0], v[1] >= r[1] ? v[1] : r[1], v[2] >= r[2] ? v[2] : r[2]);
      #endif
    }

    // make all values positive.
    vec3 abs() const {
      #if OCTET_SSE
        static const union { unsigned v[4]; __m128 mask; } u = { 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff };
        return vec3(_mm_and_ps(m, u.mask));
      #else
        return vec3(octet::abs(v[0]), octet::abs(v[1]), octet::abs(v[2]));
      #endif
    }

    // get vec2
    vec2 xy() const {
      return vec2(v[0], v[1]);
    }

    // get vec4
    vec4 xyz0() const;
    vec4 xyz1() const;

    vec4 xxxx() const;
    vec4 yyyy() const;
    vec4 zzzz() const;

    vec3 xxx() const { 
      #if OCTET_SSE
        return vec3(_mm_shuffle_ps(get_m(), get_m(), _MM_SHUFFLE(0,0,0,0)));
      #else
        return vec3(v[0], v[0], v[0]);
      #endif
    }

    vec3 yyy() const { 
      #if OCTET_SSE
        return vec3(_mm_shuffle_ps(get_m(), get_m(), _MM_SHUFFLE(1,1,1,1)));
      #else
        return vec3(v[1], v[1], v[1]);
      #endif
    }

    vec3 zzz() const { 
      #if OCTET_SSE
        return vec3(_mm_shuffle_ps(get_m(), get_m(), _MM_SHUFFLE(2,2,2,2)));
      #else
        return vec3(v[2], v[2], v[2]);
      #endif
    }

    // access x
    float &x() {
      return v[0];
    }

    // access y
    float &y() {
      return v[1];
    }

    // access z
    float &z() {
      return v[2];
    }

    // get x
    float x() const {
      return v[0];
    }

    // get y
    float y() const {
      return v[1];
    }

    // get z
    float z() const {
      return v[2];
    }

    #if OCTET_SSE
      __m128 get_m() const { return m; }
    #endif

    // sum of terms
    float sum() const {
      return v[0] + v[1] + v[2];
      /*#if OCTET_SSE
        __m128 a = _mm_add_ps(m, _mm_shuffle_ps(m, m, _MM_SHUFFLE(1,0,3,2)));
        __m128 b = _mm_add_ps(a, _mm_shuffle_ps(a, a, _MM_SHUFFLE(2,0,0,0)));
        return b.m128_f32[0];
      #else
        return v[0] + v[1] + v[2];
      #endif*/
    }

    // convert to a string
    const char *toString(char *dest, size_t size) const {
      snprintf(dest, size, "[%f, %f, %f]", v[0], v[1], v[2]);
      return dest;
    }
  };

  // dot product
  inline float dot(const vec3 &lhs, const vec3 &rhs) {
     return lhs.dot(rhs); 
  }

  // cross product
  inline vec3 cross(const vec3 &lhs, const vec3 &rhs) {
     return lhs.cross(rhs); 
  }

  // positive cross product
  inline vec3 abs_cross(const vec3 &lhs, const vec3 &rhs) {
     return lhs.abs_cross(rhs); 
  }

  // add terms
  inline float sum(const vec3 &lhs) {
     return lhs.sum(); 
  }

  // make length = 1
  inline vec3 normalize(const vec3 &lhs) {
     return lhs.normalize(); 
  }

  // comonent-wise min
  inline vec3 min(const vec3 &lhs, const vec3 &r) {
     return lhs.min(r); 
  }

  // comonent-wise max
  inline vec3 max(const vec3 &lhs, const vec3 &r) {
     return lhs.max(r); 
  }

  // euclidean length
  inline float length(const vec3 &lhs) {
     return lhs.length(); 
  }

  // 1/length
  inline float lengthRecip(const vec3 &lhs) {
     return lhs.lengthRecip(); 
  }

  // length squared
  inline float squared(const vec3 &lhs) {
     return lhs.squared(); 
  }

  // component-wise abs
  inline vec3 abs(const vec3 &lhs) {
     return lhs.abs(); 
  }

  inline vec3 operator+(float lhs, const vec3 &rhs) {
    return rhs + lhs;
  }

  inline vec3 operator-(float lhs, const vec3 &rhs) {
    return rhs - lhs;
  }

  inline vec3 operator*(float lhs, const vec3 &rhs) {
    return rhs * lhs;
  }

  inline vec3 operator/(float lhs, const vec3 &rhs) {
    return rhs / lhs;
  }

  inline vec3 mix(vec3_in a, vec3_in b, float lambda) {
    return a * (1.0f - lambda) + b * lambda;
  }

  inline vec3 mix(vec3_in a, vec3_in b, rational_in lambda) {
    return (a * (lambda.denom() - lambda.numer()) + b * lambda.numer()) / lambda.denom();
  }

  // sadly the microsoft compile is quite poor with vector code generation
  #if OCTET_SSE
    #define OCTET_VEC3_CONST(VAR, X, Y, Z) static const u_m128_f4 VAR##_UNION = { X, Y, Z, 0 }; static const vec3 VAR(VAR##_UNION.m);
  #else
    #define OCTET_VEC3_CONST(VAR, X, Y, Z) vec3 VAR(X, Y, Z);
  #endif

  // vec3p: vector of 3 floats - optimial for storage, but not computation.
  class vec3p {
    float v[3];
  public:
    vec3p() { v[0] = v[1] = v[2] = 0; }
    vec3p(const vec3p &in) { v[0] = in.v[0]; v[1] = in.v[1]; v[2] = in.v[2]; }
    vec3p(const vec3 &in) {
      #if OCTET_SSE
        static const u_m128_i4 mask = { -1, -1, -1, 0 };
        #ifdef OCTET_MAC
          _mm_maskmoveu_si128(in.get_m(), (__m128i&)mask.m, (char*)v);
        #else
          _mm_maskmoveu_si128((__m128i&)in.get_m(), (__m128i&)mask.m, (char*)v);
        #endif
      #else
        v[0] = in[0]; v[1] = in[1]; v[2] = in[2];
      #endif
    }
    vec3p(float x, float y, float z) { v[0] = x; v[1] = y; v[2] = z; }
    operator vec3() const { return vec3(v[0], v[1], v[2]); }
  };

  #if OCTET_UNIT_TEST
    class vec3_unit_test {
    public:
      vec3_unit_test() {
        vec3 a;
        assert(a[0] == 0 && a[1] == 0 && a[2] == 0);
        assert(sizeof(a) <= 16);
      }
    };
    static vec3_unit_test vec3_unit_test;
  #endif

  std::ostream &operator <<(std::ostream &os, const vec3 &rhs) {
    char tmp[256];
    os << rhs.toString(tmp, sizeof(tmp));
    return os;
  }
} }

