////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Integer Vector class
//

namespace octet { namespace math {
  class ivec3 {
    int v[3];
  public:
    ivec3() {}

    ivec3(int x, int y, int z) {
      v[0] = x; v[1] = y; v[2] = z;
    };

    ivec3(int xyz) {
      v[0] = v[1] = v[2] = xyz;
    };

    ivec3(const vec4 &rhs) {
      v[0] = (int)rhs[0];
      v[1] = (int)rhs[1];
      v[2] = (int)rhs[2];
    }

    ivec3(const vec3 &rhs) {
      v[0] = (int)rhs[0];
      v[1] = (int)rhs[1];
      v[2] = (int)rhs[2];
    }

    int &operator[](int i) { return v[i]; }
    const int &operator[](int i) const { return v[i]; }
    ivec3 operator+(int r) const { return ivec3(v[0]+r, v[1]+r, v[2]+r); }
    ivec3 operator-(int r) const { return ivec3(v[0]-r, v[1]-r, v[2]-r); }
    ivec3 operator*(int r) const { return ivec3(v[0]*r, v[1]*r, v[2]*r); }
    ivec3 operator+(const ivec3 &r) const { return ivec3(v[0]+r.v[0], v[1]+r.v[1], v[2]+r.v[2]); }
    ivec3 operator-(const ivec3 &r) const { return ivec3(v[0]-r.v[0], v[1]-r.v[1], v[2]-r.v[2]); }
    ivec3 operator*(const ivec3 &r) const { return ivec3(v[0]*r.v[0], v[1]*r.v[1], v[2]*r.v[2]); }
    ivec3 operator>>(const ivec3 &r) const { return ivec3(v[0]>>r.v[0], v[1]>>r.v[1], v[2]>>r.v[2]); }
    ivec3 operator<<(const ivec3 &r) const { return ivec3(v[0]<<r.v[0], v[1]<<r.v[1], v[2]<<r.v[2]); }
    ivec3 operator&(const ivec3 &r) const { return ivec3(v[0]&r.v[0], v[1]&r.v[1], v[2]&r.v[2]); }
    ivec3 operator|(const ivec3 &r) const { return ivec3(v[0]|r.v[0], v[1]|r.v[1], v[2]|r.v[2]); }
    ivec3 operator^(const ivec3 &r) const { return ivec3(v[0]^r.v[0], v[1]^r.v[1], v[2]^r.v[2]); }
    ivec3 operator-() const { return ivec3(-v[0], -v[1], -v[2]); }
    ivec3 operator~() const { return ivec3(~v[0], ~v[1], ~v[2]); }
    ivec3 &operator+=(const ivec3 &r) { v[0] += r.v[0]; v[1] += r.v[1]; v[2] += r.v[2]; return *this; }
    ivec3 &operator-=(const ivec3 &r) { v[0] -= r.v[0]; v[1] -= r.v[1]; v[2] -= r.v[2]; return *this; }
    ivec3 &operator*=(const ivec3 &r) { v[0] *= r.v[0]; v[1] *= r.v[1]; v[2] *= r.v[2]; return *this; }
    int dot(const ivec3 &r) const { return v[0] * r.v[0] + v[1] * r.v[1] + v[2] * r.v[2]; }
    ivec3 min(const ivec3 &r) const { return ivec3(v[0] < r[0] ? v[0] : r[0], v[1] < r[1] ? v[1] : r[1], v[2] < r[2] ? v[2] : r[2]); }
    ivec3 max(const ivec3 &r) const { return ivec3(v[0] >= r[0] ? v[0] : r[0], v[1] >= r[1] ? v[1] : r[1], v[2] >= r[2] ? v[2] : r[2]); }
    int squared() { return dot(*this); }
    ivec3 abs() const { return ivec3(v[0] < 0 ? -v[0] : v[0], v[1] < 0 ? -v[1] : v[1], v[2] < 0 ? -v[2] : v[2]); }
    int &x() { return v[0]; }
    int &y() { return v[1]; }
    int &z() { return v[2]; }
    int x() const { return v[0]; }
    int y() const { return v[1]; }
    int z() const { return v[2]; }

    // convert to a string
    const char *toString(char *dest, size_t size) const
    {
      snprintf(dest, size, "[%08x, %08x, %08x]", v[0], v[1], v[2]);
      return dest;
    }
  };

  inline vec3::vec3(const ivec3 &rhs) {
    #if OCTET_SSE
      v[0] = (float)rhs[0];
      v[1] = (float)rhs[1];
      v[2] = (float)rhs[2];
    #else
      v[0] = (float)rhs[0];
      v[1] = (float)rhs[1];
      v[2] = (float)rhs[2];
    #endif
  }
} }

