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

  class vec4 {
    static const char *Copyright() { return "Copyright(C) Andy Thomason 2011-2013"; }
    float v[4];
  public:
    // default constructor: note does not initialize!
    vec4() {
    }

    // construct from four scalars
    vec4(float x, float y, float z, float w) {
      v[0] = x; v[1] = y; v[2] = z; v[3] = w;
    };

    vec4(const vec2 &xy, float z, float w) {
      v[0] = xy.x(); v[1] = xy.y(); v[2] = z; v[3] = w;
    };

    vec4(const vec3 &xyz, float w) {
      v[0] = xyz.x(); v[1] = xyz.y(); v[2] = xyz.z(); v[3] = w;
    };

    // index accessor [0] [1] [2] [3]
    float &operator[](int i) { return v[i]; }

    // constant index accessor
    const float &operator[](int i) const {
      return v[i];
    }

    // vector - scalar operators
    vec4 operator+(float r) const {
      return vec4(v[0]+r, v[1]+r, v[2]+r, v[3]+r);
    }
    vec4 operator-(float r) const {
      return vec4(v[0]-r, v[1]-r, v[2]-r, v[3]-r);
    }
    vec4 operator*(float r) const {
      return vec4(v[0]*r, v[1]*r, v[2]*r, v[3]*r);
    }
    vec4 operator/(float r) const {
       float rcp = 1.0f / r; return vec4(v[0]*rcp, v[1]*rcp, v[2]*rcp, v[3]*rcp);
    }

    // premultiply matrix operator (forward declared as matrix uses vector)
    vec4 operator*(const mat4t &r) const;

    // vector operators
    vec4 operator+(const vec4 &r) const {
      return vec4(v[0]+r.v[0], v[1]+r.v[1], v[2]+r.v[2], v[3]+r.v[3]);
    }
    vec4 operator-(const vec4 &r) const {
      return vec4(v[0]-r.v[0], v[1]-r.v[1], v[2]-r.v[2], v[3]-r.v[3]);
    }
    vec4 operator*(const vec4 &r) const {
      return vec4(v[0]*r.v[0], v[1]*r.v[1], v[2]*r.v[2], v[3]*r.v[3]);
    }
    vec4 operator-() const {
      return vec4(-v[0], -v[1], -v[2], -v[3]);
    }

    // in-place vector operators
    vec4 &operator+=(const vec4 &r) {
      v[0] += r.v[0]; v[1] += r.v[1]; v[2] += r.v[2]; v[3] += r.v[3];
      return *this;
    }
    vec4 &operator-=(const vec4 &r) {
      v[0] -= r.v[0]; v[1] -= r.v[1]; v[2] -= r.v[2]; v[3] -= r.v[3];
      return *this;
    }
    vec4 &operator*=(const vec4 &r) {
      v[0] *= r.v[0]; v[1] *= r.v[1]; v[2] *= r.v[2]; v[3] *= r.v[3];
      return *this;
    }

    // quaternion conjugate
    vec4 qconj() const {
      return vec4(-v[0], -v[1], -v[2], v[3]);
    }

    // dot product
    float dot(const vec4 &r) const {
      return v[0] * r.v[0] + v[1] * r.v[1] + v[2] * r.v[2] + v[3] * r.v[3];
    }

    // sum of terms
    float sum() const {
      return v[0] + v[1] + v[2] + v[3];
    }

    // after perspective transform, use this to find x, y, z in the cube.
    vec4 perspectiveDivide() const {
      float r = 1.0f / v[3];
      return vec4(v[0]*r, v[1]*r, v[2]*r, v[3]*r);
    }

    // make the length equal to 1
    vec4 normalize() const {
      return *this * lengthRecip();
    }

    // minumum of two vectors
    vec4 min(const vec4 &r) const {
      return vec4(v[0] < r[0] ? v[0] : r[0], v[1] < r[1] ? v[1] : r[1], v[2] < r[2] ? v[2] : r[2], v[3] < r[3] ? v[3] : r[3]);
    }

    // maximum of two vectors
    vec4 max(const vec4 &r) const {
      return vec4(v[0] >= r[0] ? v[0] : r[0], v[1] >= r[1] ? v[1] : r[1], v[2] >= r[2] ? v[2] : r[2], v[3] >= r[3] ? v[3] : r[3]);
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
    vec4 abs() const {
      return vec4(fabsf(v[0]), fabsf(v[1]), fabsf(v[2]), fabsf(v[3]));
    }

    // get xy
    vec2 xy() const {
      return vec2(v[0], v[1]);
    }

    // get xyz
    vec3 xyz() const {
      return vec3(v[0], v[1], v[2]);
    }

    // get xy00
    vec4 xy00() const {
      return vec4(v[0], v[1], 0, 0);
    }

    // get xyz0
    vec4 xyz0() const {
      return vec4(v[0], v[1], v[2], 0);
    }

    // get xyz1
    vec4 xyz1() const {
      return vec4(v[0], v[1], v[2], 1);
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

    // access w
    float &w() {
      return v[3];
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

    // get w
    float w() const {
      return v[3];
    }

    // quaternion multiply
    vec4 qmul(const vec4 &r) const {
      return vec4(
	      v[0] * r.v[3] + v[3] * r.v[0] + v[1] * r.v[2] - v[2] * r.v[1],
		    v[1] * r.v[3] + v[3] * r.v[1] + v[2] * r.v[0] - v[0] * r.v[2],
		    v[2] * r.v[3] + v[3] * r.v[2] + v[0] * r.v[1] - v[1] * r.v[0],
		    v[3] * r.v[3] - v[0] * r.v[0] - v[1] * r.v[1] - v[2] * r.v[2]
      );
    }

    // cross product
    vec4 cross(const vec4 &r) const {
      return vec4(
        v[1] * r.v[2] - v[2] * r.v[1],
	      v[2] * r.v[0] - v[0] * r.v[2],
	      v[0] * r.v[1] - v[1] * r.v[0],
	      0
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
      sprintf(dest, "[%f, %f, %f, %f]", v[0], v[1], v[2], v[3]);
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
  inline vec4 perspectiveDivide(const vec4 &lhs) {
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
    return vec4(v[0], v[1], v[2], 0);
  }

  inline vec4 vec3::xyz1() const {
    return vec4(v[0], v[1], v[2], 1);
  }

  inline vec4 operator+(float lhs, const vec4 &rhs) {
    return rhs + lhs;
  }

  inline vec4 operator-(float lhs, const vec4 &rhs) {
    return rhs - lhs;
  }

  inline vec4 operator*(float lhs, const vec4 &rhs) {
    return rhs * lhs;
  }

  inline vec4 operator/(float lhs, const vec4 &rhs) {
    return rhs / lhs;
  }
}

