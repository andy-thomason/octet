////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// raw 3D mesh container
//

class mesh_instance : public resource, public animation_target {
  // which scene_node (model to world matrix) to use in the scene
  ref<scene_node> node_;

  // which mesh to render
  ref<mesh_state> mesh;

  // what material to use
  ref<bump_material> mat;

  // for characters, which skin to use
  ref<skin> skn;

  // for characters, which skeleton to use
  ref<skeleton> skel;

public:
  RESOURCE_META(mesh_instance)

  mesh_instance(scene_node *node_, mesh_state *mesh, bump_material *mat, skin *skn=0, skeleton *skel=0) {
    this->node_ = node_;
    this->mesh = mesh;
    this->mat = mat;
    this->skn = skn;
    this->skel = skel;
  }

  //////////////////////////////
  //
  // animation_target

  void add_ref() {
    resource::add_ref();
  }

  void release() {
    resource::release();
  }

  void set_value(atom_t sid, float *value) {
  }

  scene_node *get_node() const { return node_; }
  mesh_state *get_mesh() const { return mesh; }
  bump_material *get_material() const { return mat; }
  skin *get_skin() const { return skn; }
  skeleton *get_skeleton() const { return skel; }
};

