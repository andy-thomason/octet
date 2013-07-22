////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// generalised resource: materials, scenes, cameras, meshes etc.
//

namespace octet {
  enum atom_t {
    atom_,

    // animation-relate atoms
    atom_transform,
    atom_translate,
    atom_rotateX,
    atom_rotateY,
    atom_rotateZ,
    atom_scale
  };
  
  inline static const char *predefined_atom(int i) {
    static const char *names[] = {
      "",

      // animation-relate atoms
      "transform",
      "translate",
      "rotateX",
      "rotateY",
      "rotateZ",
      "scale",
      NULL
    };
    return names[i];
  }

  class skin;
  class skeleton;
  class mesh;
  class material;
  class material;
  class animation;
  class camera_instance;
  class light_instance;
  class mesh_instance;
  class animation_instance;
  class scene;
  class scene_node;

  #define RESOURCE_META(classname) \
    classname *get_##classname() { return this; } \
    static const char *get_type() { return #classname; }

  class resource {
    int ref_count;

  public:
    resource() {
      ref_count = 0;
    }

    void add_ref() {
      ref_count++;
    }

    void release() {
      if (--ref_count == 0) {
        delete this;
      }
    }

    void *operator new (size_t size) { return allocator::malloc(size); }
    void operator delete (void *ptr, size_t size) { return allocator::free(ptr, size); }

    virtual skin *get_skin() { return 0; }
    virtual skeleton *get_skeleton() { return 0; }
    virtual mesh *get_mesh() { return 0; }
    virtual material *get_material() { return 0; }
    virtual animation *get_animation() { return 0; }
    virtual camera_instance *get_camera_instance() { return 0; }
    virtual light_instance *get_light_instance() { return 0; }
    virtual mesh_instance *get_mesh_instance() { return 0; }
    virtual animation_instance *get_animation_instance() { return 0; }
    virtual scene *get_scene() { return 0; }
    virtual scene_node *get_scene_node() { return 0; }
  };
}

