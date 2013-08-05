////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Integer Vector class
//

namespace octet {
  class ivec4 {
    int v[4];
  public:
    ivec4() {}

    ivec4(int x, int y, int z, int w) { v[0] = x; v[1] = y; v[2] = z; v[3] = w; };

    ivec4(const vec4 &rhs) {
      v[0] = (int)rhs[0];
      v[1] = (int)rhs[1];
      v[2] = (int)rhs[2];
      v[3] = (int)rhs[3];
    }

    int &operator[](int i) { return v[i]; }
    const int &operator[](int i) const { return v[i]; }
    ivec4 operator+(int r) const { return ivec4(v[0]+r, v[1]+r, v[2]+r, v[3]+r); }
    ivec4 operator-(int r) const { return ivec4(v[0]-r, v[1]-r, v[2]-r, v[3]-r); }
    ivec4 operator*(int r) const { return ivec4(v[0]*r, v[1]*r, v[2]*r, v[3]*r); }
    ivec4 operator+(const ivec4 &r) const { return ivec4(v[0]+r.v[0], v[1]+r.v[1], v[2]+r.v[2], v[3]+r.v[3]); }
    ivec4 operator-(const ivec4 &r) const { return ivec4(v[0]-r.v[0], v[1]-r.v[1], v[2]-r.v[2], v[3]-r.v[3]); }
    ivec4 operator*(const ivec4 &r) const { return ivec4(v[0]*r.v[0], v[1]*r.v[1], v[2]*r.v[2], v[3]*r.v[3]); }
    ivec4 operator-() const { return ivec4(-v[0], -v[1], -v[2], -v[3]); }
    ivec4 &operator+=(const ivec4 &r) { v[0] += r.v[0]; v[1] += r.v[1]; v[2] += r.v[2]; v[3] += r.v[3]; return *this; }
    ivec4 &operator-=(const ivec4 &r) { v[0] -= r.v[0]; v[1] -= r.v[1]; v[2] -= r.v[2]; v[3] -= r.v[3]; return *this; }
    ivec4 &operator*=(const ivec4 &r) { v[0] *= r.v[0]; v[1] *= r.v[1]; v[2] *= r.v[2]; v[3] *= r.v[3]; return *this; }
    int dot(const ivec4 &r) const { return v[0] * r.v[0] + v[1] * r.v[1] + v[2] * r.v[2] + v[3] * r.v[3]; }
    ivec4 min(const ivec4 &r) const { return ivec4(v[0] < r[0] ? v[0] : r[0], v[1] < r[1] ? v[1] : r[1], v[2] < r[2] ? v[2] : r[2], v[3] < r[3] ? v[3] : r[3]); }
    ivec4 max(const ivec4 &r) const { return ivec4(v[0] >= r[0] ? v[0] : r[0], v[1] >= r[1] ? v[1] : r[1], v[2] >= r[2] ? v[2] : r[2], v[3] >= r[3] ? v[3] : r[3]); }
    int squared() { return dot(*this); }
    ivec4 abs() const { return ivec4(v[0] < 0 ? -v[0] : v[0], v[1] < 0 ? -v[1] : v[1], v[2] < 0 ? -v[2] : v[2], v[3] < 0 ? -v[3] : v[3]); }
    ivec4 xy() const { return ivec4(v[0], v[1], 0, 0); }
    ivec4 xyz() const { return ivec4(v[0], v[1], v[2], 0); }
    ivec4 xyz1() const { return ivec4(v[0], v[1], v[2], 1); }
    int &x() { return v[0]; }
    int &y() { return v[1]; }
    int &z() { return v[2]; }
    int &w() { return v[3]; }
    int x() const { return v[0]; }
    int y() const { return v[1]; }
    int z() const { return v[2]; }
    int w() const { return v[3]; }
  };

}

