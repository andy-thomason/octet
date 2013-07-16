////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// a container for named resources
//

class resources {
  dictionary<ref<resource>> dict;

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

  static dictionary<int> *get_atom_dict() {
    static dictionary<int> *dict;
    if (!dict) dict = new dictionary<int>();
    return dict;
  }

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
        return app_utils::make_sound_buffer(al_kind, 44100, buffer, offset, buffer.size() - offset);
      } else {
        printf("warning: unknown audio format\n");
      }
    }
    return 0;
  }
public:
  resources() {
  }

  bool has_resource(const char *name) {
    return dict.contains(name);
  }

  // get a unique int for a string.
  resource *get_resource(const char *name) {
    if (name == 0 || name[0] == 0) {
      return NULL;
    }
    if (name[0] == '#') name++;

    static atom_t num_atoms = 1;
    if (!dict.contains(name)) {
      return NULL;
    } else {
      return dict[name];
    }
  }

  void set_resource(const char *name, resource *value) {
    if (name && name[0]) {
      dict[name] = value;
    }
  }

  // factory for textures
  static GLuint get_texture_handle(unsigned gl_kind, const char *name) {
    GLuint &result = textures()[name];
    if (result == 0) {
      result = get_texture_handle_internal(gl_kind, name);
    }
    return result;
  }

  // factory for sounds
  static int get_sound_handle(unsigned al_kind, const char *name) {
    int &result = sounds()[name];
    if (result == 0) {
      result = get_sound_handle_internal(al_kind, name);
    }
    return result;
  }

  // get a unique int for a string.
  static atom_t get_atom(const char *name) {
    // the null name is 0
    if (name == 0 || name[0] == 0) {
      return 0;
    }

    dictionary<int> *dict = get_atom_dict();

    static atom_t num_atoms = 1;
    if (dict->contains(name)) {
      app_utils::log("old atom %s %d\n", name, (*dict)[name]);
      return (*dict)[name];
    } else {
      app_utils::log("new atom %s %d\n", name, num_atoms);
      return (*dict)[name] = num_atoms++;
    }
  }

  skin *get_skin(const char *id) { resource *res = get_resource(id); return res ? res->get_skin() : 0; }
  skeleton *get_skeleton(const char *id) { resource *res = get_resource(id); return res ? res->get_skeleton() : 0; }
  mesh_state *get_mesh_state(const char *id) { resource *res = get_resource(id); return res ? res->get_mesh_state() : 0; }
  material *get_material(const char *id) { resource *res = get_resource(id); return res ? res->get_material() : 0; }
  bump_material *get_bump_material(const char *id) { resource *res = get_resource(id); return res ? res->get_bump_material() : 0; }
  animation *get_animation(const char *id) { resource *res = get_resource(id); return res ? res->get_animation() : 0; }
  camera_instance *get_camera_instance(const char *id) { resource *res = get_resource(id); return res ? res->get_camera_instance() : 0; }
  light_instance *get_light_instance(const char *id) { resource *res = get_resource(id); return res ? res->get_light_instance() : 0; }
  mesh_instance *get_mesh_instance(const char *id) { resource *res = get_resource(id); return res ? res->get_mesh_instance() : 0; }
  animation_instance *get_animation_instance(const char *id) { resource *res = get_resource(id); return res ? res->get_animation_instance() : 0; }
  scene *get_scene(const char *id) { resource *res = get_resource(id); return res ? res->get_scene() : 0; }
  scene_node *get_scene_node(const char *id) { resource *res = get_resource(id); return res ? res->get_scene_node() : 0; }
};

