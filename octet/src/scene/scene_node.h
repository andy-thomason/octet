////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Scene Node
//

namespace octet {
  class scene_node : public resource, public animation_target {
    // every scene_node has a parent scene_node except the roots (NULL)
    // todo: support DAGs with multiple node parents
    ref<scene_node> parent;

    // child nodes
    dynarray<ref<scene_node> > children;

    // array of relative transforms (indexed by scene_node index)
    mat4t nodeToParent;

    // sid used to target animations
    atom_t sid;
  public:
    RESOURCE_META(scene_node)

    animation_target *get_animation_target() { return (animation_target*)this; }

    scene_node() {
      nodeToParent.loadIdentity();
      sid = atom_;
    }

    scene_node(const mat4t &nodeToParent, atom_t sid) {
      this->nodeToParent = nodeToParent;
      this->sid = sid;
    }

    // the virtual add_ref on animation_target gets passed to here and we pass iton (delegate it) to the resource
    void add_ref() {
      resource::add_ref();
    }

    // the virtual release on animation_target gets passed to here and we pass iton (delegate it) to the resource
    void release() {
      resource::release();
    }

    // animation input: for now, we only support skeleton animation
    void set_value(atom_t sid, atom_t sub_target, atom_t component, float *value) {
      if (sub_target == atom_transform) {
        nodeToParent.init_transpose(value);
      }
    }

    //
    // visitor pattern used for game saves/loads (serialisation)
    //
    void visit(visitor &v) {
      v.visit(parent, atom_parent);
      v.visit(children, atom_children);
      v.visit(nodeToParent, atom_nodeToParent);
      v.visit(sid, atom_sid);
    }

    void add_child(scene_node *new_node) {
      new_node->parent = this;
      children.push_back(new_node);
    }

    scene_node *get_parent() {
      return parent;
    }

    int get_num_children() {
      return children.size();
    }

    scene_node *get_child(int index) {
      return children[index];
    }

    // compute the scene_node to world matrix for an individual scene_node;
    mat4t calcModelToWorld() {
      mat4t result = nodeToParent;
      for (scene_node *p = parent; p != NULL; p = p->parent) {
        result = result * p->nodeToParent;
      }
      return result;
    }

    const mat4t &get_nodeToParent() const {
      return nodeToParent;
    }

    mat4t &access_nodeToParent() {
      return nodeToParent;
    }

    atom_t get_sid() {
      return sid;
    }

    // recursively fetch all child nodes
    void get_all_child_nodes(dynarray<scene_node*> &nodes, dynarray<int> &parents) {
      dynarray<scene_node*> stack;
      dynarray<int> parent_stack;
      stack.push_back(this);
      parent_stack.push_back(-1);
      while (!stack.is_empty()) {
        scene_node *node = stack.back();
        int parent = parent_stack.back();
        int new_parent = nodes.size();
        stack.pop_back();
        parent_stack.pop_back();
        nodes.push_back(node);
        parents.push_back(parent);
        for (int i = 0; i != node->children.size(); ++i) {
          stack.push_back(node->children[i]);
          parent_stack.push_back(new_parent);
        }
      }
    }
  };
}
