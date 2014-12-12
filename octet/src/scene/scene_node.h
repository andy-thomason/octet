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

    // this node's transform relative to parent
    mat4t nodeToParent;

    // sid used to target animations
    atom_t sid;

    // is this node and all its children renderable?
    bool enabled;

  public:
    RESOURCE_META(scene_node)

    /// Construct a scene node with an identity transform and no parent.
    scene_node(scene_node *parent = 0) {
      nodeToParent.loadIdentity();
      sid = atom_;
      enabled = true;
      if (parent) {
        parent->add_child(this);
      }
    }

    /// Construct a scene node with a matrix and an identifying sid atom.
    scene_node(const mat4t &nodeToParent, atom_t sid) {
      this->nodeToParent = nodeToParent;
      this->sid = sid;
      enabled = true;
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

    // calculate whether this node is enabled (recursively)
    bool calcEnabled() {
      for (scene_node *p = this; p != NULL; p = p->parent) {
        if (!p->enabled) return false;
      }
      return true;
    }

    /// transform a point from model space to world space
    vec3 transform(vec3_in world_pos) {
      mat4t model_to_world = calcModelToWorld();
      return world_pos * model_to_world;
    }

    /// transform a point from world space to model space
    vec3 inverse_transform(vec3_in world_pos) {
      mat4t model_to_world = calcModelToWorld();
      // this can be done more efficiently
      mat4t world_to_model = model_to_world.inverse3x4();
      return world_pos * world_to_model;
    }

    /// read the node to parent transform matrix
    const mat4t &get_nodeToParent() const {
      return nodeToParent;
    }

    /// access the node to parent transform matrix for writing.
    mat4t &access_nodeToParent() {
      return nodeToParent;
    }

    /// get the x axis (left, right) of the node
    vec3 get_x() {
      return calcModelToWorld().x().xyz();
    }

    /// get the y axis (up, down) of the node
    vec3 get_y() {
      return calcModelToWorld().y().xyz();
    }

    /// get the z axis (forward, back) of the node
    vec3 get_z() {
      return calcModelToWorld().z().xyz();
    }

    /// get the position of the node in world space
    vec3 get_position() {
      return calcModelToWorld().w().xyz();
    }

    /// get enabled state
    bool get_enabled() const {
      return enabled;
    }

    /// set enabled state
    void set_enabled(bool value) {
      enabled = value;
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

    #ifdef OCTET_BULLET
    private:
      btRigidBody *rigid_body;
    public:
      /// get the rigid body associated with this node (used for physics)
      btRigidBody *get_rigid_body() const {
        return rigid_body;
      }

      /// set the rigid body associated with this node (used for physics)      
      void set_rigid_body(btRigidBody *value) {
        rigid_body = value;
      }

      /// set the mass and inertia tensor
      void set_mass(float mass, vec3_in inertia) {
        rigid_body->setMassProps(mass, get_btVector3(inertia));
      }

      /// set the linear and angular damping
      void set_damping(float linear_damping, float angular_damping) {
        rigid_body->setDamping(linear_damping, angular_damping);
      }

      /// apply a force at the centre of gravity of the object (so it does not spin)
      void apply_central_force(vec3_in value) {
        rigid_body->applyCentralForce(get_btVector3(value));
      }

      /// apply a force at a position local to the object
      void apply_model_space_force(vec3_in value, vec3_in model_pos) {
        rigid_body->applyForce(get_btVector3(value), get_btVector3(model_pos));
      }

      /// apply a torque to the object
      void apply_torque(vec3_in value) {
        rigid_body->applyTorque(get_btVector3(value));
      }

      /// set the sliding friction of the object.
      void set_friction(float value) {
        rigid_body->setFriction(value);
      }

      /// set the rolling friction of the object (tyres for example).
      void set_rolling_friction(float value) {
        rigid_body->setRollingFriction(value);
      }

      /// set the "bounciness" of the object. 0 is dead, 1 is bouncy.
      void set_resitution(float value) {
        rigid_body->setRestitution(value);
      }

      /// brute force set the angular velocity (spin) of the object directly: warning, this may break something!
      void set_angular_velocity(vec3_in value) {
        rigid_body->setAngularVelocity(get_btVector3(value));
      }

      /// brute force set the linear velocity of the object directly: warning, this may break something!
      void set_linear_velocity(vec3_in value) {
        rigid_body->setLinearVelocity(get_btVector3(value));
      }

      /// brute force transform set: warning, this may break something!
      void set_transform(mat4t_in value) {
        btTransform trans;// = rigid_body->getWorldTransform();
        trans.setFromOpenGLMatrix(value.get());
        rigid_body->setWorldTransform(trans);
      }

      /// brute force transform set: warning, this may break something!
      void set_position(vec3_in value) {
        btTransform trans = rigid_body->getWorldTransform();
        trans.setOrigin(get_btVector3(value));
        rigid_body->setWorldTransform(trans);
      }

      /// brute force tranform set: warning, this may break something!
      void set_rotation(mat4t_in value) {
        btTransform trans = rigid_body->getWorldTransform();
        trans.setBasis(get_btMatrix3x3(value));
        rigid_body->setWorldTransform(trans);
      }

      /// activate the rigid body. You must do this periodicaly if you want your object to stay awake (see fps example).
      void activate() {
        rigid_body->activate();
      }

      /// This is the amount that the body will respond to torques in certain directions.
      void set_angular_factor(vec3_in value) {
        rigid_body->setAngularFactor(get_btVector3(value));
      }

      /// This is the amount that the body will respond to forces in diferrent directions.
      void set_linear_factor(vec3_in value) {
        rigid_body->setLinearFactor(get_btVector3(value));
      }

      /// gravity is a constant force that affects the object.
      void set_gravity(vec3_in value) {
        rigid_body->setGravity(get_btVector3(value));
      }

      /// sleep the object if its velocity falls below these values.
      void set_sleeping_thresholds(float linear, float angular) {
        rigid_body->setSleepingThresholds(linear, angular);
      }

      /// get the current position and orientation of the matrix.
      mat4t get_physics_transform() const {
        mat4t result;
        rigid_body->getWorldTransform().getOpenGLMatrix(result.get());
        return result;
      }

      /// get the angular velocity (spin) of the object.
      vec3_ret get_angular_velocity() const {
        return get_vec3(rigid_body->getAngularVelocity());
      }

      /// get the linear velocity of the object.
      vec3_ret get_linear_velocity() const {
        return get_vec3(rigid_body->getLinearVelocity());
      }

      /// set a speed limit for this object on this frame
      void clamp_linear_velocity(float max_speed) {
        btVector3 vel = rigid_body->getLinearVelocity();
        float s2 = vel.dot(vel);
        if (s2 > max_speed * max_speed) {
          rigid_body->setLinearVelocity(vel * (max_speed/std::sqrt(s2)));
        }
      }
    #endif
  };
}}
