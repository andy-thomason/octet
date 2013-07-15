////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// scene-specific list of bones for skeletal animation
//

class skeleton : public resource {
  dynarray<mat4t> nodeToParents;
  dynarray<mat4t> nodeToCameras;
  dynarray<atom_t> sids;
  dynarray<int> parents;
public:
  RESOURCE_META(skeleton)

  skeleton() {
  }

  void add_bone(const mat4t &nodeToParent, atom_t sid, int parent) {
    nodeToCameras.push_back(nodeToParent);
    nodeToParents.push_back(nodeToParent);
    sids.push_back(sid);
    parents.push_back(parent);
    //app_utils::log("add_bone [%s]\n", nodeToParent.toString());
  }

  int get_num_bones() const { return nodeToParents.size(); }

  mat4t *calc_transforms(const mat4t &worldToCamera, skin *skn) {
    for (int i = 0; i != nodeToParents.size(); ++i) {
      int parent = parents[i];
      // scene_node -> parent -> parent -> world -> camera
      if (parent == -1) {
        nodeToCameras[i] = nodeToParents[i] * worldToCamera;
      } else {
        nodeToCameras[i] = nodeToParents[i] * nodeToCameras[parent];
      }
      //app_utils::log("%d %s p=%d\n", i, nodeToCameras[i].toString(), parent);
    }
    for (int i = 0; i != nodeToParents.size(); ++i) {
      // mesh -> bind -> model -> scene_node -> parent -> parent -> world -> camera
      nodeToCameras[i] = skn->get_modelToBind() * skn->get_bindToModel(i) * nodeToCameras[i];
      //app_utils::log("%d [%s]\n", i, nodeToCameras[i].toString());
    }
    return &nodeToCameras[0];
  }
};

