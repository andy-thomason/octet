////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
//
// zip deflate format decoder
// 
namespace octet {
  class zip_decoder {
  public:
    zip_decoder() {
    }

    void decode(uint8_t *dest, const uint8_t *src, const uint8_t *src_max) {
      unsigned acc = 0;
      unsigned bits = 0;
      unsigned prev_code = ~0;

      while (src < src_max) {
        unsigned byte = *src++;
        acc |= byte << bits;
        bits += 8;
      }
    }
  };
}

