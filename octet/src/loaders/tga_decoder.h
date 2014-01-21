////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
//
// tga file decoder - only the most common variants
//
// TGA files are used for artwork generation but almost never for production
// as they are very large (you should use GIF files or other compressed formats by choice)
// 

namespace octet { namespace loaders {
  class tga_decoder {
    typedef unsigned char uint8_t;
    // this is the TGA header
    // http://en.wikipedia.org/wiki/Truevision_TGA
    //
    struct TgaHeader
    {
      uint8_t identsize;          // size of ID field that follows 18 uint8_t header (0 usually)
      uint8_t colourmaptype;      // type of colour map 0=none, 1=has palette
      uint8_t imagetype;          // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

      uint8_t colourmapstart[2];     // first colour map entry in palette
      uint8_t colourmaplength[2];    // number of colours in palette
      uint8_t colourmapbits;      // number of bits per palette entry 15,16,24,32

      uint8_t xstart[2];             // image x origin
      uint8_t ystart[2];             // image y origin
      uint8_t width[2];              // image width in pixels
      uint8_t height[2];             // image height in pixels
      uint8_t bits;               // image bits per pixel 8,16,24,32
      uint8_t descriptor;         // image descriptor bits (vh flip bits)
    };

    // read a pair of bytes as a little-endian value
    int le2( uint8_t val[2] )
    {
      return val[0] + val[1] * 0x100;
    }
  public:
    // get an opengl texture from a file in memory
    void get_image(dynarray<uint8_t> &image, uint16_t &format, uint16_t &width, uint16_t &height, const uint8_t *src, const uint8_t *src_max) {
      // convert the data
      TgaHeader *header = (TgaHeader*)src;
      const uint8_t *data = (uint8_t *)src + sizeof(TgaHeader);
    
      width = le2(header->width);
      height = le2(header->height);

      // make sure this is the GIMP flavour  
      assert(header->identsize == 0);
      assert(header->colourmaptype == 0);
      assert(header->imagetype == 2);
      //assert(header->descriptor == 0);
      assert(header->bits == 32 || header->bits == 24);
    
      unsigned num_components = header->bits / 8;

      unsigned size = width * height * num_components;
      image.resize(size);
      format = num_components == 3 ? 0x1907 : 0x1908; // GL_RGB / GL_RGBA

      uint8_t *dest = &image[0];

      if (num_components == 4) {
        // swap red and blue!
        for (int y = 0; y != height; ++y)
        {
          const uint8_t *src = data + y * width * num_components;
          for (int x = 0; x != width; ++x)
          {
            uint8_t alpha = src[ x*4 + 3 ];
            uint8_t red = src[ x*4 + 2 ];
            uint8_t green = src[ x*4 + 1 ];
            uint8_t blue = src[ x*4 + 0 ];
            dest[0] = red;
            dest[1] = green;
            dest[2] = blue;
            dest[3] = alpha;
            dest += 4;
          }
        }
      } else {
        for (int y = 0; y != height; ++y)
        {
          const uint8_t *src = data + y * width * num_components;
          for (int x = 0; x != width; ++x)
          {
            uint8_t red = src[ x*3 + 2 ];
            uint8_t green = src[ x*3 + 1 ];
            uint8_t blue = src[ x*3 + 0 ];
            dest[0] = red;
            dest[1] = green;
            dest[2] = blue;
            dest += 3;
          }
        }
      }
    }
  };
}}
