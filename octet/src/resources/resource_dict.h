////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// a container for named resources
//

namespace octet { namespace resources {
  /// Resource dictionary / game world class.
  ///
  /// Used to hold resources in a game and access them by name.
  ///
  class resource_dict : public resource {
    dictionary<ref<resource> > dict;
    ref<scene::visual_scene> active_scene;

    #ifdef WIN32
      // vc2010/../
      static const char *prefix() { return "../"; }
    #else
      // xcode/../
      static const char *prefix() { return "../"; }
    #endif

    typedef dictionary<GLuint> textures_t;
    typedef dictionary<int> sounds_t;

    static textures_t &textures() { static textures_t instance;  return instance; }
    static sounds_t &sounds() { static sounds_t instance;  return instance; }

    static GLuint get_texture_handle_internal(unsigned gl_kind, const char *name);

    static unsigned u4(unsigned char *src) {
      return src[0] + src[1] * 256 + src[2] * 65536 + src[3] * 0x1000000;
    }

    static ALuint get_sound_handle_internal(unsigned al_kind, const char *name) {
      if (name[0] == '#') {
        // todo: implement notes etc.
        return 0;
      } else {
        dynarray<unsigned char> buffer;
        app_utils::get_url(buffer, name);
        if (buffer.size() >= 6 && !memcmp(&buffer[0], "RIFF", 4)) {
          unsigned offset = 0;
          unsigned samples = 44100;
          unsigned char *src = &buffer[0];
          for (unsigned i = 12; i+8 <= buffer.size(); i += 8 + u4(src+i+4)) {
            if (src[i] == 'f' && src[i+1] == 'm' && src[i+2] == 't' && src[i+3] == ' ') {
              samples = u4(src+i+12);
            } else if (src[i] == 'd' && src[i+1] == 'a' && src[i+2] == 't' && src[i+3] == 'a') {
              offset = i + 8;
              break;
            }
          }
          return app_utils::make_sound_buffer(al_kind, samples, buffer, offset, buffer.size() - offset);
        } else {
          printf("warning: unknown audio format\n");
        }
      }
      return 0;
    }
  public:
    /// Construct a new resource dictionary
    resource_dict() {
    }

    /// Visitor for loading and saving
    virtual void visit(visitor &v) {
      v.visit(active_scene, atom_active_scene);
      v.visit(dict, atom_dict);
    }

    /// Reset the dictionary, clearing all data
    void reset() {
      dict.reset();
    }

    /// does the dictionary have this resource?
    bool has_resource(const char *name) {
      return dict.contains(name);
    }

    /// Get a generic resource by name
    /// Note: you can get a specific type using get_<typename>
    /// For example, scene_node *node = dict.get_scene_node("name");
    resource *get_resource(const char *name) {
      if (name == 0 || name[0] == 0) {
        return NULL;
      }
      if (name[0] == '#') name++;

      if (!dict.contains(name)) {
        return NULL;
      } else {
        return dict[name];
      }
    }

    /// As this dict represents a game world, what is the active scene?
    scene::visual_scene *get_active_scene() const {
      return active_scene;
    }

    /// Set the active scene for this game world
    void set_active_scene(scene::visual_scene *value) {
      active_scene = value;
    }

    void set_resource(const char *name, resource *value) {
      if (name && name[0]) {
        dict[name] = value;
      }
    }

    /// factory for textures: Deprecated will use Image object in future
    static GLuint get_texture_handle(unsigned gl_kind, const char *name) {
      GLuint &result = textures()[name];
      if (result == 0) {
        result = get_texture_handle_internal(gl_kind, name);
      }
      return result;
    }

    /// factory for sounds: Deprecated will use Sound object in future
    static int get_sound_handle(unsigned al_kind, const char *name) {
      int &result = sounds()[name];
      if (result == 0) {
        result = get_sound_handle_internal(al_kind, name);
      }
      return result;
    }

    #define OCTET_CLASS(N, X) N::X *get_##X(const char *id) { resource *res = get_resource(id); return res ? res->get_##X() : 0; }
    //#pragma message("resource_dict.h")
    #include "classes.h"
    #undef OCTET_CLASS

    /// Find all resources of a certain type
    void find_all(dynarray<resource*> &result, atom_t type) {
      unsigned num_indices = dict.get_num_indices();
      for (unsigned i = 0; i != num_indices; ++i) {
        const char *key = dict.get_key(i);
        if (key) {
          resource *res = dict.get_value(i);
          if (res->get_type() == type) {
            result.push_back(res);
          }
        }
      }
    }

    // dump the assets in the dictionary as code.
    void dump_assets(FILE *log) {
      unsigned num_indices = dict.get_num_indices();
      for (unsigned i = 0; i != num_indices; ++i) {
        const char *key = dict.get_key(i);
        if (key) {
          resource *res = dict.get_value(i);
          const char *type = app_utils::get_atom_name(res->get_type());
          string c_name = key;
          for (char *p = c_name.data(); *p; ++p) {
            if (*p == '-' || *p == '+') *p = '_';
          }
          fprintf(log, "    %s *%s = dict.get_%s(\"%s\");\n", type, c_name.c_str(), type, key);
        }
      }
      fflush(log);
    }
  };

} }
