////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Boolean Vector class
//

namespace octet { namespace math {
  class bvec2 {
    int v[2]; // true if < 0
  public:
    bvec2() {}

    bvec2(bool x, bool y) { v[0] = x ? -1 : 0; v[1] = y ? -1 : 0; };
    bvec2(int x, int y) { v[0] = x; v[1] = y; };

    int &operator[](int i) { return v[i]; }
    const int &operator[](int i) const { return v[i]; }
    bvec2 operator&(int r) const { return bvec2(v[0]&r, v[1]&r); }
    bvec2 operator|(int r) const { return bvec2(v[0]|r, v[1]|r); }
    bvec2 operator^(int r) const { return bvec2(v[0]^r, v[1]^r); }
    bvec2 operator&(const bvec2 &r) const { return bvec2(v[0]&r.v[0], v[1]&r.v[1]); }
    bvec2 operator|(const bvec2 &r) const { return bvec2(v[0]|r.v[0], v[1]|r.v[1]); }
    bvec2 operator^(const bvec2 &r) const { return bvec2(v[0]^r.v[0], v[1]^r.v[1]); }
    bvec2 &operator&=(const bvec2 &r) { v[0] &= r.v[0]; v[1] &= r.v[1]; return *this; }
    bvec2 &operator|=(const bvec2 &r) { v[0] |= r.v[0]; v[1] |= r.v[1]; return *this; }
    bvec2 &operator^=(const bvec2 &r) { v[0] ^= r.v[0]; v[1] ^= r.v[1]; return *this; }
    bvec2 operator~() const { return bvec2(~v[0], ~v[1]); }
    int &x() { return v[0]; }
    int &y() { return v[1]; }
    int x() const { return v[0]; }
    int y() const { return v[1]; }
  };

  inline bvec2 operator>(const vec2 &lhs, const vec2 &rhs) { return bvec2(fgt(lhs.x(), rhs.x()), fgt(lhs.y(), rhs.y()) ); }
  inline bvec2 operator<(const vec2 &lhs, const vec2 &rhs) { return bvec2(flt(lhs.x(), rhs.x()), flt(lhs.y(), rhs.y()) ); }
  inline bvec2 operator>=(const vec2 &lhs, const vec2 &rhs) { return bvec2(fge(lhs.x(), rhs.x()), fge(lhs.y(), rhs.y()) ); }
  inline bvec2 operator<=(const vec2 &lhs, const vec2 &rhs) { return bvec2(fle(lhs.x(), rhs.x()), fle(lhs.y(), rhs.y()) ); }
  inline bvec2 operator==(const vec2 &lhs, const vec2 &rhs) { return bvec2(feq(lhs.x(), rhs.x()), feq(lhs.y(), rhs.y()) ); }
  inline bvec2 operator!=(const vec2 &lhs, const vec2 &rhs) { return bvec2(fne(lhs.x(), rhs.x()), fne(lhs.y(), rhs.y()) ); }

  bool all(const bvec2 &b) {
    return (b.x() & b.y()) < 0;
  }

  bool any(const bvec2 &b) {
    return (b.x() | b.y()) < 0;
  }
} }

