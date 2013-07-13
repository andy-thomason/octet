////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// raw 3D mesh container
//

class mesh_instance : public resource {
  // which node (model to world matrix) to use in the scene
  int node;

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

  mesh_instance(int node, mesh_state *mesh, bump_material *mat, skin *skn=0, skeleton *skel=0) {
    this->node = node;
    this->mesh = mesh;
    this->mat = mat;
    this->skn = skn;
    this->skel = skel;
  }

  int get_node() const { return node; }
  const mesh_state *get_mesh() const { return mesh; }
  const bump_material *get_material() const { return mat; }
  const skin *get_skin() const { return skn; }
  const skeleton *get_skeleton() const { return skel; }
};

