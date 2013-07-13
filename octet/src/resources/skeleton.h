////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// scene-specific list of bones for skeletal animation
//

class skeleton : public resource {
  dynarray<int> bones;
public:
  RESOURCE_META(skeleton)

  skeleton() {
  }

  void add_bone(int node_index) {
    bones.push_back(node_index);
  }

  int get_num_bones() const { return bones.size(); }
  int get_bone(int i) const { return bones[i]; }
};

