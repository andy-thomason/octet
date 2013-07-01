////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Random number generator
//
//

class random {
  int seed;
public:
  random(unsigned new_seed = 0x9bac7615) : seed(new_seed) {
    seed = new_seed;
  }

  // get a floating point value
  float get(float min, float max) {
    // todo: test for period 
    seed = ( ( seed >> 31 ) & 0xa67b9c35 ) ^ ( seed << 1 );
    return min + ( ( seed >> 8 ) & 0xffff ) * ( ( max - min ) / 0xffff );
  }

  // get an int value in a small range
  int get(int min, int max) {
    assert(max - min <= 0xffff);
    seed = ( ( seed >> 31 ) & 0xa67b9c35 ) ^ ( seed << 1 );
    return min + ( ( seed >> 8 ) & 0xffff ) * ( max - min ) / 0xffff;
  }
};
