////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
//

namespace octet {
  class app_utils {
  public:
    static const char *prefix(const char *new_prefix=NULL) {
      static const char *value = "../";
      if (new_prefix) {
        value = new_prefix;
      }
      return value;
    }
  
    static void setrgb(dynarray<unsigned char> &buffer, int size, int x, int y, unsigned rgb, unsigned a = 0xff) {
      buffer[(y*size+x)*4+0] = rgb >> 16;
      buffer[(y*size+x)*4+1] = rgb >> 8;
      buffer[(y*size+x)*4+2] = rgb >> 0;
      buffer[(y*size+x)*4+3] = a;
    }
  
    static const char *get_path(const char *url) {
      static dynarray<char> path;
      path.resize((unsigned)(strlen(prefix()) + strlen(url) + 1));
      char *dest = &path[0];
      strcpy(dest, prefix());
      strcat(dest, url);
      return dest;
    }

    static void get_url(dynarray<unsigned char> &buffer, const char *url) {
      if (!strncmp(url, "http://", 7)) {
        // http
      } else {
        FILE *file = fopen(get_path(url), "rb");
        if (!file) {
          printf("file %s not found\n", get_path(url));
        } else {
          fseek(file, 0, SEEK_END);
          buffer.resize((unsigned)ftell(file));
          fseek(file, 0, SEEK_SET);
          fread(buffer.data(), 1, buffer.size(), file);
          fclose(file);
        }
      }
    }

    static GLuint get_stock_texture(unsigned gl_kind, const char *name) {
      //stock_texture_generator stock;
      if (!strcmp(name, "bricks")) {
        // bricks texture: make a brick pattern by poking numbers
        // into an array of RGB values
        enum { size = 64 };
        dynarray<unsigned char> buffer(size*size*4);
        for (int y = 0; y != size; ++y) {
          for (int x = 0; x != size; ++x) {
            setrgb(buffer, size, x, y, 0x604020);
          }
        }
        for (int x = 0; x != size; ++x) {
          setrgb(buffer, size, x, 0, 0x808080);
          setrgb(buffer, size, x, size/2, 0x808080);
        }
        for (int y = 0; y != size/2; ++y) {
          setrgb(buffer, size, 0, y, 0x808080);
          setrgb(buffer, size, size/2, y+size/2, 0x808080);
        }

        return make_texture(gl_kind, buffer, size, size);
      } else if (!strcmp(name, "bump")) {
        // bump texture: make a random bump map with 0x808000 (0.5,0.5,0) the default normal x and y offsets
        class random rand(0x9bac7615);
        enum { size = 128 };

        dynarray<unsigned char> buffer(size*size*4);
        for (int y = 0; y != size; ++y) {
          for (int x = 0; x != size; ++x) {
            int r = rand.get(64,  192);
            int g = rand.get(64,  192);
            setrgb(buffer, size, x, y, r * 0x10000 + g * 0x100);
          }
        }
        return make_texture(gl_kind, buffer, size, size);
      } else {
        printf("warning: stock texture %s not found\n", name);
        return 0;
      }
    }

    static GLuint get_solid_texture(unsigned gl_kind, const char *name) {
      dynarray<unsigned char>buffer(1*1*4);
      unsigned val = 0;
      unsigned ndigits = 0;
      for (int i = 0; name[i]; ++i) {
        char c = name[i];
        val = val * 16 + ( ( c <= '9' ? c - '0' : c - 'A' + 10 ) & 0x0f );
        ndigits++;
      }

      buffer[3] = 0xff;
      if (ndigits == 8) {
        buffer[3] = val >> 0;
        val >>= 8;
      }
      buffer[0] = val >> 16;
      buffer[1] = val >> 8;
      buffer[2] = val >> 0;
      return make_texture(gl_kind, buffer, 1, 1);
    }

    // utility function for making textures from arrays of bytes
    // gl_kind is GL_RGB or GL_RGBA
    static GLuint make_texture(unsigned gl_kind, dynarray<unsigned char> &buffer, unsigned width, unsigned height) {
      assert(buffer.size() == width * height * 4);
      // make a new texture handle
      GLuint handle = 0;
      glGenTextures(1, &handle);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, handle);

      glTexImage2D(GL_TEXTURE_2D, 0, gl_kind, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)buffer.data());

      glGenerateMipmap(GL_TEXTURE_2D);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      return handle;
    }

    static ALuint make_sound_buffer(unsigned kind, unsigned rate, dynarray<unsigned char> &buffer, unsigned offset, unsigned size) {
      ALuint id = 0;
      alGenBuffers(1, &id);
      alBufferData(id, kind, &buffer[offset], size, rate);
      return id;
    }

    // write some text to log.txt
    static FILE * log(const char *fmt, ...) {
      static FILE *file;
      va_list list;
      va_start(list, fmt);
      if (!file) file = fopen("log.txt", "w");
      vfprintf(file, fmt, list);
      va_end(list);
      fflush(file);
      return file;
    }
  };
}
