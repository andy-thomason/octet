////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// raw 3D mesh container
//

namespace octet { namespace scene {
  class skin : public resource {
    // the original transform of the skin to world space (bind space)
    mat4t modelToBind;

    // for each scene_node, map from world space (bind space) to model space
    dynarray<mat4t> bindToModel;

    // a name for each joint (sid)
    dynarray<atom_t> joints;

  public:
    RESOURCE_META(skin)

    skin() {
    }

    skin(const mat4t &modelToBind) {
      this->modelToBind = modelToBind;
    }

    void visit(visitor &v) {
      v.visit(modelToBind, atom_modelToBind);
      v.visit(bindToModel, atom_bindToModel);
      v.visit(joints, atom_joints);
    }

    void add_joint(const mat4t &bindToModel, atom_t sid) {
      this->bindToModel.push_back(bindToModel);
      joints.push_back(sid);
      log("skin: add_joint %d\n", sid);
    }

    int find_joint(atom_t sid) {
      for (unsigned i = 0; i != joints.size(); ++i) {
        if (joints[i] == sid) {
          return i;
        }
      }
      return -1;
    }

    const mat4t &get_bindToModel(int i) const { return bindToModel[i]; }
    const mat4t &get_modelToBind() const { return modelToBind; }
    atom_t get_joint(int i) const { return joints[i]; }
    unsigned get_num_joints() const { return joints.size(); }
  };

}}
