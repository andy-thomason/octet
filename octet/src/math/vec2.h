////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Vector class
//

namespace octet {
  class mat4t;

  class vec2 {
    static const char *Copyright() { return "Copyright(C) Andy Thomason 2011-2013"; }
    float v[2];
  public:
    // default constructor: note does not initialize!
    vec2() {
    }

    // construct from four scalars
    vec2(float x, float y) {
      v[0] = x; v[1] = y;
    };

    // index accessor [0] [1] [2]
    float &operator[](int i) { return v[i]; }

    // constant index accessor
    const float &operator[](int i) const {
      return v[i];
    }

    // vector - scalar operators
    vec2 operator+(float r) const {
      return vec2(v[0]+r, v[1]+r);
    }
    vec2 operator-(float r) const {
      return vec2(v[0]-r, v[1]-r);
    }
    vec2 operator*(float r) const {
      return vec2(v[0]*r, v[1]*r);
    }
    vec2 operator/(float r) const {
       float rcp = 1.0f / r; return vec2(v[0]*rcp, v[1]*rcp);
    }

    // premultiply matrix operator (forward declared as matrix uses vector)
    vec2 operator*(const mat4t &r) const;

    // vector operators
    vec2 operator+(const vec2 &r) const {
      return vec2(v[0]+r.v[0], v[1]+r.v[1]);
    }
    vec2 operator-(const vec2 &r) const {
      return vec2(v[0]-r.v[0], v[1]-r.v[1]);
    }
    vec2 operator*(const vec2 &r) const {
      return vec2(v[0]*r.v[0], v[1]*r.v[1]);
    }
    vec2 operator-() const {
      return vec2(-v[0], -v[1]);
    }

    // in-place vector operators
    vec2 &operator+=(const vec2 &r) {
      v[0] += r.v[0]; v[1] += r.v[1];
      return *this;
    }
    vec2 &operator-=(const vec2 &r) {
      v[0] -= r.v[0]; v[1] -= r.v[1];
      return *this;
    }
    vec2 &operator*=(const vec2 &r) {
      v[0] *= r.v[0]; v[1] *= r.v[1];
      return *this;
    }

    // dot product
    float dot(const vec2 &r) const {
      return v[0] * r.v[0] + v[1] * r.v[1];
    }

    // sum of terms
    float sum() const {
      return v[0] + v[1];
    }

    // make the length equal to 1
    vec2 normalize() const {
      return *this * lengthRecip();
    }

    // minumum of two vectors
    vec2 min(const vec2 &r) const {
      return vec2(v[0] < r[0] ? v[0] : r[0], v[1] < r[1] ? v[1] : r[1]);
    }

    // maximum of two vectors
    vec2 max(const vec2 &r) const {
      return vec2(v[0] >= r[0] ? v[0] : r[0], v[1] >= r[1] ? v[1] : r[1]);
    }

    // euclidean length of a vector
    float length() const {
      return sqrtf(dot(*this));
    }

    // one over the euclidean length of a vector
    float lengthRecip() const {
      return 1.0f/sqrtf(dot(*this));
    }

    // length squared
    float squared() const {
      return dot(*this);
    }

    // make all values positive.
    vec2 abs() const {
      return vec2(fabsf(v[0]), fabsf(v[1]));
    }

    // get xy
    vec2 xy() const {
      return vec2(v[0], v[1]);
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

    // access the floating point numbers
    float *get() { return &v[0]; }

    // get the floating point numbers
    const float *get() const { return &v[0]; }

    // convert to a string (up to 4 strings can be included at a time)
    const char *toString() const
    {
      char *dest = get_sprintf_buffer();
      sprintf(dest, "[%f, %f]", v[0], v[1]);
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
}

