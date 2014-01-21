////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// scene-specific list of bones for skeletal animation
//
// A mini scene heirachy for one actor.

namespace octet { namespace scene {
  class skeleton : public resource {
    // skeleton components
    dynarray<mat4t> nodeToParents;
    dynarray<atom_t> joints;
    dynarray<ref<scene_node> > nodes;
    dynarray<int> parents;
    dynarray<mat4t> boneToNode;

    // cached skin components
    dynarray<mat4t> result;  /// uniforms to shader
    dynarray<int> indices;   /// map skeleton to skin indices
  public:
    RESOURCE_META(skeleton)

    skeleton() {
    }

    void visit(visitor &v) {
      v.visit(nodeToParents, atom_nodeToParents);
      v.visit(joints, atom_joints);
      v.visit(nodes, atom_nodes);
      v.visit(parents, atom_parents);
      v.visit(boneToNode, atom_boneToNode);
      v.visit(result, atom_result);  /// uniforms to shader
      v.visit(indices, atom_indices);   /// map skeleton to skin indices
    }

    void add_bone(scene_node *node, int parent) {
      nodes.push_back(node);
      nodeToParents.push_back(node->get_nodeToParent());
      joints.push_back(node->get_sid());
      parents.push_back(parent);
      //char tmp[256];
      //log("skeleton: add_bone %d [%s]\n", node->get_sid(), node->access_nodeToParent().toString(tmp, sizeof(tmp)));
    }

    int get_num_bones() const { return result.size(); }

    int find_joint(atom_t sid) {
      for (unsigned i = 0; i != joints.size(); ++i) {
        if (joints[i] == sid) {
          return i;
        }
      }
      return -1;
    }

    mat4t *calc_transforms(const mat4t &worldToCamera, skin *skn) {
      //static bool first_frame = true;
      if (boneToNode.size() < nodeToParents.size()) {
        boneToNode.resize(nodeToParents.size());
      }

      // todo: optionally drive animation directly to the skeleton.
      for (int i = 0; i != nodes.size(); ++i) {
        nodeToParents[i] = nodes[i]->access_nodeToParent();
      }

      // compute matrix heirachy
      for (int i = 0; i != nodeToParents.size(); ++i) {
        int parent = parents[i];
        // skeleton -> parent -> parent -> world -> camera
        if (parent == -1) {
          boneToNode[i] = nodeToParents[i] * worldToCamera;
        } else {
          boneToNode[i] = nodeToParents[i] * boneToNode[parent];
        }
        //log("%d %s p=%d\n", i, result[i].toString(), parent);
      }

      unsigned num_joints = skn->get_num_joints();
      if (result.size() < num_joints) {
        result.resize(num_joints);
        indices.resize(num_joints);
        for (int i = 0; i != num_joints; ++i) {
          // skin -> bind space -> skeleton -> parent -> parent -> world -> camera
          indices[i] = find_joint(skn->get_joint(i));
        }
      }

      // premultiply by skin matrices
      for (int i = 0; i != num_joints; ++i) {
        // skin -> bind space -> skeleton -> parent -> parent -> world -> camera
        int index = indices[i];
        if (index != -1) {
          result[i] = skn->get_modelToBind() * skn->get_bindToModel(i) * boneToNode[index];
        } else {
          result[i] = worldToCamera;
        }
        //if (first_frame) log("%d %d [%s]\n", i, index, result[i].toString());
      }

      return &result[0];
    }

    // convert an sid into an index. (should be cached!)
    int get_bone_index(atom_t sid) {
      for (int i = 0; i != joints.size(); ++i) {
        if (joints[i] == sid) return i;
      }
      return -1;
    }

    void set_bone(int index, const mat4t &value) {
      nodeToParents[index] = value;
    }
  };
}}
