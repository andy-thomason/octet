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
  class vec4;

  class vec3 {
    static const char *Copyright() { return "Copyright(C) Andy Thomason 2011-2013"; }
    float v[3];
  public:
    // default constructor: note does not initialize!
    vec3() {
    }

    // construct from four scalars
    vec3(float x, float y, float z) {
      v[0] = x; v[1] = y; v[2] = z;
    };

    // index accessor [0] [1] [2]
    float &operator[](int i) { return v[i]; }

    // constant index accessor
    const float &operator[](int i) const {
      return v[i];
    }

    // vector - scalar operators
    vec3 operator+(float r) const {
      return vec3(v[0]+r, v[1]+r, v[2]+r);
    }
    vec3 operator-(float r) const {
      return vec3(v[0]-r, v[1]-r, v[2]-r);
    }
    vec3 operator*(float r) const {
      return vec3(v[0]*r, v[1]*r, v[2]*r);
    }
    vec3 operator/(float r) const {
       float rcp = 1.0f / r; return vec3(v[0]*rcp, v[1]*rcp, v[2]*rcp);
    }

    // premultiply matrix operator (forward declared as matrix uses vector)
    vec3 operator*(const mat4t &r) const;

    // vector operators
    vec3 operator+(const vec3 &r) const {
      return vec3(v[0]+r.v[0], v[1]+r.v[1], v[2]+r.v[2]);
    }
    vec3 operator-(const vec3 &r) const {
      return vec3(v[0]-r.v[0], v[1]-r.v[1], v[2]-r.v[2]);
    }
    vec3 operator*(const vec3 &r) const {
      return vec3(v[0]*r.v[0], v[1]*r.v[1], v[2]*r.v[2]);
    }
    vec3 operator-() const {
      return vec3(-v[0], -v[1], -v[2]);
    }

    // in-place vector operators
    vec3 &operator+=(const vec3 &r) {
      v[0] += r.v[0]; v[1] += r.v[1]; v[2] += r.v[2];
      return *this;
    }
    vec3 &operator-=(const vec3 &r) {
      v[0] -= r.v[0]; v[1] -= r.v[1]; v[2] -= r.v[2];
      return *this;
    }
    vec3 &operator*=(const vec3 &r) {
      v[0] *= r.v[0]; v[1] *= r.v[1]; v[2] *= r.v[2];
      return *this;
    }

    // dot product
    float dot(const vec3 &r) const {
      return v[0] * r.v[0] + v[1] * r.v[1] + v[2] * r.v[2];
    }

    // sum of terms
    float sum() const {
      return v[0] + v[1] + v[2];
    }

    // make the length equal to 1
    vec3 normalize() const {
      return *this * lengthRecip();
    }

    // minumum of two vectors
    vec3 min(const vec3 &r) const {
      return vec3(v[0] < r[0] ? v[0] : r[0], v[1] < r[1] ? v[1] : r[1], v[2] < r[2] ? v[2] : r[2]);
    }

    // maximum of two vectors
    vec3 max(const vec3 &r) const {
      return vec3(v[0] >= r[0] ? v[0] : r[0], v[1] >= r[1] ? v[1] : r[1], v[2] >= r[2] ? v[2] : r[2]);
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
    vec3 abs() const {
      return vec3(fabsf(v[0]), fabsf(v[1]), fabsf(v[2]));
    }

    // get vec2
    vec2 xy() const {
      return vec2(v[0], v[1]);
    }

    // get vec4
    vec4 xyz0() const;
    vec4 xyz1() const;

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

    // cross product
    vec3 cross(const vec3 &r) const {
      return vec3(
        v[1] * r.v[2] - v[2] * r.v[1],
	      v[2] * r.v[0] - v[0] * r.v[2],
	      v[0] * r.v[1] - v[1] * r.v[0]
	    );
    }

    // positive cross product (for box tests)
    vec3 abs_cross(const vec3 &r) const {
      return vec3(
        v[1] * r.v[2] + v[2] * r.v[1],
	      v[2] * r.v[0] + v[0] * r.v[2],
	      v[0] * r.v[1] + v[1] * r.v[0]
	    );
    }

    // access the floating point numbers
    float *get() { return &v[0]; }

    // get the floating point numbers
    const float *get() const { return &v[0]; }

    // convert to a string (up to 4 strings can be included at a time)
    const char *toString() const
    {
      char *dest = get_sprintf_buffer();
      sprintf(dest, "[%f, %f, %f]", v[0], v[1], v[2]);
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
}

