////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Vector class
//

namespace octet { namespace math {
  class bvec3 {
    #if OCTET_SSE
      union {
        __m128 m;
        int v[4];
      };
    #else
      int v[3];
    #endif
  public:
    bvec3() {}

    bvec3(bool x, bool y, bool z) { v[0] = x ? -1 : 0; v[1] = y ? -1 : 0; v[2] = z ? -1 : 0; };
    bvec3(int x, int y, int z) { v[0] = x; v[1] = y; v[2] = z; };

    #if OCTET_SSE
      bvec3(__m128 m) {
        this->m = m;
      }

      __m128 get_m() const {
        return m;
      }
    #endif

    int &operator[](int i) { return v[i]; }
    const int &operator[](int i) const { return v[i]; }
    bvec3 operator&(int r) const { return bvec3(v[0]&r, v[1]&r, v[2]&r); }
    bvec3 operator|(int r) const { return bvec3(v[0]|r, v[1]|r, v[2]|r); }
    bvec3 operator^(int r) const { return bvec3(v[0]^r, v[1]^r, v[2]^r); }
    bvec3 operator&(const bvec3 &r) const { return bvec3(v[0]&r.v[0], v[1]&r.v[1], v[2]&r.v[2]); }
    bvec3 operator|(const bvec3 &r) const { return bvec3(v[0]|r.v[0], v[1]|r.v[1], v[2]|r.v[2]); }
    bvec3 operator^(const bvec3 &r) const { return bvec3(v[0]^r.v[0], v[1]^r.v[1], v[2]^r.v[2]); }
    bvec3 &operator&=(const bvec3 &r) { v[0] &= r.v[0]; v[1] &= r.v[1]; v[2] &= r.v[2]; return *this; }
    bvec3 &operator|=(const bvec3 &r) { v[0] |= r.v[0]; v[1] |= r.v[1]; v[2] |= r.v[2]; return *this; }
    bvec3 &operator^=(const bvec3 &r) { v[0] ^= r.v[0]; v[1] ^= r.v[1]; v[2] ^= r.v[2]; return *this; }
    bvec3 operator~() const { return bvec3(~v[0], ~v[1], ~v[2]); }
    int &x() { return v[0]; }
    int &y() { return v[1]; }
    int &z() { return v[2]; }
    int x() const { return v[0]; }
    int y() const { return v[1]; }
    int z() const { return v[2]; }

    // convert to a string (up to 4 strings can be included at a time)
    const char *toString() const
    {
      char *dest = get_sprintf_buffer();
      sprintf(dest, "[%2d, %2d, %2d]", v[0]>>31, v[1]>>31, v[2]>>31);
      return dest;
    }
  };

  inline bvec3 operator>(const vec3 &lhs, const vec3 &rhs) {
    #if OCTET_SSE
      return bvec3(_mm_cmpgt_ps(lhs.get_m(), rhs.get_m()) );
    #else
      return bvec3(fgt(lhs.x(), rhs.x()), fgt(lhs.y(), rhs.y()), fgt(lhs.z(), rhs.z()) );
    #endif
  }

  inline bvec3 operator<(const vec3 &lhs, const vec3 &rhs) {
    #if OCTET_SSE
      return bvec3(_mm_cmplt_ps(lhs.get_m(), rhs.get_m()) );
    #else
      return bvec3(flt(lhs.x(), rhs.x()), flt(lhs.y(), rhs.y()), flt(lhs.z(), rhs.z()) );
    #endif
  }

  inline bvec3 operator>=(const vec3 &lhs, const vec3 &rhs) {
    #if OCTET_SSE
      return bvec3(_mm_cmpge_ps(lhs.get_m(), rhs.get_m()) );
    #else
      return bvec3(fge(lhs.x(), rhs.x()), fge(lhs.y(), rhs.y()), fge(lhs.z(), rhs.z()) );
    #endif
  }

  inline bvec3 operator<=(const vec3 &lhs, const vec3 &rhs) {
    #if OCTET_SSE
      return bvec3(_mm_cmple_ps(lhs.get_m(), rhs.get_m()) );
    #else
      return bvec3(fle(lhs.x(), rhs.x()), fle(lhs.y(), rhs.y()), fle(lhs.z(), rhs.z()) );
    #endif
  }

  inline bvec3 operator==(const vec3 &lhs, const vec3 &rhs) {
    #if OCTET_SSE
      return bvec3(_mm_cmpeq_ps(lhs.get_m(), rhs.get_m()) );
    #else
      return bvec3(feq(lhs.x(), rhs.x()), feq(lhs.y(), rhs.y()), feq(lhs.z(), rhs.z()) );
    #endif
  }

  inline bvec3 operator!=(const vec3 &lhs, const vec3 &rhs) {
    #if OCTET_SSE
      return bvec3(_mm_cmpneq_ps(lhs.get_m(), rhs.get_m()) );
    #else
      return bvec3(fne(lhs.x(), rhs.x()), fne(lhs.y(), rhs.y()), fne(lhs.z(), rhs.z()) );
    #endif
  }

  bool all(const bvec3 &b) {
    #if OCTET_SSE
      return (_mm_movemask_ps(b.get_m()) & 7) == 7;
    #else
      return (b.x() & b.y() & b.z()) < 0;
    #endif
  }

  bool any(const bvec3 &b) {
    #if OCTET_SSE
      return (_mm_movemask_ps(b.get_m()) & 7) != 0;
    #else
      return (b.x() | b.y() | b.z()) < 0;
    #endif
  }
} }

