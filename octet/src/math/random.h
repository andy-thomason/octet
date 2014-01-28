////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Random number generator
//
//
namespace octet { namespace math {
  class random {
    int seed;
  public:
    random(unsigned new_seed = 0x9bac7615) : seed(new_seed) {
      seed = new_seed;
    }

    void set_seed(unsigned new_seed) {
      seed = new_seed;
    }

    // get a floating point value
    float get(float min, float max) {
      // todo: test for period 
      seed = ( ( seed >> 31 ) & 0xa67b9c35 ) ^ ( seed << 1 );
      seed = ( ( seed >> 31 ) & 0xcb73194c ) ^ ( seed << 1 );
      return min + ( ( seed >> 8 ) & 0xffff ) * ( ( max - min ) / 0xffff );
    }

    // get an int value in a small range
    int get(int min, int max) {
      assert(max - min <= 0x7fff);
      seed = ( ( seed >> 31 ) & 0xa67b9c35 ) ^ ( seed << 1 );
      return min + ( ( seed >> 8 ) & 0xffff ) * ( max - min ) / 0xffff;
    }

    // get an value between 0 and 0xffff
    unsigned get0xffff() {
      seed = ( ( seed >> 31 ) & 0xa67b9c35 ) ^ ( seed << 1 );
      return ( ( seed >> 8 ) & 0xffff );
    }
  };
} }
