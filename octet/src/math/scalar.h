////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// scalar operations
//

namespace octet {
  // cheaty floating point compares
  // not exactly ieee!

  // top bit set (ie negative) if a > b and returns zero if they equal
  inline int fgt(float a, float b) {
    // note we have to use a union because of GCC's
    // type based alias analysis interpretation.
    #if OCTET_SSE
      union { __m128 m; float f; int i; } fur, fua, fub;
      // negative numbers are 1.......
      fua.f = a;
      fub.f = b;
      fur.m = _mm_cmple_ps(fua.m, fub.m);
      return fur.i;
    #else
      union { float f; int i; } fu;
      // negative numbers are 1.......
      fu.f = b - a;
      return fu.i;
    #endif
  }

  // top bit set if a <= b
  inline int fle(float a, float b) { return ~fgt(a, b); }

  // top bit set if a < b
  inline int flt(float a, float b) { return fgt(b, a); }

  // top bit set if a >= b
  inline int fge(float a, float b) { return ~fgt(b, a); }

  // top bit set if a == b
  inline int feq(float a, float b) { return ~(fgt(b, a) | fgt(a, b)); }

  // top bit set if a != b
  inline int fne(float a, float b) { return fgt(b, a) | fgt(a, b); }

  // return sel < 0 ? t : f
  inline float fsel(int sel, float t, float f) {
    #if OCTET_SSE
      union { __m128 m; float f; int i; } fum, fua, fub;
      fua.f = f;
      fub.f = t;
      fum.i = sel;
      __m128 b = _mm_and_ps( fua.m, fum.m );
      __m128 a = _mm_andnot_ps( fum.m, fub.m );
      fum.m =_mm_or_ps( a, b );
      return fum.f;
    #else
      union { float f; int i; } fua, fub;
      fua.f = f;
      fub.f = t;
      int mask = sel >> 31; // all 1s or 0s
      fua.i = ( fua.i & ~mask ) | ( fub.i & mask );
      return fua.f;
    #endif
  }

  inline float abs(float f) {
    return fabsf(f);
  }

  inline float sin(float f) {
    return sinf(f);
  }

  inline float cos(float f) {
    return cosf(f);
  }

  inline float sqrt(float f) {
    return sqrtf(f);
  }

  inline float rsqrt(float f) {
    return 1.0f/sqrtf(f);
  }

  inline float recip(float f) {
    return 1.0f/f;
  }

  inline float atan2(float dy, float dx) {
    return atan2f(dy, dx);
  }

  inline bool is_power_of_two(unsigned i) {
    return i != 0 && ( i & (i-1) ) == 0;
  }

  inline bool is_power_of_two(float f) {
    union { float f; int i; } fu;
    fu.f = f;
    // make sure mantissa is zero.
    return f != 0 && (fu.i & 0x007fffff) == 0;
  }

  template <class T> void swap(T &a, T &b) {
    T t = a; a = b; b = t;
  }

  template <class T> T min(const T &a, const T &b) {
    return a < b ? a : b;
  }

  template <class T> T max(const T &a, const T &b) {
    return a > b ? a : b;
  }
}

