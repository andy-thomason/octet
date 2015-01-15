////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// scalar operations
//

namespace octet { namespace math {
  #if OCTET_SSE
    union u_m128_f4 { float v[4]; __m128 m; };
    union u_m128_i4 { int v[4]; __m128 m; };
  #endif

  /// cheaty floating point compares
  /// not exactly ieee!

  /// top bit set (ie negative) if a > b and returns zero if they equal
  inline int fgt(float a, float b) {
    /// note we have to use a union because of GCC's
    /// type based alias analysis interpretation.
    #if OCTET_SSE
      union { __m128 m; float f; int i; } fur, fua, fub;
      /// negative numbers are 1.......
      fua.f = a;
      fub.f = b;
      fur.m = _mm_cmple_ps(fua.m, fub.m);
      return fur.i;
    #else
      union { float f; int i; } fu;
      /// negative numbers are 1.......
      fu.f = b - a;
      return fu.i;
    #endif
  }

  /// top bit set if a <= b
  inline int fle(float a, float b) { return ~fgt(a, b); }

  /// top bit set if a < b
  inline int flt(float a, float b) { return fgt(b, a); }

  /// top bit set if a >= b
  inline int fge(float a, float b) { return ~fgt(b, a); }

  /// top bit set if a == b
  inline int feq(float a, float b) { return ~(fgt(b, a) | fgt(a, b)); }

  /// top bit set if a != b
  inline int fne(float a, float b) { return fgt(b, a) | fgt(a, b); }

  /// return sel < 0 ? t : f
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
      int mask = sel >> 31; /// all 1s or 0s
      fua.i = ( fua.i & ~mask ) | ( fub.i & mask );
      return fua.f;
    #endif
  }

  /// absolute value of f
  inline float abs(float f) {
    return fabsf(f);
  }

  /// absolute value of v
  inline int abs(int v) {
    return v < 0 ? -v : v;
  }

  /// sin of f
  inline float sin(float f) {
    return sinf(f);
  }

  /// cosine of f
  inline float cos(float f) {
    return cosf(f);
  }

  /// square root of f
  inline float sqrt(float f) {
    return sqrtf(f);
  }

  /// reciprocal square root of f
  inline float rsqrt(float f) {
    return 1.0f/sqrtf(f);
  }

  /// reciprocal of f
  inline float recip(float f) {
    return 1.0f/f;
  }

  /// f squared
  inline float squared(float f) {
    return f * f;
  }

  /// cartesian to polar conversion: atan(y/x)
  inline float atan2(float dy, float dx) {
    return atan2f(dy, dx);
  }

  /// fast cos to x^6 (+/- 0.001) angle is 2^32 * 2*Pi. ie. 180 degrees = 0x80000000
  inline float fast_cos6(uint32_t angle) {
    float x = (float)angle * 1.462918079e-9f - 3.141592653f;
    float x2 = x * x;
    float x4 = x2 * x2;
    return
      (0.9986067546f - 0.4953497028f *x2) +
      (0.3922769266E-1f - 0.9696679575E-3f*x2) * x4
    ;
  }

  /// fast sin, will schedule with fast_cos.
  inline float fast_sin6(uint32_t angle) {
    return fast_cos6(angle + 0x40000000);
  }

  /// return true if i is a power of 2
  inline bool is_power_of_two(unsigned i) {
    return i != 0 && ( i & (i-1) ) == 0;
  }

  /// return true if f is a power of 2
  inline bool is_power_of_two(float f) {
    union { float f; int i; } fu;
    fu.f = f;
    /// make sure mantissa is zero.
    return f != 0 && (fu.i & 0x007fffff) == 0;
  }

  /// big endian unaligned load
  inline static unsigned uint32_be(const uint8_t *src) {
    return (src[0] << 24) | (src[1] << 16) | (src[2] << 8) | (src[3] << 0);
    //return _byteswap_ulong(*(unsigned*)src);
  }

  /// little endian unaligned load
  inline static unsigned uint32_le(const uint8_t *src) {
    return (src[3] << 24) | (src[2] << 16) | (src[1] << 8) | (src[0] << 0);
    //return *(unsigned*)src;
  }

  /// return number of 1 bits
  inline static int pop_count(uint32_t v) {
    v = (v & 0x55555555) + ((v>>1) & 0x55555555);
    v = (v & 0x33333333) + ((v>>2) & 0x33333333);
    v = (v & 0x0f0f0f0f) + ((v>>4) & 0x0f0f0f0f);
    v = (v & 0x00ff00ff) + ((v>>8) & 0x00ff00ff);
    return (v + (v>>16)) & 0xff;
  }

  /// count leading zeros. Examples: 0xffffffff -> 0, 0x00ffffff -> 8, 0x00000000 -> 32
  inline static int clz(uint32_t v) {
    int res = 0;
    if (!(v >> 16)) { v <<= 16; res += 16; }
    if (!(v >> 24)) { v <<= 8; res += 8; }
    if (!(v >> 28)) { v <<= 4; res += 4; }
    if (!(v >> 30)) { v <<= 2; res += 2; }
    if (!(v >> 31)) { v <<= 1; res += 1; }
    if (!v) { res += 1; }
    return res;
  }

  /// floor(log(2, v))
  inline static int ilog2(uint32_t v) {
    return 31 - (int)clz(v);
  }

  /// discard odd bits and compress event bits into lower 16 bits
  inline static unsigned even_bits(unsigned a) {
    a &= 0x55555555;
    a = (a | a >> 1) & 0x33333333;
    a = (a | a >> 2) & 0x0f0f0f0f;
    a = (a | a >> 4) & 0x00ff00ff;
    return (a | a >> 8) & 0x0000ffff;
  }

  /// discard odd nibbles and compress even nibbles into lower 16 bits
  inline static unsigned low_nibbles(unsigned a) {
    a &= 0x0f0f0f0f;
    a = (a | a >> 4) & 0x00ff00ff;
    return (a | a >> 8) & 0x0000ffff;
  }

  /// a pair of objects, like std::pair
  template <typename first_t, typename second_t> class pair {
  public:
    pair() {}
    pair(first_t f, second_t s) : first(f), second(s) {}
    first_t first;
    second_t second;
  };

  #if OCTET_UNIT_TEST
    class scalar_unit_test {
    public:
      scalar_unit_test() {
        assert(clz(0xffffffff) == 0);
        assert(clz(0x00ffffff) == 8);
        assert(clz(0x00000040) == 25);
        assert(clz(0x00000000) == 32);
        assert(ilog2(1<<7) == 7);
        assert(ilog2((1<<7)+1) == 7);
        assert(ilog2((1<<7)-1) == 6);
      }
    };
    static scalar_unit_test scalar_unit_test;
  #endif
} }

