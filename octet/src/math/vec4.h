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

  class vec4 {
    static const char *Copyright() { return "Copyright(C) Andy Thomason 2011-2013"; }
    #if OCTET_SSE
      union {
        __m128 m;
        float v[4];
      };

    #else
      float v[4];
    #endif
  public:
    OCTET_HOT vec4() {
      #if OCTET_SSE
        m = _mm_setzero_ps();
      #else
        v[0] = v[1] = v[2] = v[3] = 0;
      #endif
    }

    #if OCTET_SSE
      OCTET_HOT vec4(__m128 m) {
        this->m = m;
      }
    #endif

    OCTET_HOT vec4(const vec4 &rhs) {
      #if OCTET_SSE
        m = rhs.m;
      #else
        v[0] = rhs.v[0]; v[1] = rhs.v[1]; v[2] = rhs.v[2]; v[3] = rhs.v[3];
      #endif
    }

    OCTET_HOT vec4(float f) {
      #if OCTET_SSE
        m = _mm_set_ps1(f);
      #else
        v[0] = v[1] = v[2] = v[3] = f;
      #endif
    }

    // construct from four scalars
    OCTET_HOT vec4(float x, float y, float z, float w) {
      #if OCTET_SSE
        m = _mm_setr_ps(x, y, z, w);
      #else
        v[0] = x; v[1] = y; v[2] = z; v[3] = w;
      #endif
    };

    /*explicit vec4(int x, int y, int z, int w) {
      #if OCTET_SSE
        __m64 lo = _mm_set_pi32(y, x);
        __m64 hi = _mm_set_pi32(w, z);
        m = _mm_cvtpi32x2_ps(lo, hi);
      #else
        v[0] = x; v[1] = y; v[2] = z; v[3] = w;
      #endif
    };*/

    OCTET_HOT vec4(const vec2 &xy, float z, float w) {
      v[0] = xy.x(); v[1] = xy.y(); v[2] = z; v[3] = w;
    };

    OCTET_HOT vec4(const vec3 &xyz, float w) {
      v[0] = xyz.x(); v[1] = xyz.y(); v[2] = xyz.z(); v[3] = w;
    };

    // index accessor [0] [1] ...
    OCTET_HOT float &operator[](int i) { return v[i]; }

    // constant index accessor
    OCTET_HOT const float &operator[](int i) const {
      return v[i];
    }

    // vector - scalar operators
    OCTET_HOT vec4 operator+(float r) const {
      return *this + vec4(r);
    }
    OCTET_HOT vec4 operator-(float r) const {
      return *this - vec4(r);
    }
    OCTET_HOT vec4 operator*(float r) const {
      return *this * vec4(r);
    }
    OCTET_HOT vec4 operator/(float r) const {
      return *this * vec4(recip(r));
    }

    // in-place vector operators
    OCTET_HOT vec4 &operator+=(const vec4 &r) {
      *this = *this + r;
      return *this;
    }

    OCTET_HOT vec4 &operator-=(const vec4 &r) {
      *this = *this - r;
      return *this;
    }

    OCTET_HOT vec4 &operator*=(const vec4 &r) {
      *this = *this * r;
      return *this;
    }

    // dot product
    OCTET_HOT float dot(const vec4 &r) const {
      return (*this * r).sum();
    }

    // make the length equal to 1
    OCTET_HOT vec4 normalize() const {
      return *this * lengthRecip();
    }

    // euclidean length of a vector
    OCTET_HOT float length() const {
      return sqrt(dot(*this));
    }

    // one over the euclidean length of a vector
    OCTET_HOT float lengthRecip() const {
      return rsqrt(dot(*this));
    }

    // length squared
    OCTET_HOT float squared() const {
      return dot(*this);
    }

    // cross product
    OCTET_HOT vec4 cross(const vec4 &r) const {
      return vec4(
        v[1] * r.v[2] - v[2] * r.v[1],
	      v[2] * r.v[0] - v[0] * r.v[2],
	      v[0] * r.v[1] - v[1] * r.v[0],
	      0.0f
	    );
    }

    // positive cross product (for box tests)
    OCTET_HOT vec4 abs_cross(const vec4 &r) const {
      return vec4(
        v[1] * r.v[2] + v[2] * r.v[1],
	      v[2] * r.v[0] + v[0] * r.v[2],
	      v[0] * r.v[1] + v[1] * r.v[0],
        0.0f
	    );
    }

    // access the floating point numbers
    OCTET_HOT float *get() { return &v[0]; }

    // get the floating point numbers
    OCTET_HOT const float *get() const { return &v[0]; }

    ////////////////////////////////////
    //
    // vec4 specific
    //

    // vector operators
    OCTET_HOT vec4 operator+(const vec4 &r) const {
      #if OCTET_SSE
        return vec4(_mm_add_ps(m, r.m));
      #else
        return vec4(v[0]+r.v[0], v[1]+r.v[1], v[2]+r.v[2], v[3]+r.v[3]);
      #endif
    }

    OCTET_HOT vec4 operator-(const vec4 &r) const {
      #if OCTET_SSE
        return vec4(_mm_sub_ps(m, r.m));
      #else
        return vec4(v[0]-r.v[0], v[1]-r.v[1], v[2]-r.v[2], v[3]-r.v[3]);
      #endif
    }

    OCTET_HOT vec4 operator*(const vec4 &r) const {
      #if OCTET_SSE
        return vec4(_mm_mul_ps(m, r.m));
      #else
        return vec4(v[0]*r.v[0], v[1]*r.v[1], v[2]*r.v[2], v[3]*r.v[3]);
      #endif
    }

    OCTET_HOT vec4 operator/(const vec4 &r) const {
      #if OCTET_SSE
        return vec4(_mm_div_ps(m, r.m));
      #else
        return vec4(v[0]*r.v[0], v[1]*r.v[1], v[2]*r.v[2], v[3]*r.v[3]);
      #endif
    }

    OCTET_HOT vec4 operator-() const {
      #if OCTET_SSE
        return vec4(_mm_sub_ps(_mm_setzero_ps(), m));
      #else
        return vec4(-v[0], -v[1], -v[2], -v[3]);
      #endif
    }

    // premultiply matrix operator (forward declared as matrix uses vector)
    OCTET_HOT vec4 operator*(const mat4t &r) const;

    // sum of terms
    OCTET_HOT float sum() const {
      return v[0] + v[1] + v[2] + v[3];
    }

    // quaternion conjugate
    OCTET_HOT vec4 qconj() const {
      return *this * vec4(-1, -1, -1, 1);
    }

    // after perspective transform, use this to find x, y, z in the cube.
    OCTET_HOT vec3 perspectiveDivide() const {
      return vec3(v[0], v[1], v[2]) * vec3(recip(v[3]));
    }

    // minumum of two vectors
    OCTET_HOT vec4 min(const vec4 &r) const {
      #if OCTET_SSE
        return vec4(_mm_min_ps(m, r.m));
      #else
        return vec4(v[0] < r[0] ? v[0] : r[0], v[1] < r[1] ? v[1] : r[1], v[2] < r[2] ? v[2] : r[2], v[3] < r[3] ? v[3] : r[3]);
      #endif
    }

    // maximum of two vectors
    OCTET_HOT vec4 max(const vec4 &r) const {
      #if OCTET_SSE
        return vec4(_mm_max_ps(m, r.m));
      #else
        return vec4(v[0] >= r[0] ? v[0] : r[0], v[1] >= r[1] ? v[1] : r[1], v[2] >= r[2] ? v[2] : r[2], v[3] >= r[3] ? v[3] : r[3]);
      #endif
    }

    // make all values positive.
    OCTET_HOT vec4 abs() const {
      #if OCTET_SSE
        static const union {
          int v[4];
          __m128 m;
        } u = { 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff };
        //u.v[0] = u.v[1] = u.v[2] = u.v[3] = 0x7fffffff;
        return vec4(_mm_and_ps(m, u.m));
      #else
        return vec4(octet::abs(v[0]), octet::abs(v[1]), octet::abs(v[2]), octet::abs(v[3]));
      #endif
    }

    OCTET_HOT operator const vec3&() const {
      return (const vec3&)v;
    }

    OCTET_HOT operator const vec2&() const {
      return (const vec2&)v;
    }

    // get xy
    OCTET_HOT const vec2 &xy() const {
      return (const vec2&)v;
    }

    // get xyz
    OCTET_HOT const vec3 &xyz() const {
      return (const vec3&)v;
    }

    // get xy00
    OCTET_HOT vec4 xy00() const {
      return vec4(v[0], v[1], 0.0f, 0.0f);
    }

    // get xyz0
    OCTET_HOT vec4 xyz0() const {
      return vec4(v[0], v[1], v[2], 0.0f);
    }

    // get xyz1
    OCTET_HOT vec4 xyz1() const {
      return vec4(v[0], v[1], v[2], 1.0f);
    }

    OCTET_HOT vec4 xxxx() const {
      #if OCTET_SSE
        return vec4(_mm_shuffle_ps(m, m, _MM_SHUFFLE(0,0,0,0)));
      #else
        return vec4(v[0], v[0], v[0], v[0]);
      #endif
    }

    OCTET_HOT vec4 yyyy() const {
      #if OCTET_SSE
        return vec4(_mm_shuffle_ps(m, m, _MM_SHUFFLE(1,1,1,1)));
      #else
        return vec4(v[1], v[1], v[1], v[1]);
      #endif
    }

    OCTET_HOT vec4 zzzz() const {
      #if OCTET_SSE
        return vec4(_mm_shuffle_ps(m, m, _MM_SHUFFLE(2,2,2,2)));
      #else
        return vec4(v[2], v[2], v[2], v[2]);
      #endif
    }

    OCTET_HOT vec4 wwww() const {
      #if OCTET_SSE
        return vec4(_mm_shuffle_ps(m, m, _MM_SHUFFLE(3,3,3,3)));
      #else
        return vec4(v[3], v[3], v[3], v[3]);
      #endif
    }

    // access x
    OCTET_HOT float &x() {
      return v[0];
    }

    // access y
    OCTET_HOT float &y() {
      return v[1];
    }

    // access z
    OCTET_HOT float &z() {
      return v[2];
    }

    // access w
    OCTET_HOT float &w() {
      return v[3];
    }

    // get x
    OCTET_HOT float x() const {
      return v[0];
    }

    // get y
    OCTET_HOT float y() const {
      return v[1];
    }

    // get z
    OCTET_HOT float z() const {
      return v[2];
    }

    // get w
    OCTET_HOT float w() const {
      return v[3];
    }

    // quaternion multiply
    OCTET_HOT vec4 qmul(const vec4 &r) const {
      return vec4(
	      v[0] * r.v[3] + v[3] * r.v[0] + v[1] * r.v[2] - v[2] * r.v[1],
		    v[1] * r.v[3] + v[3] * r.v[1] + v[2] * r.v[0] - v[0] * r.v[2],
		    v[2] * r.v[3] + v[3] * r.v[2] + v[0] * r.v[1] - v[1] * r.v[0],
		    v[3] * r.v[3] - v[0] * r.v[0] - v[1] * r.v[1] - v[2] * r.v[2]
      );
    }

    // convert to a string (up to 4 strings can be included at a time)
    const char *toString(char *dest, size_t len) const
    {
      snprintf(dest, len, "[%f, %f, %f, %f]", v[0], v[1], v[2], v[3]);
      return dest;
    }
  };

  // dot product
  inline float dot(const vec4 &lhs, const vec4 &rhs) {
     return lhs.dot(rhs); 
  }

  // add terms
  inline float sum(const vec4 &lhs) {
     return lhs.sum(); 
  }

  // divide by w
  inline vec3 perspectiveDivide(const vec4 &lhs) {
     return lhs.perspectiveDivide(); 
  }

  // make length = 1
  inline vec4 normalize(const vec4 &lhs) {
     return lhs.normalize(); 
  }

  // comonent-wise min
  inline vec4 min(const vec4 &lhs, const vec4 &r) {
     return lhs.min(r); 
  }

  // comonent-wise max
  inline vec4 max(const vec4 &lhs, const vec4 &r) {
     return lhs.max(r); 
  }

  // euclidean length
  inline float length(const vec4 &lhs) {
     return lhs.length(); 
  }

  // 1/length
  inline float lengthRecip(const vec4 &lhs) {
     return lhs.lengthRecip(); 
  }

  // length squared
  inline float squared(const vec4 &lhs) {
     return lhs.squared(); 
  }

  // component-wise abs
  inline vec4 abs(const vec4 &lhs) {
     return lhs.abs(); 
  }

  inline vec4 vec3::xyz0() const {
    return vec4(v[0], v[1], v[2], 0.0f);
  }

  inline vec4 vec3::xyz1() const {
    return vec4(v[0], v[1], v[2], 1.0f);
  }

  inline vec4 vec3::xxxx() const {
    #if OCTET_SSE
      return vec4(_mm_shuffle_ps(get_m(), get_m(), _MM_SHUFFLE(0,0,0,0)));
    #else
      return vec4(v[0], v[0], v[0], v[0]);
    #endif
  }

  inline vec4 vec3::yyyy() const {
    #if OCTET_SSE
      return vec4(_mm_shuffle_ps(get_m(), get_m(), _MM_SHUFFLE(1,1,1,1)));
    #else
      return vec4(v[1], v[1], v[1], v[1]);
    #endif
  }

  inline vec4 vec3::zzzz() const {
    #if OCTET_SSE
      return vec4(_mm_shuffle_ps(get_m(), get_m(), _MM_SHUFFLE(2,2,2,2)));
    #else
      return vec4(v[2], v[2], v[2], v[2]);
    #endif
  }

  inline vec4 operator+(float lhs, const vec4 &rhs) {
    return vec4(lhs) + rhs;
  }

  inline vec4 operator-(float lhs, const vec4 &rhs) {
    return vec4(lhs) - rhs;
  }

  inline vec4 operator*(float lhs, const vec4 &rhs) {
    return vec4(lhs) * rhs;
  }

  inline vec4 operator/(float lhs, const vec4 &rhs) {
    return vec4(lhs) / rhs;
  }

  // sadly the microsoft compile is quite poor with vector code generation
  #if OCTET_SSE
    #define OCTET_VEC4_CONST(VAR, X, Y, Z, W) static const u_m128_f4 VAR##_UNION = { X, Y, Z, W }; const vec4 VAR(VAR##_UNION.m);
  #else
    #define OCTET_VEC4_CONST(VAR, X, Y, Z, W) vec4 VAR(X, Y, Z, W);
  #endif

  std::ostream &operator <<(std::ostream &os, const vec4 &rhs) {
    char tmp[256];
    os << rhs.toString(tmp, sizeof(tmp));
    return os;
  }
} }

