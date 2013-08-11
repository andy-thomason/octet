////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Boolean Vector class
//

namespace octet {
  // boolean vector
  class bvec4 {
    int v[4]; // (v < 0) = true, (v >= 0) = false
  public:
    bvec4() {}

    bvec4(bool x, bool y, bool z, bool w) { v[0] = x ? -1 : 0; v[1] = y ? -1 : 0; v[2] = z ? -1 : 0; v[3] = w ? -1 : 0; };
    bvec4(int x, int y, int z, int w) { v[0] = x; v[1] = y; v[2] = z; v[3] = w; };

    int &operator[](int i) { return v[i]; }
    const int &operator[](int i) const { return v[i]; }
    bvec4 operator&(int r) const { return bvec4(v[0]&r, v[1]&r, v[2]&r, v[3]&r); }
    bvec4 operator|(int r) const { return bvec4(v[0]|r, v[1]|r, v[2]|r, v[3]|r); }
    bvec4 operator^(int r) const { return bvec4(v[0]^r, v[1]^r, v[2]^r, v[3]^r); }
    bvec4 operator&(const bvec4 &r) const { return bvec4(v[0]&r.v[0], v[1]&r.v[1], v[2]&r.v[2], v[3]&r.v[3]); }
    bvec4 operator|(const bvec4 &r) const { return bvec4(v[0]|r.v[0], v[1]|r.v[1], v[2]|r.v[2], v[3]|r.v[3]); }
    bvec4 operator^(const bvec4 &r) const { return bvec4(v[0]^r.v[0], v[1]^r.v[1], v[2]^r.v[2], v[3]^r.v[3]); }
    bvec4 &operator&=(const bvec4 &r) { v[0] &= r.v[0]; v[1] &= r.v[1]; v[2] &= r.v[2]; v[3] &= r.v[3]; return *this; }
    bvec4 &operator|=(const bvec4 &r) { v[0] |= r.v[0]; v[1] |= r.v[1]; v[2] |= r.v[2]; v[3] |= r.v[3]; return *this; }
    bvec4 &operator^=(const bvec4 &r) { v[0] ^= r.v[0]; v[1] ^= r.v[1]; v[2] ^= r.v[2]; v[3] ^= r.v[3]; return *this; }
    bvec4 operator~() const { return bvec4(~v[0], ~v[1], ~v[2], ~v[3]); }
    bvec4 xy() const { return bvec4(v[0], v[1], 0, 0); }
    bvec4 xyz() const { return bvec4(v[0], v[1], v[2], 0); }
    bvec4 xyz1() const { return bvec4(v[0], v[1], v[2], 1); }
    int &x() { return v[0]; }
    int &y() { return v[1]; }
    int &z() { return v[2]; }
    int &w() { return v[3]; }
    int x() const { return v[0]; }
    int y() const { return v[1]; }
    int z() const { return v[2]; }
    int w() const { return v[3]; }
  };

  inline bvec4 operator>(const vec4 &lhs, const vec4 &rhs) { return bvec4(fgt(lhs.x(), rhs.x()), fgt(lhs.y(), rhs.y()), fgt(lhs.z(), rhs.z()), fgt(lhs.w(), rhs.w())); }
  inline bvec4 operator<(const vec4 &lhs, const vec4 &rhs) { return bvec4(flt(lhs.x(), rhs.x()), flt(lhs.y(), rhs.y()), flt(lhs.z(), rhs.z()), flt(lhs.w(), rhs.w())); }
  inline bvec4 operator>=(const vec4 &lhs, const vec4 &rhs) { return bvec4(fge(lhs.x(), rhs.x()), fge(lhs.y(), rhs.y()), fge(lhs.z(), rhs.z()), fge(lhs.w(), rhs.w())); }
  inline bvec4 operator<=(const vec4 &lhs, const vec4 &rhs) { return bvec4(fle(lhs.x(), rhs.x()), fle(lhs.y(), rhs.y()), fle(lhs.z(), rhs.z()), fle(lhs.w(), rhs.w())); }
  inline bvec4 operator==(const vec4 &lhs, const vec4 &rhs) { return bvec4(feq(lhs.x(), rhs.x()), feq(lhs.y(), rhs.y()), feq(lhs.z(), rhs.z()), feq(lhs.w(), rhs.w())); }
  inline bvec4 operator!=(const vec4 &lhs, const vec4 &rhs) { return bvec4(fne(lhs.x(), rhs.x()), fne(lhs.y(), rhs.y()), fne(lhs.z(), rhs.z()), fne(lhs.w(), rhs.w())); }

  bool all(const bvec4 &b) {
    return (b.x() & b.y() & b.z() & b.w()) < 0;
  }

  bool any(const bvec4 &b) {
    return (b.x() | b.y() | b.z() | b.w()) < 0;
  }
}

