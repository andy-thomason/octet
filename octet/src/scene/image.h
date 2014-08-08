////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Material 
//
//
// Materials are represented as GL textures with solid colours as single pixel textures.
// This simplifies shader design.
//

namespace octet { namespace scene {
  /// Image from a file. Stored as an array of bytes for later conversion to GL resource.
  class image : public resource {
    // primary attributes (to save)

    // source of image for reloads
    string url;

    // image data
    dynarray<uint8_t> bytes;

    // dimensions
    uint32_t frames;
    uint16_t width;
    uint16_t height;
    uint16_t depth;
    uint16_t format;
    uint8_t mip_levels;
    uint8_t cube_faces;

    // derived attributes (not for saving)
    // todo: use gl_resource
    GLuint gl_texture;

    GLuint gl_target;

    void init(const char *name) {
      bool is_cubemap = strstr(name, "%s") != 0;
      this->url = name;
      width = height = 0;
      depth = 1;
      gl_texture = 0;
      gl_target = is_cubemap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
      mip_levels = 1;
      cube_faces = is_cubemap ? 6 : 1;
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

    /// Make mipmaps for this image.
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

    /// DXT encode the image, making it smaller and grainier.
    /// Todo: do standard error-diffusion and other improvements.
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
          log("w=%d y=%d src=%08x\n", w, y, src - &bytes[0]);
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
            //if (y == 27) log("%s %s %s\n", mean.toString(), axis.toString(), covariance.toString(tmp, sizeof(tmp)));

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
            //if (y == 27) log("%s -> %s\n", cmin.toString(), cmax.toString(tmp, sizeof(tmp)));
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

    void add_texture() {
      glBindTexture(gl_target, gl_texture);

      if (mip_levels == 1 || gl_target != GL_TEXTURE_2D) {
        if (gl_target == GL_TEXTURE_2D) {
          glTexImage2D(gl_target, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, (void*)&bytes[0]);
          // this may not work on very old systems, comment it out.
          glGenerateMipmap(gl_target);
        } else if (gl_target == GL_TEXTURE_3D) {
          glTexImage3D(gl_target, 0, format, width, height, 1, 0, format, GL_UNSIGNED_BYTE, (void*)&bytes[0]);
          printf("err=%08x\n", glGetError());
        } else if (gl_target == GL_TEXTURE_CUBE_MAP) {
          unsigned num_comps = format == RGBA ? 4 : 3;
          for (int i = 0; i != 6; ++i) {
            size_t offset = width * height * num_comps * i;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, (void*)&bytes[offset]);
            //static const unsigned cols[6] = { 0xff0000ff, 0xffff00ff, 0xffffffff, 0xff00ffff, 0x0000ffff, 0x00ffffff };
            //glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, 1, 1, 0, format, GL_UNSIGNED_BYTE, (void*)&cols[i]);
          }
          glGenerateMipmap(gl_target);
        }
      } else if (gl_target == GL_TEXTURE_2D) {
        unsigned num_comps = format == RGBA ? 4 : 3;
        unsigned w = width;
        unsigned h = height;
        uint8_t *src = &bytes[0];
        unsigned level = 0;
        while (w != 0 && h != 0) {
          glTexImage2D(gl_target, level++, format, w, h, 0, format, GL_UNSIGNED_BYTE, (void*)src);
          src += w * h * num_comps;
          w >>= 1;
          h >>= 1;
        }
      }
    }

  public:
    RESOURCE_META(image)

    /// default constructor makes a blank image.
    image() {
      init("");
    }

    /// give url of file to load.
    image(const char *name) {
      init(name);
    }

    /// generate an image from an opengl texture
    image(GLuint _target, GLuint _texture, unsigned _width, unsigned _height, unsigned _depth=1) {
      gl_target = _target;
      gl_texture = _texture;
      width = _width;
      height = _height;
      depth = _depth; // for 3D textures
    }

    /// release resources.
    ~image() {
    }

    /// width in pixels
    unsigned get_width() const {
      return width;
    }

    /// height in pixels
    unsigned get_height() const {
      return height;
    }

    /// depth in voxels for 3d textures
    unsigned get_depth() const {
      return depth;
    }

    /// GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP
    unsigned get_target() const {
      return gl_target;
    }

    /// animated textures have multiple frames. eg. MPEG file. return ~0 for infinite.
    unsigned get_frames() const {
      return frames;
    }

    /// access attributes by name
    void visit(visitor &v) {
      v.visit(url, atom_url);
      v.visit(bytes, atom_bytes);
      v.visit(format, atom_format);
      v.visit(width, atom_width);
      v.visit(height, atom_height);
      v.visit(mip_levels, atom_mip_levels);
      v.visit(cube_faces, atom_cube_faces);
    }

    /// load the image from a url
    void load() {
      string x;
      if (cube_faces == 6) {
        bytes.resize(0);
        x.format(url, "left");
        load_part(x.c_str());
        x.format(url, "right");
        load_part(x.c_str());
        x.format(url, "top");
        load_part(x.c_str());
        x.format(url, "bottom");
        load_part(x.c_str());
        x.format(url, "front");
        load_part(x.c_str());
        x.format(url, "back");
        load_part(x.c_str());
      } else {
        bytes.resize(0);
        load_part(url.c_str());
      }
    }

    void load_part(const char *_url) {
      dynarray<uint8_t> buffer;
      app_utils::get_url(buffer, _url);
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
      } else if (buffer.size() >= 348 && (!memcmp(&buffer[344], "ni1", 4) || !memcmp(&buffer[344], "n+1", 4))) {
        nifti_decoder dec;
        gl_target = GL_TEXTURE_3D;
        dec.get_image(bytes, format, width, height, depth, frames, src, src_max);
      } else {
        printf("warning: unknown texture format\n");
        return;
      }

      make_mipmaps();
      //dxt_encode();
    }

    /// get the OpenGL texture handle for this image.
    GLuint get_gl_texture() {
      if (!gl_texture) {
        if (bytes.size() == 0 || width == 0 || height == 0) {
          load();
        }

        // make a new texture handle
        glGenTextures(1, &gl_texture);
        glActiveTexture(GL_TEXTURE0);

        // todo: handle compressed textures
        if (format == GL_RGB || format == GL_RGBA) {
          add_texture();
        } else if (format == COMPRESSED_RGB_S3TC_DXT1_EXT || format == COMPRESSED_RGBA_S3TC_DXT1_EXT || format == COMPRESSED_RGBA_S3TC_DXT3_EXT || format == COMPRESSED_RGBA_S3TC_DXT5_EXT) {
          glBindTexture(gl_target, gl_texture);
          unsigned w = width;
          unsigned h = height;
          uint8_t *src = &bytes[0];
          unsigned level = 0;
          unsigned shift = ( format == COMPRESSED_RGB_S3TC_DXT1_EXT || format == COMPRESSED_RGBA_S3TC_DXT1_EXT ) ? 1 : 0;
          while (w != 0 && h != 0) {
            unsigned wmin = w < 4 ? 4 : w;
            unsigned hmin = h < 4 ? 4 : h;
            unsigned size = ( wmin * hmin ) >> shift;
            glCompressedTexImage2D(gl_target, level++, format, w, h, 0, size, (void*)src);
            //printf("%d\n", glGetError());
            src += size;
            w >>= 1;
            h >>= 1;
          }
          //printf("%d %d\n", src - image_, size);
        }

        glTexParameteri(gl_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(gl_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      }
      return gl_texture;
    }

    /// todo: merge gl_resource with textures.
    GLuint get_gl_target() const {
      return gl_target;
    }

    /// fast reload once get_gl_target has been called.
    void reload(GLuint format, GLuint type, void *pixels) {
      if (gl_target == 0) return;

      glBindTexture(gl_target, gl_texture);
      glTexSubImage2D(gl_target, 0, 0, 0, width, height, format, type, pixels);
    }
  };
}}

