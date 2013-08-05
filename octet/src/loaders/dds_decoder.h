////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
//
// DDS file decoder - direct draw surface
//

namespace octet {
  class dds_decoder {
    // http://en.wikipedia.org/wiki/DirectDraw_Surface
    // http://www.mindcontrol.org/~hplus/graphics/dds-info/
    //

    enum {
      dds_magic = 0x20534444,

      // flags,
      ddsd_caps = 0x00000001,
      ddsd_height = 0x00000002,
      ddsd_width = 0x00000004,
      ddsd_pitch = 0x00000008,
      ddsd_pixelformat = 0x00001000,
      ddsd_mipmapcount = 0x00020000,
      ddsd_linearsize = 0x00080000,
      ddsd_depth = 0x00800000,

      //  spixelformat.dwflags
      ddpf_alphapixels = 0x00000001,
      ddpf_fourcc = 0x00000004,
      ddpf_indexed = 0x00000020,
      ddpf_rgb = 0x00000040,

      //  scaps.dwcaps1
      ddscaps_complex = 0x00000008,
      ddscaps_texture = 0x00001000,
      ddscaps_mipmap = 0x00400000,

      //  scaps.dwcaps2
      ddscaps2_cubemap = 0x00000200,
      ddscaps2_cubemap_positivex = 0x00000400,
      ddscaps2_cubemap_negativex = 0x00000800,
      ddscaps2_cubemap_positivey = 0x00001000,
      ddscaps2_cubemap_negativey = 0x00002000,
      ddscaps2_cubemap_positivez = 0x00004000,
      ddscaps2_cubemap_negativez = 0x00008000,
      ddscaps2_volume = 0x00200000,
    };

    struct dds_header {
      uint8_t magic[4];
      uint8_t size[4];
      uint8_t flags[4];
      uint8_t height[4];
      uint8_t width[4];
      uint8_t pitch_or_linear_size[4];
      uint8_t depth[4];
      uint8_t mipmap_count[4];
      uint8_t reserved1[ 44 ];

      //  DDPIXELFORMAT
      struct {
        uint8_t size[4];
        uint8_t flags[4];
        uint8_t fourcc[4];
        uint8_t rgb_bit_count[4];
        uint8_t r_bitmask[4];
        uint8_t g_bitmask[4];
        uint8_t b_bitmask[4];
        uint8_t alpha_bitmask[4];
      } pf;

      //  DDCAPS2
      struct {
        uint8_t caps1[4];
        uint8_t caps2[4];
        uint8_t ddsx[4];
        uint8_t reserved[4];
      } caps;

      uint8_t reserved2[4];
    };

    // read a pair of bytes as a little-endian value
    // this will work on the PS3 and other big-endian machines
    int le2( uint8_t val[2] )
    {
      return val[0] + val[1] * 0x100;
    }

    // read four bytes as a little-endian value
    // this will work on the PS3 and other big-endian machines
    int le4( uint8_t val[4] )
    {
      return val[0] + val[1] * 0x100 + val[2] * 0x10000 + val[3] * 0x1000000;
    }

    // rrrrrggggggbbbbb
    // rrrrrggggggbbbbb
    // 0011223344556677
    // 8899aabbccddeeff
    void hack_dxt1(uint8_t *dest, const uint8_t *src, unsigned stride, bool always_use_four_colours) {
      uint16_t color0 = src[0] + src[1] * 256;
      uint16_t color1 = src[2] + src[3] * 256;
      src += 4;

      uint8_t r0 = ( ( ( color0 >> 11 ) & 0x1f ) * (0xffff / 0x1f)) >> 8;
      uint8_t g0 = ( ( ( color0 >> 6 ) & 0x3f ) * (0xffff / 0x3f)) >> 8;
      uint8_t b0 = ( ( ( color0 >> 0 ) & 0x1f ) * (0xffff / 0x1f)) >> 8;
      uint8_t r1 = ( ( ( color1 >> 11 ) & 0x1f ) * (0xffff / 0x1f)) >> 8;
      uint8_t g1 = ( ( ( color1 >> 6 ) & 0x3f ) * (0xffff / 0x3f)) >> 8;
      uint8_t b1 = ( ( ( color1 >> 0 ) & 0x1f ) * (0xffff / 0x1f)) >> 8;

      // if it were not for the patent, some other code would go here
      dest[0] = ( r0 + r1 ) >> 1;
      dest[1] = ( g0 + g1 ) >> 1;
      dest[2] = ( b0 + b1 ) >> 1;
      dest[3] = 0xff;
    }

    // quantised alpha
    void hack_dxt3(uint8_t *dest, const uint8_t *src, unsigned stride) {
      dest[3] = ( src[0] + src[1] ) >> 1;
    }

    // interpolated alpha
    void hack_dxt5(uint8_t *dest, const uint8_t *src, unsigned stride) {
      dest[3] = ( src[0] + src[1] ) >> 1;
    }

    // see http://www.opengl.org/registry/specs/EXT/texture_compression_s3tc.txt
    // We can't decode s3 data as there is a patent. If you were foolish enough to use a DDS file,
    // you can expect lousy resolution!
    void hack_s3(dynarray<uint8_t> &full_image, unsigned dxt, unsigned width, unsigned height, const uint8_t *src, const uint8_t *src_max) {
      unsigned max_bytes = (width) * (height) * 8;
      if (dxt != 1) max_bytes *= 2;
      //if (src_max - src > max_bytes) return; // oops

      for (unsigned y = 0; y != height; ++y) {
        uint8_t *dest = (uint8_t *)( &full_image[0] + y * (width * 4) );
        for (unsigned x = 0; x != width; ++x) {
          if (dxt == 1) {
            hack_dxt1(dest, src+8, width * 4, false);
            src += 8;
          } else if (dxt <= 3) {
            hack_dxt1(dest, src+8, width * 4, true);
            hack_dxt3(dest, src, width * 4);
            src += 16;
          } else {
            hack_dxt1(dest, src+8, width * 4, true);
            hack_dxt5(dest, src, width * 4);
            src += 16;
          }
          dest += 4;
        }
      }
    }

  public:
    // get an opengl texture from a file in memory
    void get_image(dynarray<uint8_t> &full_image, uint32_t &num_components, uint32_t &width, uint32_t &height, const uint8_t *src, const uint8_t *src_max) {
      // convert the data
      dds_header *header = (dds_header*)src;

      if (le4(header->magic) != dds_magic) return;


      unsigned pf_flags = le4(header->pf.flags);

      if (pf_flags & ddpf_fourcc) {
        uint8_t *fourcc = header->pf.fourcc;
        if (fourcc[0] == 'D' && fourcc[1] == 'X' && fourcc[2] == 'T') {
          // as S3TC is patented, we can't put a decoder in open source code.
          // this is a hack that gets a 1/4 res image without using the S3 algorithm.
          width = le4(header->width)/4;
          height = le4(header->height)/4;
          num_components = 4;

          full_image.resize(width * height * 4);
          hack_s3(full_image, fourcc[3]-'0', width, height, src + 128, src_max);
          return;
        }
      }
      printf("warning: DDS decoder only supports DXTn\n");
    }
  };
}
