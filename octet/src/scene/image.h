////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Material 
//
//
// Materials are represented as GL textures with solid colours as single pixel textures.
// This simplifies shader design.
//

namespace octet {
  class image : public resource {
    // primary attributes (to save)

    // source of image for reloads
    string url;

    // image data
    dynarray<uint8_t> bytes;

    // dimensions
    uint16_t width;
    uint16_t height;
    uint16_t format;
    uint8_t mip_levels;
    uint8_t cube_faces;

    // derived attributes (not for saving)
    GLuint gl_texture;

    void init(const char *name) {
      this->url = name;
      width = height = 0;
      gl_texture = 0;
      mip_levels = 1;
      cube_faces = 1;
      format = 0;
    }

    // these are here to avoid including glext.h which may be platform dependent.
    enum {
      // format options
      DEPTH_COMPONENT = 0x1902,
      ALPHA = 0x1906,
      RGB = 0x1907,
      RGBA = 0x1908,
      LUMINANCE = 0x1909,
      LUMINANCE_ALPHA = 0x190A,
      COMPRESSED_RGB_S3TC_DXT1_EXT = 0x83F0,
      COMPRESSED_RGBA_S3TC_DXT1_EXT = 0x83F1,
      COMPRESSED_RGBA_S3TC_DXT3_EXT = 0x83F2,
      COMPRESSED_RGBA_S3TC_DXT5_EXT = 0x83F3,
    };

    void make_mipmaps() {
      if (format != RGB && format != RGBA) return;

      bytes.resize(bytes.size() * 4 / 3);

      unsigned num_comps = format == RGB ? 3 : 4;
      uint8_t *src = &bytes[0];
      uint8_t *dest = &bytes[width * height * num_comps];
      unsigned w = width;
      unsigned h = height;
      unsigned stride = w * num_comps;
      mip_levels = 0;
      while (w > 1 && h > 1) {
        for (unsigned y = 0; y < h/2; ++y) {
          for (unsigned x = 0; x < w/2; ++x) {
            // this is a rather dreadful box filter, it is simple to make, but introduces
            // artifacts. We can do better than this.
            for (unsigned i = 0; i != num_comps; ++i) {
              *dest++ = ( src[0] + src[num_comps] + src[stride] + src[stride+num_comps] + 3 ) >> 2;
              src++;
            }
            src += num_comps;
          }
          src += stride;
        }
        w >>= 1;
        h >>= 1;
        stride >>= 1;
        mip_levels++;
      }
      assert(dest - &bytes[0] < (int)bytes.size());
      //printf("%d %d\n", dest - &bytes[0], bytes.size());
    }

    void dxt_encode() {
      if (format != RGB && format != RGBA) return;

      unsigned num_comps = format == RGB ? 3 : 4;
      dynarray<uint8_t> result;
      result.resize(bytes.size() / num_comps);
      uint8_t *src = &bytes[0];
      uint8_t *dest = &result[0];
      unsigned w = width;
      unsigned h = height;
      unsigned stride = w * num_comps;
      unsigned new_mip_levels = 0;
      while (w > 4 && h > 4 && new_mip_levels < mip_levels) {
        for (unsigned y = 0; y < h/4; ++y) {
          app_utils::log("w=%d y=%d src=%08x\n", w, y, src - &bytes[0]);
          for (unsigned x = 0; x < w/4; ++x) {
            // ye olde covariance method http://en.wikipedia.org/wiki/Linear_discriminant_analysis
            vec4 tot(0, 0, 0, 0);
            vec4 colours[16];
            for (unsigned j = 0; j != 4; ++j) {
              for (unsigned i = 0; i != 4; ++i) {
                //vec4 colour(src[0] * (1.0f/255), src[1] * (1.0f/255), src[2] * (1.0f/255), (num_comps == 4 ? src[3] * (1.0f/255) : 1.0f));
                vec4 colour(src[0] * (1.0f/255), src[1] * (1.0f/255), src[2] * (1.0f/255), 1.0f);
                colours[i+j*4] = colour;
                tot += colour;
                src += num_comps;
              }
              src += ( w - 4 ) * num_comps;
            }
            src -= ( w - 1 ) * 4 * num_comps;

            // calculate the covariance matrix
            vec4 mean = tot * 0.0625f;
            mat4t covariance(0);
            for (unsigned i = 0; i != 16; ++i) {
              vec4 colour = colours[i] -= mean;
              covariance += outer(colour, colour);
            }

            // power method to find largest eigenvector (axis)
            vec4 axis = covariance.trace();
            for (unsigned i = 0; i != 4; ++i) {
              axis = axis * covariance;
            }
            float len = axis.length();
            if (abs(len) >= 0.001f) axis = axis / len;
            if (y == 27) app_utils::log("%s %s %s\n", mean.toString(), axis.toString(), covariance.toString());

            // our colours all have to live on the axis.
            // in practice, we can ignore "odd man out" colours
            // for better compression. But for now, do all the colours.
            float pmin = dot(colours[0], axis);
            float pmax = pmin;
            float projs[16];
            projs[0] = pmin;
            for (unsigned i = 1; i != 16; ++i) {
              float proj = dot(colours[i], axis);
              projs[i] = proj;
              pmin = pmin < proj ? pmin : proj;
              pmax = pmax > proj ? pmax : proj;
            }
            vec4 cmin = mean + axis * pmin;
            vec4 cmax = mean + axis * pmax;
            if (y == 27) app_utils::log("%s -> %s\n", cmin.toString(), cmax.toString());
            cmin = min(max(cmin, vec4(0, 0, 0, 0)), vec4(1, 1, 1, 1));
            cmax = min(max(cmax, vec4(0, 0, 0, 0)), vec4(1, 1, 1, 1));

            unsigned c0 =
              ( (unsigned)(cmin.x() * 31.999f) << 11 ) |
              ( (unsigned)(cmin.y() * 63.999f) << 5 ) |
              ( (unsigned)(cmin.z() * 31.999f) << 0 )
            ;
            unsigned c1 =
              ( (unsigned)(cmax.x() * 31.999f) << 11 ) |
              ( (unsigned)(cmax.y() * 63.999f) << 5 ) |
              ( (unsigned)(cmax.z() * 31.999f) << 0 )
            ;

            if (c0 < c1) {
              unsigned t = c0; c0 = c1; c1 = t;
              float p = pmin; pmin = pmax; pmax = p;
            }

            float pscale = abs(pmax - pmin) > 0.001f ? 3.999f / (pmax - pmin) : 0;
            uint8_t pal[16];
            for (unsigned i = 0; i != 16; ++i) {
              pal[i] = (unsigned)( ( projs[i] - pmin ) * pscale );
            }

            dest[0] = ( c0 >> 0 ) & 0xff;
            dest[1] = ( c0 >> 8 ) & 0xff;
            dest[2] = ( c1 >> 0 ) & 0xff;
            dest[3] = ( c1 >> 8 ) & 0xff;
            for (int i = 0; i != 4; ++i) {
              dest[i+4] = pal[i*4+0] + pal[i*4+1] * 4 + pal[i*4+2] * 16 + pal[i*4+3] * 64;
            }
            dest += 8;
          }
          src += stride * 3;
        }
        w >>= 1;
        h >>= 1;
        stride >>= 1;
        new_mip_levels++;
      }
      assert(dest - &result[0] < (int)result.size());
      bytes.resize((int)(dest - &result[0]));
      memcpy(&bytes[0], &result[0], dest - &result[0]);
      format = COMPRESSED_RGB_S3TC_DXT1_EXT;
      mip_levels = new_mip_levels;
    }

  public:
    RESOURCE_META(image)

    // default constructor makes a blank image.
    image() {
      init("");
    }

    image(const char *name) {
      init(name);
    }

    ~image() {
    }

    // access attributes by name
    void visit(visitor &v) {
      v.visit(url, atom_url);
      v.visit(bytes, atom_bytes);
      v.visit(format, atom_format);
      v.visit(width, atom_width);
      v.visit(height, atom_height);
      v.visit(mip_levels, atom_mip_levels);
      v.visit(cube_faces, atom_cube_faces);
    }

    // load the image from a file
    void load() {
      dynarray<uint8_t> buffer;
      app_utils::get_url(buffer, url);
      const unsigned char *src = &buffer[0];
      const unsigned char *src_max = src + buffer.size();
      if (buffer.size() >= 6 && !memcmp(&buffer[0], "GIF89a", 6)) {
        gif_decoder dec;
        dec.get_image(bytes, format, width, height, src, src_max);
      } else if (buffer.size() >= 6 && buffer[0] == 0xff && buffer[1] == 0xd8) {
        jpeg_decoder dec;
        dec.get_image(bytes, format, width, height, src, src_max);
      } else if (buffer.size() >= 6 && buffer[0] == 0 && buffer[1] == 0 && buffer[2] == 2) {
        tga_decoder dec;
        dec.get_image(bytes, format, width, height, src, src_max);
      } else if (buffer.size() >= 4 && buffer[0] == 'D' && buffer[1] == 'D' && buffer[2] == 'S' && buffer[3] == ' ') {
        dds_decoder dec;
        dec.get_image(bytes, format, width, height, src, src_max);
      } else {
        printf("warning: unknown texture format\n");
        return;
      }

      make_mipmaps();
      //dxt_encode();
    }

    GLuint get_gl_texture() {
      if (!gl_texture) {
        if (bytes.size() == 0 || width == 0 || height == 0) {
          load();
        }

        // make a new texture handle
        glGenTextures(1, &gl_texture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gl_texture);

        // todo: handle compressed textures
        if (format == GL_RGB || format == GL_RGBA) {
          if (mip_levels == 1) {
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, (void*)&bytes[0]);
            // this may not work on very old systems, comment it out.
            glGenerateMipmap(GL_TEXTURE_2D);
          } else {
            unsigned num_comps = format == RGBA ? 4 : 3;
            unsigned w = width;
            unsigned h = height;
            uint8_t *src = &bytes[0];
            unsigned level = 0;
            while (w != 0 && h != 0) {
              glTexImage2D(GL_TEXTURE_2D, level++, format, w, h, 0, format, GL_UNSIGNED_BYTE, (void*)src);
              src += w * h * num_comps;
              w >>= 1;
              h >>= 1;
            }
          }
        } else if (format == COMPRESSED_RGB_S3TC_DXT1_EXT || format == COMPRESSED_RGBA_S3TC_DXT1_EXT || format == COMPRESSED_RGBA_S3TC_DXT3_EXT || format == COMPRESSED_RGBA_S3TC_DXT5_EXT) {
          unsigned w = width;
          unsigned h = height;
          uint8_t *src = &bytes[0];
          unsigned level = 0;
          unsigned shift = ( format == COMPRESSED_RGB_S3TC_DXT1_EXT || format == COMPRESSED_RGBA_S3TC_DXT1_EXT ) ? 1 : 0;
          while (w != 0 && h != 0) {
            unsigned wmin = w < 4 ? 4 : w;
            unsigned hmin = h < 4 ? 4 : h;
            unsigned size = ( wmin * hmin ) >> shift;
            glCompressedTexImage2D(GL_TEXTURE_2D, level++, format, w, h, 0, size, (void*)src);
            //printf("%d\n", glGetError());
            src += size;
            w >>= 1;
            h >>= 1;
          }
          //printf("%d %d\n", src - image_, size);
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      }
      return gl_texture;
    }
  };
}

