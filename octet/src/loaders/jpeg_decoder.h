////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
//
// jpeg file decoder - tiny and fast
//
// See http://en.wikipedia.org/wiki/JPEG
// 
namespace octet { namespace loaders {
  class jpeg_decoder {
    enum { debug = 0 };

    // image dimensions
    unsigned precision;
    unsigned width;
    unsigned height;
    unsigned num_components;

    // What kind of image
    unsigned sof_code;

    // progressive parameters
    unsigned spectral_start;
    unsigned spectral_end;
    unsigned successive_high;
    unsigned successive_low;

    // how many blocks in a MCU (see mcu_block below)
    unsigned num_mcu_blocks;
    unsigned num_components_in_scan;

    // skip a number of bits in the file.
    // there is a special case where every 0xff byte is followed by 0x00
    static void skip_bits(unsigned bits, unsigned &acc, const uint8_t *&src, int &shift) {
      shift -= bits;
      while (shift < 0) {
        // grab more bytes
        uint8_t byte = *src++;
        acc = acc * 256 + byte;
        if (byte == 0xff) {
          // in JPEG, an 0xff byte is followed by a zero
          // do not advance past any other 0xff marker
          src += src[0] == 0x00 ? 1 : -1;
        }
        shift += 8;
      }
    }

    // this is a component usually Y (brightness), Cb (blueness) and Cr (redness)
    // from the file.
    // Some JPEGs have 2x2 blocks for Y and only 1x1 for Cb and Cr (4:2:0)
    // as you can't see colour in high resolution.
    struct component {
      uint8_t id;
      uint8_t hsamp;
      uint8_t vsamp;
      uint8_t quantisation_table;
    } components[4];

    // this is a component that is used for a particluar "scan"
    // of the image data. With progressive files there may be more than
    // one scan.
    struct scan_component {
      uint8_t comp;
      uint8_t ac_table;
      uint8_t dc_table;
      unsigned width_in_blocks;
      unsigned height_in_blocks;
      int last_dc;
    } scan_components[4];

    // quantisation table. We multiply the dc and ac coefficients by these numbers.
    // this is the lossy part of the compression
    struct quant_table {
      float table[64];
    } quant_tables[4];

    // A huffman table maps variable length codes to lengths and values.
    // for example. 00 010 011 100 1010 1011 1100 1110 1111 might be a huffman code
    // where each code is distinct from the previous one, even if it has more bits.
    // (ie. 100(0) and 100(1) are less than 1010).
    struct huffman_table {
      unsigned min_len;
      uint8_t huffval[257];
      uint16_t maxcodes[17];
      uint16_t offset[17];

      // decode a variable length huffman code
      // we grab the next 16 bits and look in the maxcodes table to see how many
      // bits the code has. After that, we strip the right hand bits and
      // look up the code in a table.
      unsigned decode(unsigned &acc, const uint8_t *&src, int &shift) {
        unsigned i = min_len;
        unsigned short acc16 = acc >> shift;

        // find the shortest code that this could be
        for (; acc16 > maxcodes[i]; ++i) {
        }

        unsigned code = ( acc16 >> (15-i) ) - offset[i];
        skip_bits(i + 1, acc, src, shift);
        return huffval[code];
      }
    } huffman_tables[2][4];

    // a mcu_block is an 8x8 component of a MCU
    // (Minimal coding unit). The image is tiled by MCUs
    // which have components.
    struct mcu_block {
      huffman_table *dc_table;
      huffman_table *ac_table;
      quant_table *quant;
      scan_component *scan_comp;
    } mcu_blocks[8];

    float dct_coeffs[8*64];
    float ycrcb_values[8*64];

    unsigned u2(const uint8_t *src) {
      return src[0] * 256 + src[1];
    }

    // dct coefficients are stored in zig-zag order because the top
    // left is far more common.
    uint8_t zig_zag(unsigned i) {
      static const uint8_t zig_zag_[64] = {
        0, 1, 8, 16, 9, 2, 3, 10,
        17, 24, 32, 25, 18, 11, 4, 5,
        12, 19, 26, 33, 40, 48, 41, 34,
        27, 20, 13, 6, 7, 14, 21, 28,
        35, 42, 49, 56, 57, 50, 43, 36,
        29, 22, 15, 23, 30, 37, 44, 51,
        58, 59, 52, 45, 38, 31, 39, 46,
        53, 60, 61, 54, 47, 55, 62, 63,
      };
      return i < 63 ? zig_zag_[i] : 63;
    }

    // negative numbers need to be twiddled as all numbers coming in are positive.
    static int extend(unsigned bits, unsigned &acc, const uint8_t *&src, int &shift) {
      uint16_t acc16 = acc >> shift;
      unsigned v = acc16 >> (16 - bits);
      return v < ( 1u << ( bits-1 ) ) ? (int)v + ( -1 << bits ) + 1 : (int)v;
    }

    // decode one block of an MCU which may contain many blocks
    // The Y component may have four blocks, for example, and only one each of Cr, Cb
    void decode_mcu_block(unsigned block_num, unsigned &acc, const uint8_t *&src, int &shift, float *outptr) {
      mcu_block &block = mcu_blocks[block_num];

      unsigned value = block.dc_table->decode(acc, src, shift);

      int dc = 0;
      if (value) {
        dc = extend(value, acc, src, shift);
        skip_bits(value, acc, src, shift);
        //if (debug) printf("dc=%d\n", dc);
      }
      int abs_dc = block.scan_comp->last_dc += dc;
      outptr[0] = abs_dc * block.quant->table[0];

      for (int ac_coef = 1; ac_coef < 64; ++ac_coef) {
        unsigned value = block.ac_table->decode(acc, src, shift);
        unsigned skip = value >> 4;
        value &= 0x0f;
        ac_coef += skip;

        if (value) {
          int ac = extend(value, acc, src, shift);
          skip_bits(value, acc, src, shift);
          //if (debug) printf("ac=%d,%d coef=%d zig_zag=%d\n", skip, ac, ac_coef, zig_zag(ac_coef));
          outptr[zig_zag(ac_coef)] = (float)ac * block.quant->table[ac_coef];
        } else if (skip != 15) {
          break;
        }
      }
      if (debug) {
        for (int j = 0; j != 8; ++j) {
          for (int i = 0; i != 8; ++i) {
            printf("%3.0f ", outptr[i+j*8]);
          }
          printf("\n");
        }
      }
    }

    // one dimensional inverse DCT.
    // c0 is the DC term and c1..c7 increase in frequency
    // example: c0 = 128, c1..c7 = 0 -> 128, 128, 128, 128, 128, 128, 128, 128
    // todo: improve this!
    OCTET_HOT void idct(float &c0, float &c1, float &c2, float &c3, float &c4, float &c5, float &c6, float &c7) {
      float c2c6_1 = (c2 + c6) * 0.541196100f;
      float c2c6_2 = c2c6_1 + c6 * -1.847759065f;
      float c2c6_3 = c2c6_1 + c2 * 0.765366865f;
    
      float c0c4_1 = c0 + c4;
      float c0c4_2 = c0 - c4;
    
      float ceven_1 = c0c4_1 + c2c6_3;
      float ceven_2 = c0c4_1 - c2c6_3;
      float ceven_3 = c0c4_2 + c2c6_2;
      float ceven_4 = c0c4_2 - c2c6_2;
    
      float c1c7 = c7 + c1;
      float c3c5 = c5 + c3;
      float c7c3 = c7 + c3;
      float c5c1 = c5 + c1;
      float codd_0 = (c7c3 + c5c1) * 1.175875602f;
    
      float codd_4 = c7 * 0.298631336f;
      float codd_3 = c5 * 2.053119869f;
      float codd_2 = c3 * 3.072711026f;
      float codd_1 = c1 * 1.501321110f;
      c1c7 = c1c7 * -0.899976223f;
      c3c5 = c3c5 * -2.562915447f;
      c7c3 = c7c3 * -1.961570560f;
      c5c1 = c5c1 * -0.390180644f;
    
      c7c3 += codd_0;
      c5c1 += codd_0;
    
      codd_4 += c1c7 + c7c3;
      codd_3 += c3c5 + c5c1;
      codd_2 += c3c5 + c7c3;
      codd_1 += c1c7 + c5c1;
    
      c0 = ceven_1 + codd_1;
      c7 = ceven_1 - codd_1;
      c1 = ceven_3 + codd_2;
      c6 = ceven_3 - codd_2;
      c2 = ceven_4 + codd_3;
      c5 = ceven_4 - codd_3;
      c3 = ceven_2 + codd_4;
      c4 = ceven_2 - codd_4;
    }

    // Two dimensional inverse DCT
    // we can do the rows and columns separately.
    // Optimisations include spotting blank rows and columns, but this
    // is just a reference design.
    void inverse_dct(float *inptr) {
      // do rows
      for (unsigned i = 0; i != 8; ++i) {
        idct(inptr[8*0+i], inptr[8*1+i], inptr[8*2+i], inptr[8*3+i], inptr[8*4+i], inptr[8*5+i], inptr[8*6+i], inptr[8*7+i]);
      }

      // do columns
      for (unsigned i = 0; i != 8; ++i) {
        idct(inptr[8*i+0], inptr[8*i+1], inptr[8*i+2], inptr[8*i+3], inptr[8*i+4], inptr[8*i+5], inptr[8*i+6], inptr[8*i+7]);
      }

      if (debug) {
        for (int j = 0; j != 8; ++j) {
          for (int i = 0; i != 8; ++i) {
            printf("%f ", inptr[i+j*8]);
          }
          printf("\n");
        }
      }
    }

    // clamp to 0..255 range without using branches.
    // fabsf is usually implemented in hardware (with fast math options)
    OCTET_HOT uint8_t clamp(float v) {
      // v + fabsf(v) = 2v when v > 0
      // v + fabsf(v) = 0  when v < 0
      float clamp0 = v + fabsf(v);

      // v - fabsf(v-n) = n when v > n
      // v - fabsf(v-n) = 2v - n when v < n
      return (uint8_t)( ( clamp0 - fabsf( clamp0 - (255.999f * 2) ) ) * 0.25f + 128 );
    }

    // convert from Y to RGB
    // The 0.125 scaling factor is because the DCT data has a scale of 8
    void color_convert_444_greyscale(uint8_t *outptr, int stride, float *inptr) {
      for (unsigned j = 0; j != 8; ++j) {
        for (unsigned i = 0; i != 8; ++i) {
          float y = inptr[0];
          inptr++;
          outptr[0] = clamp(128 + y * 0.125f);
          outptr[1] = clamp(128 + y * 0.125f);
          outptr[2] = clamp(128 + y * 0.125f);
          outptr[3] = 0xff;
          outptr += 4;
        }
        outptr += stride - 32;
      }
    }

    // convert from YCrCb to RGB
    // See http://en.wikipedia.org/wiki/YCbCr
    // The 0.125 scaling factor is because the DCT data has a scale of 8
    void color_convert_444(uint8_t *outptr, int stride, float *inptr) {
      for (unsigned j = 0; j != 8; ++j) {
        for (unsigned i = 0; i != 8; ++i) {
          float y = inptr[0];
          float cb = inptr[64];
          float cr = inptr[128];
          inptr++;
          outptr[0] = clamp(128 + y * 0.125f + cr * (1.402f * 0.125f));
          outptr[1] = clamp(128 + y * 0.125f - cb * (0.34414f * 0.125f) - cr * (0.71414f * 0.125f));
          outptr[2] = clamp(128 + y * 0.125f + cb * (1.772f * 0.125f));
          outptr[3] = 0xff;
          outptr += 4;
        }
        outptr += stride - 32;
      }
    }

    // convert from YCrCb to RGB
    // See http://en.wikipedia.org/wiki/YCbCr
    // The 0.125 scaling factor is because the DCT data has a scale of 8
    void color_convert_411(uint8_t *outptr, int stride, float *inptr) {
      for (unsigned j = 0; j != 8; ++j) {
        for (unsigned i = 0; i != 8; ++i) {
          float y0 = inptr[0x00];
          float y1 = inptr[0x40];
          float y2 = inptr[0x80];
          float y3 = inptr[0xc0];
          float cb = inptr[0x100];
          float cr = inptr[0x140];
          inptr++;
          outptr[0] = clamp(128 + y0 * 0.125f + cr * (1.402f * 0.125f));
          outptr[1] = clamp(128 + y0 * 0.125f - cb * (0.34414f * 0.125f) - cr * (0.71414f * 0.125f));
          outptr[2] = clamp(128 + y0 * 0.125f + cb * (1.772f * 0.125f));
          outptr[3] = 0xff;
          outptr += 4;
          outptr[0] = clamp(128 + y1 * 0.125f + cr * (1.402f * 0.125f));
          outptr[1] = clamp(128 + y1 * 0.125f - cb * (0.34414f * 0.125f) - cr * (0.71414f * 0.125f));
          outptr[2] = clamp(128 + y1 * 0.125f + cb * (1.772f * 0.125f));
          outptr[3] = 0xff;
          outptr += stride - 4;
          outptr[0] = clamp(128 + y2 * 0.125f + cr * (1.402f * 0.125f));
          outptr[1] = clamp(128 + y2 * 0.125f - cb * (0.34414f * 0.125f) - cr * (0.71414f * 0.125f));
          outptr[2] = clamp(128 + y2 * 0.125f + cb * (1.772f * 0.125f));
          outptr[3] = 0xff;
          outptr += 4;
          outptr[0] = clamp(128 + y3 * 0.125f + cr * (1.402f * 0.125f));
          outptr[1] = clamp(128 + y3 * 0.125f - cb * (0.34414f * 0.125f) - cr * (0.71414f * 0.125f));
          outptr[2] = clamp(128 + y3 * 0.125f + cb * (1.772f * 0.125f));
          outptr[3] = 0xff;
          outptr += -stride + 4;
        }
        outptr += stride*2 - 64;
      }
    }

    // JPEG files are split up into chunks starting with 0xff
    unsigned decode_chunk(const uint8_t *src, dynarray<uint8_t> &image, uint16_t &format) {
      if (debug) printf("decode_chunk %02x\n", src[1]);

      unsigned length = 2;

      switch (src[1]) {
        // different kinds of image (SOF0-7)
        case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc5: case 0xc6: case 0xc7: {
          sof_code = src[1];
          length = u2(src + 2) + 2;
          precision = src[4];
          height = u2(src + 5);
          width = u2(src + 7);
          num_components = src[9];

          if (src[1] != 0xc0) {
            printf("warning: only baseline JPEG is supported - disable progressive\n");
            return 0;
          }

          if (precision != 8 || width == 0 || height == 0 || num_components > 4) {
            printf("warning: precision=%d width=%d height=%d num_components=%d\n", precision, width, height, num_components);
            return 0;
          }

          if (debug) printf("SOF w=%d h=%d nc=%d\n", width, height, num_components);

          // ycrcb only
          if (num_components != 1 && num_components != 3) {
            printf("warning: num_components=%d\n", num_components);
            return 0;
          }

          for (unsigned i = 0; i != num_components; ++i) {
            component &c = components[i];
            c.id = src[10 + i*3 + 0];
            c.hsamp = src[10 + i*3 + 1] >> 4;
            c.vsamp = src[10 + i*3 + 1] & 15;
            c.quantisation_table = src[10 + i*3 + 2] & 3;
            if (debug) printf("id=%d h=%d v=%d q=%d\n", c.id, c.hsamp, c.vsamp, c.quantisation_table);
          }
          
        } break;

        // huffman tables
        case 0xc4: {
          length = u2(src + 2) + 2;
          src += 4;
          const uint8_t *src_max = src + length;
          while (src + 17 <= src_max) {
            unsigned index = src[0];
            unsigned is_ac = (index >> 4) & 1;
            index &= 3;
            huffman_table &h = huffman_tables[is_ac][index];
            const uint8_t *num_codes = src + 1;
            unsigned count = 0;
            for (unsigned i = 0; i != 16; ++i) {
              count += num_codes[i];
            }
            src += 17;
            if (src + count > src_max || count > 256) return 0;
            memcpy(h.huffval, src, count);
            src += count;

            unsigned dest = 0;
            unsigned code = 0;
            h.min_len = 0;
            bool done_min_len = false;
            for (unsigned len = 1; len < 17; ++len) {
              h.offset[len-1] = code - dest;
              if (!done_min_len && num_codes[len-1]) {
                h.min_len = len - 1;
                done_min_len = true;
              }
              for (unsigned i = 0; i != num_codes[len-1]; ++i) {
                if (debug) printf("code=%04x len=%d\n", ( ( code + i ) << (16 - len) ), len );
              }
              dest += num_codes[len-1];
              code = code + num_codes[len-1];
              h.maxcodes[len-1] = ( code << (16 - len) ) - 1;
              code *= 2;
              if (debug) printf("h.maxcodes[%d] = %04x\n", len-1, h.maxcodes[len-1]);
            }
            h.maxcodes[16] = 0xffff;
            
            if (debug) printf("DHT %d\n", index);
          }
        } break;

        // start
        case 0xd8: {
          if (debug) printf("SOI\n");
        } break;

        // end
        case 0xd9: {
          if (debug) printf("EOI\n");
        } break;

        // image data
        case 0xda: {
          const uint8_t *src0 = src;
          length = u2(src + 2) + 2;
          src += 4;
          num_components_in_scan = *src++;
          unsigned max_hsamp = 1;
          unsigned max_vsamp = 1;
          num_mcu_blocks = 0;
          const uint8_t *src_max = src + length;
          for (unsigned i = 0; i != num_components_in_scan; ++i) {
            scan_component &sc = scan_components[i];
            unsigned id = *src++;
            sc.ac_table = *src & 0x0f;
            sc.dc_table = *src++ >> 4;
            unsigned comp = 0;
            while (comp < num_components) {
              if (components[comp].id == id) break;
              comp++;
            }
            if (comp >= num_components) return 0;
            component &c = components[comp];
            max_hsamp = c.hsamp > max_hsamp ? c.hsamp : max_hsamp;
            max_vsamp = c.vsamp > max_vsamp ? c.vsamp : max_vsamp;
            sc.comp = comp;
            if (debug) printf("SOS comp=%d ac=%d dc=%d\n", comp, sc.ac_table, sc.dc_table);
            unsigned samps = c.hsamp * c.vsamp;

            if (num_mcu_blocks + samps > sizeof(mcu_blocks)/sizeof(mcu_blocks[0])) {
              printf("too many mcu blocks\n");
              return 0;
            }

            for (unsigned j = 0; j != samps; ++j) {
              mcu_block &m = mcu_blocks[num_mcu_blocks++];
              m.dc_table = &huffman_tables[0][sc.dc_table];
              m.ac_table = &huffman_tables[1][sc.ac_table];
              m.quant = &quant_tables[c.quantisation_table];
              m.scan_comp = &sc;
            }

            sc.last_dc = 0;
          }

          // at present, we only support YCrCb in 4:4:4
          if (num_mcu_blocks != 1 && num_mcu_blocks != 3 && num_mcu_blocks != 6) {
            printf("only 4:4:4 and 4:1:1 greyscale and ycrcb supported (%d mcu blocks)\n", num_mcu_blocks);
            return 0;
          }

          spectral_start = *src++;
          spectral_end = *src++;
          successive_high = src[0] >> 4;
          successive_low = *src++ & 0x0f;
          if (src > src_max) return 0;

          for (unsigned i = 0; i != num_components_in_scan; ++i) {
            scan_component &sc = scan_components[i];
            component &c = components[sc.comp];
            sc.width_in_blocks = width * c.hsamp / max_hsamp;
            sc.height_in_blocks = height * c.hsamp / max_hsamp;
          }

          width = (width + max_hsamp * 8 - 1) & ~(max_hsamp * 8 - 1);
          height = (height + max_vsamp * 8 - 1) & ~(max_vsamp * 8 - 1);

          unsigned xmax = ( width + max_hsamp * 8 - 1 ) / (max_hsamp * 8);
          unsigned ymax = ( height + max_vsamp * 8 - 1 ) / (max_vsamp * 8);

          unsigned acc = 0;
          int shift = 0;
          skip_bits(16, acc, src, shift);
          
          int stride = width * 4;

          unsigned size = width * height * 4;
          size_t base = image.size();
          image.resize(base + size);
          format = 0x1908; // GL_RGBA

          uint8_t *image_base = image.data() + base;

          for (unsigned y = 0; y != ymax; ++y) {
            for (unsigned x = 0; x != xmax; ++x) {
              float *coeffs = dct_coeffs;
              memset(coeffs, 0, 64 * num_mcu_blocks * sizeof(float));
              for (unsigned b = 0; b < num_mcu_blocks; ++b) {
                decode_mcu_block(b, acc, src, shift, coeffs);
                inverse_dct(coeffs);
                coeffs += 64;
              }
              if (num_mcu_blocks == 1) {
                // assume 4:4:4 Greyscale
                color_convert_444_greyscale(&image_base[((height - 1 - y * 8) * stride) + (x * 8 * 4)], -stride, dct_coeffs);
              } else if (num_mcu_blocks == 3) {
                // assume 4:4:4 YCbCr
                color_convert_444(&image_base[((height - 1 - y * 8) * stride) + (x * 8 * 4)], -stride, dct_coeffs);
              } else if (num_mcu_blocks == 6) {
                // assume 4:1:1 YCbCr
                color_convert_411(&image_base[((height - 1 - y * 16) * stride) + (x * 16 * 4)], -stride, dct_coeffs);
              }
            }
          }
          skip_bits(shift, acc, src, shift);
          length = (unsigned)(src - src0);
        } break;

        // quantisation tables (the lossy bit)
        case 0xdb: {
          length = u2(src + 2) + 2;
          const uint8_t *src_max = src + length;
          src += 4;
          while (src < src_max) {
            unsigned prec = (src[0] >> 4) & 1;
            unsigned n = src[0] & 0x0f;
            src++;
            for (unsigned i = 0; i != 64; ++i) {
              quant_tables[n&3].table[i] = (float)( prec ? u2(src) : *src );
              src += prec + 1;
            }
            if (debug) printf("DQT %d %d\n", prec, n);
          }
        } break;

        // JFIF stubset of JPEG
        case 0xe0: {
          length = u2(src + 2) + 2;
          if (debug) printf("M_APP0 (JFIF)\n");
        } break;

        // unknown chunk
        default: {
          if (src[2] != 0xff) length = u2(src + 2) + 2;
          if (debug) printf("unknown\n");
        } break;
      }
      return length;
    }
  public:
    // get an opengl texture from a file in memory
    void get_image(dynarray<uint8_t> &image, uint16_t &format, uint16_t &width_, uint16_t &height_, const uint8_t *src, const uint8_t *src_max) {
      while (src < src_max) {
        if (src[0] != 0xff) {
          printf("warning: bad JPEG file\n");
          return;
        }
        unsigned length = decode_chunk(src, image, format);
        if (!length) {
          printf("warning: bad JPEG file @ chunk %02x\n", src[1]);
          return;
        }
        src += length;
      }
      width_ = width;
      height_ = height;
      num_components = 3;
    }
  };
}}

