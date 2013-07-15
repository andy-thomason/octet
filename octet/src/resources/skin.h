////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// raw 3D mesh container
//

class skin : public resource {
  // the original transform of the skin to world space (bind space)
  mat4t modelToBind;

  // for each scene_node, map from world space (bind space) to model space
  dynarray<mat4t> bindToModel;

  // a name for each joint (sid)
  dynarray<atom_t> joints;

public:
  RESOURCE_META(skin)

  skin(const mat4t &modelToBind) {
    this->modelToBind = modelToBind;
  }

  void add_joint(const mat4t &bindToModel, atom_t sid) {
    this->bindToModel.push_back(bindToModel);
    joints.push_back(sid);
  }

  const mat4t &get_bindToModel(int i) const { return bindToModel[i]; }
  const mat4t &get_modelToBind() const { return modelToBind; }
};

