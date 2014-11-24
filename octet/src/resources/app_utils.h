////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
//

namespace octet {
  // this enum is used to avoid using strings in code and files
  enum atom_t {
    atom_, // null atom
    
    #define OCTET_ATOM(X) atom_##X,
    #include "atoms.h"
    #undef OCTET_ATOM

    // put classes at a fixed offset to prevent older files becomming obsolete
    atom_class_base = 0x10000,
    #define OCTET_CLASS(C, X) atom_##X,
    //#pragma message("app_utils.h")
    #include "classes.h"
    #undef OCTET_CLASS
  };
}

namespace octet { namespace resources {
  /// A set of utilities   
  class app_utils {
  public:
    /// Set and get the file prefix. This is used to find resource files in the game.
    static const char *prefix(const char *new_prefix=NULL) {
      static const char *value = NULL;
      if (new_prefix) {
        value = new_prefix;
      } else if (value == NULL) {
        // if the prefix is not set, try to find the root directory by opening README.txt
        const char *rme = "../../../../README.txt";
        const char *pfx = "../../../../";
        for (int i = 0; i != 5; ++i) {
          FILE *test = fopen(rme + i * 3, "rb");
          if (test) {
            fclose(test);
            value = pfx + i * 3;
            break;
          }
        }
      }
      return value;
    }

    /// open a zip file for a given URL
    static zip_file *get_zip_file(const char *url) {
      static dictionary<ref<zip_file> > zip_files;
      int index = zip_files.get_index(url);
      if (index == -1) {
        return zip_files[url] = new zip_file(get_path(url));
      } else {
        return zip_files.get_value(index);
      }
    }
  
    /// utility function to set rgb values in a buffer.
    static void setrgb(dynarray<unsigned char> &buffer, int size, int x, int y, unsigned rgb, unsigned a = 0xff) {
      buffer[(y*size+x)*4+0] = rgb >> 16;
      buffer[(y*size+x)*4+1] = rgb >> 8;
      buffer[(y*size+x)*4+2] = rgb >> 0;
      buffer[(y*size+x)*4+3] = a;
    }
  
    /// Convert a url into a file path.
    static const char *get_path(const char *url) {
      if (url == NULL) return "";

      string url_str;
      url_str.urldecode(url);
      static string path;

      if (url[0] == '/' || (url[0] >= 'A' && url[0] <= 'Z' && url[1] == ':')) {
        path = url_str;
      } else {
        // relative path
        path.format("%s%s", prefix(), url_str.c_str());
      }
      return path;
    }

    /// Get a file into a buffer, given a URL.
    static void get_url(dynarray<unsigned char> &buffer, const char *url) {
      if (!strncmp(url, "zip://", 6)) {
        const char *zip = strstr(url + 6, ".zip");
        if (zip) {
          int path_len = (int)(zip - (url + 6) + 4);
          string zip_url;
          zip_url.set(url + 6, path_len);
          const char *file = (url + 6) + path_len;
          file += file[0] == '/';
          zip_file *zip = get_zip_file(zip_url.c_str());
          zip->get_file(buffer, file);
        }
      } else if (!strncmp(url, "http://", 7)) {
        // http
      } else {
        const char *path = get_path(url);
        FILE *file = fopen(path, "rb");
        if (!file) {
          char tmp[1024];
          printf("file %s not found. cwd=%s\n", path, getcwd(tmp, sizeof(tmp)));
        } else {
          fseek(file, 0, SEEK_END);
          unsigned size = (unsigned)ftell(file);
          buffer.reserve(size+1); // 1 more byte for zero terminator on a text file
          buffer.resize(size);
          fseek(file, 0, SEEK_SET);
          fread(buffer.data(), 1, buffer.size(), file);
          fclose(file);
        }
      }
    }

    /// Generate a stock texture. To be deprecated.
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

        return make_texture(gl_kind, &buffer[0], buffer.size(), GL_RGBA, size, size);
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
        return make_texture(gl_kind, &buffer[0], buffer.size(), GL_RGBA, size, size);
      } else {
        printf("warning: stock texture %s not found\n", name);
        return 0;
      }
    }

    /// Generate a solid texture. to be deprecated.
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
      return make_texture(gl_kind, &buffer[0], buffer.size(), GL_RGBA, 1, 1);
    }

    /// Utility function for making textures from arrays of bytes.
    /// gl_kind is GL_RGB or GL_RGBA
    static GLuint make_texture(unsigned gl_kind, uint8_t *image, unsigned size, unsigned in_format, unsigned width, unsigned height) {
      //assert(buffer.size() == width * height * 4);
      // make a new texture handle
      GLuint handle = 0;
      glGenTextures(1, &handle);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, handle);

      glTexImage2D(GL_TEXTURE_2D, 0, gl_kind, width, height, 0, in_format, GL_UNSIGNED_BYTE, (void*)image);

      glGenerateMipmap(GL_TEXTURE_2D);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      return handle;
    }

    /// Make an OpenAL sound buffer
    static ALuint make_sound_buffer(unsigned kind, unsigned rate, dynarray<unsigned char> &buffer, unsigned offset, unsigned size) {
      ALuint id = 0;
      alGenBuffers(1, &id);
      alBufferData(id, kind, &buffer[offset], size, rate);
      return id;
    }

    /// Get the system atom dictionary. Atoms are unique names with an integer representation.
    static dictionary<atom_t> *get_atom_dict() {
      static dictionary<atom_t> *dict;
      if (!dict) dict = new dictionary<atom_t>();
      return dict;
    }

    /// Get a unique int for a string (atom). Atoms are unique names with an integer representation.
    /// These values are much cheaper to work with than strings.
    static atom_t get_atom(const char *name) {
      // the null name is 0
      if (name == 0 || name[0] == 0) {
        return atom_;
      }

      dictionary<atom_t> *dict = get_atom_dict();

      static int num_atoms = 0;
      if (num_atoms == 0) {
        for (++num_atoms; predefined_atom(num_atoms); num_atoms++) {
          (*dict)[predefined_atom(num_atoms)] = (atom_t)num_atoms;
        }
      }
      if (dict->contains(name)) {
        //log("old atom %s %d\n", name, (*dict)[name]);
        return (*dict)[name];
      } else {
        //log("new atom %s %d\n", name, num_atoms);
        return (*dict)[name] = (atom_t)num_atoms++;
      }
    }

    /// Get the text of a predefined atom (atom_*)
    static const char *predefined_atom(unsigned i) {
      static const char *atom_names[] = {
        "",

        #define OCTET_ATOM(X) #X,
        #include "atoms.h"
        #undef OCTET_ATOM
        NULL
      };
      static const char *class_names[] = {
        "",

        #define OCTET_CLASS(N, X) #X,
        //#pragma message("app_utils.h 2")
        #include "classes.h"
        #undef OCTET_CLASS
        NULL
      };
      if (i < sizeof(atom_names)/sizeof(atom_names[0])-1) {
        return atom_names[i];
      } else if (i-(unsigned)atom_class_base < sizeof(class_names)/sizeof(class_names[0])-1) {
        return class_names[i-(unsigned)atom_class_base];
      } else {
        return NULL;
      }
    }

    /// Get the name of an atom, either predefined or user defined.
    static const char *get_atom_name(atom_t atom) {
      const char *name = predefined_atom((unsigned)atom);
      if (name) return name;

      // slow!
      dictionary<atom_t> *dict = get_atom_dict();
      unsigned num_indices = dict->get_num_indices();
      for (unsigned i = 0; i != num_indices; ++i) {
        if (dict->get_value(i) == atom) {
          return dict->get_key(i);
        }
      }
      return "???";
    }
  };
} }
