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
    atom_scale,

    // classes
    atom_skin,
    atom_skeleton,
    atom_mesh,
    atom_material,
    atom_animation,
    atom_camera_instance,
    atom_light_instance,
    atom_mesh_instance,
    atom_animation_instance,
    atom_scene,
    atom_scene_node,
    atom_animation_target,

    // lights
    atom_ambient,
    atom_directional,
    atom_spot,
    atom_point,
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

      "skin",
      "skeleton",
      "mesh",
      "material",
      "animation",
      "camera_instance",
      "light_instance",
      "mesh_instance",
      "animation_instance",
      "scene",
      "scene_node",
      "animation_target",

      "ambient",
      "directional",
      "spot",
      "point",
      NULL
    };
    return names[i];
  }

  class skin;
  class skeleton;
  class mesh;
  class material;
  class animation;
  class camera_instance;
  class light_instance;
  class mesh_instance;
  class animation_instance;
  class scene;
  class scene_node;
  class animation_target;

  // this macro implements standard functions for each class
  #define RESOURCE_META(classname) \
    classname *get_##classname() { return this; } \
    atom_t get_type() { return atom_##classname; } \
    static atom_t get_type_static() { return atom_##classname; }

  class resource {
    int ref_count;

  public:
    resource() {
      ref_count = 0;
    }

    virtual atom_t get_type() {
      return atom_;
    }

    virtual ~resource() {
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

    // casting and aggregation
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
    virtual animation_target *get_animation_target() { return 0; }
  };
}

