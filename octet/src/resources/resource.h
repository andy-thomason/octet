////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// generalised resource: materials, scenes, cameras, meshes etc.
//

typedef int atom_t;

class skin;
class skeleton;
class mesh_state;
class material;
class bump_material;
class animation;
class camera_instance;
class light_instance;
class mesh_instance;
class animation_instance;
class scene;
class scene_node;

#define RESOURCE_META(classname) classname *get_##classname() { return this; }

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

  virtual skin *get_skin() { return 0; }
  virtual skeleton *get_skeleton() { return 0; }
  virtual mesh_state *get_mesh_state() { return 0; }
  virtual material *get_material() { return 0; }
  virtual bump_material *get_bump_material() { return 0; }
  virtual animation *get_animation() { return 0; }
  virtual camera_instance *get_camera_instance() { return 0; }
  virtual light_instance *get_light_instance() { return 0; }
  virtual mesh_instance *get_mesh_instance() { return 0; }
  virtual animation_instance *get_animation_instance() { return 0; }
  virtual scene *get_scene() { return 0; }
  virtual scene_node *get_node() { return 0; }

};

