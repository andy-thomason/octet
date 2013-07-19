////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// raw 3D mesh container
//

class mesh_instance : public resource, public animation_target {
  // which scene_node (model to world matrix) to use in the scene
  ref<scene_node> node;

  // which mesh to render
  ref<mesh> msh;

  // what material to use
  ref<material> mat;

  // for characters, which skin to use
  ref<skin> skn;

  // for characters, which skeleton to use
  ref<skeleton> skel;

public:
  RESOURCE_META(mesh_instance)

  mesh_instance(scene_node *node, mesh *msh, material *mat, skin *skn=0, skeleton *skel=0) {
    this->node = node;
    this->msh = msh;
    this->mat = mat;
    this->skn = skn;
    this->skel = skel;
  }

  void visit(visitor &v) {
    v.visit(node, "node");
    v.visit(msh, "msh");
    v.visit(mat, "mat");
    v.visit(skn, "skn");
    v.visit(skel, "skel");
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
    if (skel) {
      // todo: cache the index
      int index = skel->get_bone_index(sid);
      if (index != -1) {
        skel->set_bone(index, (mat4t&)*value);
      }
    }
  }

  scene_node *get_node() const { return node; }
  mesh *get_mesh() const { return msh; }
  material *get_material() const { return mat; }
  skin *get_skin() const { return skn; }
  skeleton *get_skeleton() const { return skel; }
};

