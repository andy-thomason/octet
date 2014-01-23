////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Scene Node
//

namespace octet { namespace scene {
  /// Scene node. Part of a scene heirachy.
  /// Each node has a transform matrix, an identifying atom (sid), a parent and children.
  class scene_node : public resource {
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

    /// Construct a scene node with an identity transform and no parent.
    scene_node() {
      nodeToParent.loadIdentity();
      sid = atom_;
    }

    /// Construct a scene node with a matrix and an identifying sid atom.
    scene_node(const mat4t &nodeToParent, atom_t sid) {
      this->nodeToParent = nodeToParent;
      this->sid = sid;
    }

    /// the virtual add_ref on animation_target gets passed to here and we pass iton (delegate it) to the resource
    void add_ref() {
      resource::add_ref();
    }

    /// the virtual release on animation_target gets passed to here and we pass iton (delegate it) to the resource
    void release() {
      resource::release();
    }

    /// animation input: for now, we only support skeleton animation
    void set_value(atom_t sid, atom_t sub_target, atom_t component, float *value) {
      if (sub_target == atom_transform) {
        nodeToParent.init_transpose(value);
      }
    }

    /// visitor pattern used for game saves/loads (serialisation)
    void visit(visitor &v) {
      //log("visit scene_node\n");
      v.visit(parent, atom_parent);
      //log("visit scene_node children\n");
      v.visit(children, atom_children);
      //log("visit scene_node nodeToParent\n");
      v.visit(nodeToParent, atom_nodeToParent);
      v.visit(sid, atom_sid);
    }


    /// add a child node to this node.
    void add_child(scene_node *new_node) {
      new_node->parent = this;
      children.push_back(new_node);
    }

    /// Get the parent node of this node.
    scene_node *get_parent() {
      return parent;
    }

    /// Get the number of chilren for iteration.
    int get_num_children() {
      return children.size();
    }

    /// Get a specific child node.
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

    /// read the node to parent transform matrix
    const mat4t &get_nodeToParent() const {
      return nodeToParent;
    }

    /// access the node to parent transform matrix for writing.
    mat4t &access_nodeToParent() {
      return nodeToParent;
    }

    /// reset the matrix
    void loadIdentity() {
      nodeToParent.loadIdentity();
    }

    /// Translate the matrix
    void translate(vec3_in xyz) {
      nodeToParent.translate(xyz[0], xyz[1], xyz[2]);
    }

    /// Rotate the matrix
    void rotate(float angle, vec3_in axis) {
      nodeToParent.rotate(angle, axis[0], axis[1], axis[2]);
    }

    /// Scale the matrix
    void scale(vec3_in xyz) {
      nodeToParent.scale(xyz[0], xyz[1], xyz[2]);
    }

    /// Get the identifying sid
    atom_t get_sid() {
      return sid;
    }

    /// recursively fetch all child nodes
    void get_all_child_nodes(dynarray<scene_node*> &nodes, dynarray<int> &parents) {
      dynarray<scene_node*> stack;
      dynarray<int> parent_stack;
      stack.push_back(this);
      parent_stack.push_back(-1);
      while (!stack.empty()) {
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
}}
