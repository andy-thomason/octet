////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Rational number
//
// Using rational numbers helps us to avoid divides and bad things
// that happen around them.
//

namespace octet { namespace math {
  class rational {
    static const char *Copyright() { return "Copyright(C) Andy Thomason 2011-2013"; }
    // value = n / d
    float n;
    float d;
  public:
    // default constructor: note does not initialize!
    rational() {
    }

    // construct from scalars
    rational(float n, float d=1) {
      this->n = n;
      this->d = d;
    };


    // vector - scalar operators

    // n / d + x = ( n + x * d ) / d
    rational operator+(float x) const {
      return rational(n + x * d, d);
    }

    // n / d - x = ( n - x * d ) / d
    rational operator-(float x) const {
      return rational(n - x * d, d);
    }

    // n / d * x = ( n * x ) / d
    rational operator*(float x) const {
      return rational(n * x, d);
    }

    // n / d / x = n / (d * x)
    rational operator/(float x) const {
      return rational(n, d * x);
    }

    // fraction operators
    rational operator+(const rational &r) const {
      // n / d + r.n / r.d = ( n * r.d + r.n * d ) / d  * r.d
      return rational(n * r.d + r.n * d, d  * r.d);
    }

    rational operator-(const rational &r) const {
      // n / d - r.n / r.d = ( n * r.d - r.n * d ) / d  * r.d
      return rational(n * r.d - r.n * d, d  * r.d);
    }

    rational operator*(const rational &r) const {
      // n / d * r.n / r.d = ( n * r.n ) / ( d * r.d )
      return rational(n * r.n, d * r.d);
    }

    // negative
    rational operator-() const {
      return rational(-n, d);
    }

    // the thing on top
    float numer() const {
      return n;
    }

    // the thing below
    float denom() const {
      return d;
    }

    // avoid this!
    operator float() {
      // divide is very expensive and likely to cause NANs
      return n / d;
    }

    // avoid rationals overflowing both exponents by shrinking
    // denominator exponent. This also makes the denominator positve.
    rational normalize() const {
      union { float f; int i; } factor;
      factor.f = d;
      if (factor.i & 0x7fffffff) {
        factor.i ^= 0x7f800000; // approx(1/d)
      }
      return rational( n * factor.f, d * factor.f );
    }

    // return negative if *this < r
    int lt(const rational &r) const {
      rational lhs = normalize();
      rational rhs = r.normalize();
      return flt(lhs.n * rhs.d, rhs.n * lhs.d);
    }

    // return negative if *this < r
    int le(const rational &r) const {
      rational lhs = normalize();
      rational rhs = r.normalize();
      return fle(lhs.n * rhs.d, rhs.n * lhs.d);
    }

    // return negative if *this < r
    int gt(const rational &r) const {
      rational lhs = normalize();
      rational rhs = r.normalize();
      return fgt(lhs.n * rhs.d, rhs.n * lhs.d);
    }

    // return negative if *this < r
    int ge(const rational &r) const {
      rational lhs = normalize();
      rational rhs = r.normalize();
      return fge(lhs.n * rhs.d, rhs.n * lhs.d);
    }

    rational min(const rational &r) const {
      rational lhs = normalize();
      rational rhs = r.normalize();
      // note: infinities always return false with < and >
      int z = fgt(lhs.n * rhs.d, rhs.n * lhs.d);
      return rational(fsel(z, rhs.n, lhs.n), fsel(z, rhs.d, lhs.d));
    }

    rational max(const rational &r) const {
      rational lhs = normalize();
      rational rhs = r.normalize();
      int z = flt(lhs.n * rhs.d, rhs.n * lhs.d);
      return rational(fsel(z, rhs.n, lhs.n), fsel(z, rhs.d, lhs.d));
    }

    rational min(const rational &r, const rational &r2) const {
      rational lhs = normalize();
      rational rhs = r.normalize();
      rational rhs2 = r2.normalize();
      int z = fgt(lhs.n * rhs.d, rhs.n * lhs.d);
      rational tmin(fsel(z, rhs.n, lhs.n), fsel(z, rhs.d, lhs.d));
      int z2 = fgt(tmin.n * rhs2.d, rhs2.n * tmin.d);
      return rational(fsel(z2, rhs.n, tmin.n), fsel(z2, rhs.d, tmin.d));
    }

    rational max(const rational &r, const rational &r2) const {
      rational lhs = normalize();
      rational rhs = r.normalize();
      rational rhs2 = r2.normalize();
      int z = flt(lhs.n * rhs.d, rhs.n * lhs.d);
      rational tmax(fsel(z, rhs.n, lhs.n), fsel(z, rhs.d, lhs.d));
      int z2 = flt(tmax.n * rhs2.d, rhs2.n * tmax.d);
      return rational(fsel(z2, rhs.n, tmax.n), fsel(z2, rhs.d, tmax.d));
    }

    rational abs() const {
      return rational( fabsf(n), fabsf(d) );
    }

    // access the floating point numbers
    float *get() { return &n; }

    // get the floating point numbers
    const float *get() const { return &n; }

    // convert to a string (up to 4 strings can be included at a time)
    const char *toString() const
    {
      char *dest = get_sprintf_buffer();
      sprintf(dest, "[%f, %f]", n, d);
      return dest;
    }
  };

  // return negative if *this < r
  int lt(const rational &l, const rational &r) {
    return l.lt(r);
  }

  // return negative if *this < r
  int le(const rational &l, const rational &r) {
    return l.le(r);
  }

  // return negative if *this < r
  int gt(const rational &l, const rational &r) {
    return l.gt(r);
  }

  // return negative if *this < r
  int ge(const rational &l, const rational &r) {
    return l.ge(r);
  }

  rational min(const rational &l, const rational &r) {
    return l.min(r);
  }

  rational max(const rational &l, const rational &r) {
    return l.max(r);
  }

  rational min(const rational &l, const rational &r, const rational &r2) {
    return l.min(r, r2);
  }

  rational max(const rational &l, const rational &r, const rational &r2) {
    return l.max(r, r2);
  }

  rational abs(const rational &l) {
    return l.abs();
  }
} }
