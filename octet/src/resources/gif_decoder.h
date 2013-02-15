////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
//
// gif file decoder
// 
class gif_decoder {
  unsigned short lzw_head[0x1001];
  unsigned char lzw_tail[0x1001];
  enum { debug_gif = 0 };

  // add a lzw string for a specific code.
  // all strings consist of a head and a single byte tail.
  // we follow the chains to get the whole string.
  unsigned char add_lwz_string(unsigned code, unsigned char *&bytes, unsigned char *max_bytes, unsigned reset_code) {
    unsigned i = code;
    unsigned num_bytes = 1;
    for (; i > reset_code; i = lzw_head[i]) {
      num_bytes++;
    }

    unsigned char *dest = bytes += num_bytes;
    if (bytes > max_bytes) return 0;
    i = code;
    for (; i > reset_code; i = lzw_head[i]) {
      *--dest = lzw_tail[i];
    }
    *--dest = i;

    if (debug_gif) {
      for (unsigned i = 0; i != num_bytes; ++i) {
        printf("out %02x\n", dest[i]);
      }
    }    
    return i;
  }

  // decode image data from a gif file as a lzw coding of palette values
  bool gif_decode_bytes(unsigned char *bytes, unsigned char *max_bytes, int min_lzw_size, const unsigned char *&srcref) {
    const unsigned char *src = srcref;
    unsigned lzw_size = min_lzw_size + 1;
    unsigned reset_code = ( 1 << min_lzw_size );
    unsigned mask = reset_code * 2 - 1;
    unsigned cur_code = reset_code + 1;

    unsigned acc = 0;
    unsigned bits = 0;
    unsigned prev_code = ~0;

    while (*src) {
      unsigned len = *src++;
      do {
        unsigned byte = *src++;
        acc |= byte << bits;
        bits += 8;
        if (debug_gif) printf("byte %02x acc=%08x bits=%d\n", byte, acc, bits);
        while (bits >= lzw_size) {
          unsigned code = acc & mask;
          if (debug_gif) printf("code=%03x\n", code);
          bits -= lzw_size;
          acc >>= lzw_size;
          if ((code & ~1) == reset_code) {
            if (code == reset_code+1) {
              // end
              break;
            } else {
              // reset
              lzw_size = min_lzw_size + 1;
              cur_code = reset_code + 1;
              mask = reset_code * 2 - 1;
            }
          } else {
            if (
              (prev_code == ~0 && code >= reset_code) ||
              code > cur_code
            ) {
              return true;
            }

            unsigned char tail = 0;
            if (code == cur_code) {
              *bytes++ = tail = add_lwz_string(prev_code, bytes, max_bytes-1, reset_code);
              if (debug_gif) printf("out %02x\n", tail);
            } else {
              tail = add_lwz_string(code, bytes, max_bytes, reset_code);
            }

            if (bytes > max_bytes) return true;

            lzw_head[cur_code] = prev_code;
            lzw_tail[cur_code] = tail;
            prev_code = code;

            cur_code++;
            if (cur_code > mask) {
              if (debug_gif) printf("resize\n");
              if (mask != 0xfff) {
                lzw_size++;
                mask = mask * 2 + 1;
              } else {
                cur_code = 0x1000;
              }
            }
          }
        } 
      } while( --len );
    }
    src++;
    srcref = src;
    return false;
  }

public:
  // get an opengl texture of gl_kind from a gif file in memory
  GLuint get_texture(unsigned gl_kind, const unsigned char *src, const unsigned char *src_max) {
    unsigned width = src[6] + src[7]*256;
    unsigned height = src[8] + src[9]*256;
    unsigned flags = src[10];
    unsigned gct_size = flags & 0x80 ? 1 << ((flags & 7)+1) : 0;
    dynarray<unsigned char> texture(width*height*4);
    //unsigned background = src[11];
    //unsigned aspect = src[12];
    unsigned transparency_index = 0x100; // disable transparency
    memset(texture.data(), 0xff, width*height*4);
    src += 13;
    const unsigned char *gct = src;
    src += gct_size * 3;
    while (src < src_max) {
      unsigned code = *src++;
      if (code == 0x3b) {
        // end
        break;
      } else if (code == 0x21) {
        if (*src == 0xf9) {
          // graphics control extension
          //unsigned block_size = src[1];
          unsigned flags = src[2];
          //unsigned delay = src[3] + src[4] * 256;
          transparency_index = flags & 1 ? src[5] : 0x100;

          src++;
          while (*src) {
            if (debug_gif) printf("    len=%02x\n", *src);
            src += *src + 1;
          }
          src++;
        } else {
          // unknown extension
          src++;
          while (*src) {
            if (debug_gif) printf("    len=%02x\n", *src);
            src += *src + 1;
          }
          src++;
        }
      } else if (code == 0x2c) {
        // image descriptor
        unsigned left = src[0] + src[1]*256;
        unsigned top = src[2] + src[3]*256;
        unsigned lwidth = src[4] + src[5]*256;
        unsigned lheight = src[6] + src[7]*256;
        unsigned flags = src[8];
        unsigned lct_size = ( flags & 0x80 ) ? 1 << ((flags & 7)+1) : 0;
        src += 9;
        const unsigned char *color_table = ( flags & 0x80 ) ? src : gct;
        src += lct_size * 3;
        unsigned min_lzw_size = *src++;

        dynarray<unsigned char> bytes(lwidth*lheight);
        bool error = 
          left + lwidth > width ||
          top + lheight > height ||
          gif_decode_bytes(&bytes[0], &bytes[lwidth*lheight], min_lzw_size, src)
        ;
        if (error) {
          printf("warning: gif_decode_bytes - broken gif file\n");
          goto fail;
        } else {
          unsigned char *src = &bytes[0];
          for (unsigned j = 0; j != lheight; ++j) {
            unsigned char *dest = &texture[((height - 1 - j - top) * width + left) * 4];
            for (unsigned i = 0; i != lwidth; ++i) {
              unsigned idx = *src++;
              dest[0] = color_table[idx*3+0];
              dest[1] = color_table[idx*3+1];
              dest[2] = color_table[idx*3+2];
              dest[3] = idx == transparency_index ? 0x00 : 0xff;
              //printf("%02x %02x %02x\n", dest[0], dest[1], dest[2]);
              dest += 4;
            }
          }
        }
      } else {
        printf("warning: unknown gif file section type\n");
      }
    }
  fail:;
    return app_utils::make_texture(gl_kind, texture, width, height);
  }
};

