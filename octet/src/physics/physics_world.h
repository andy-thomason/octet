namespace octet {
  class physics_world {
    btDefaultCollisionConfiguration config;      /// setup for the world
    btCollisionDispatcher dispatcher;            /// handler for collisions between objects
    btDbvtBroadphase broadphase;                 /// handler for broadphase (rough) collision
    btSequentialImpulseConstraintSolver solver;  /// handler to resolve collisions
    btContinuousDynamicsWorld world;             /// physics world, contains rigid bodies

    dynarray<btRigidBody*> rigid_bodies;

  public:

    enum body_kind {
      body_box,
      body_sphere,
      body_cone,
      body_capsule,
      body_static_plane,
      body_convex_mesh,
      body_concave_mesh,
    };

    physics_world() :
      dispatcher(&config),
      world(&dispatcher, &broadphase, &solver, &config
    ) {
    }
  
    // add a rigid body to the world and return a handle to access it
    int add_rigid_body(const mat4t &modelToWorld, const vec4 &param, bool is_dynamic, body_kind kind) {
      btCollisionShape *shape = 0;
      switch (kind) {
        case body_box: shape = new btBoxShape(btVector3(param[0], param[1], param[2])); break;
        case body_sphere: shape = new btSphereShape(param[0]); break;
        case body_cone: shape = new btConeShape(param[0], param[1]); break;
        case body_capsule: shape = new btCapsuleShape(param[0], param[1]); break;
        case body_static_plane: shape = new btStaticPlaneShape(btVector3(param[0], param[1], param[2]), param[3]); break;
        default: printf("warning: bad body kind\n"); return -1;
      }

      btMatrix3x3 matrix(
        modelToWorld[0][0], modelToWorld[0][1], modelToWorld[0][2],
        modelToWorld[1][0], modelToWorld[1][1], modelToWorld[1][2],
        modelToWorld[2][0], modelToWorld[2][1], modelToWorld[2][2]
      );

      btVector3 pos(modelToWorld[3][0], modelToWorld[3][1], modelToWorld[3][2]);

      btTransform transform(matrix, pos);

      btDefaultMotionState *motionState = new btDefaultMotionState(transform);
      btScalar mass = is_dynamic ? 1.0f : 0.0f;
      btVector3 inertiaTensor;
   
      shape->calculateLocalInertia(mass, inertiaTensor);
    
      btRigidBody * rigid_body = new btRigidBody(mass, motionState, shape, inertiaTensor);

      world.addRigidBody(rigid_body);

      int result = rigid_bodies.size();
      rigid_bodies.push_back(rigid_body);
      return result;
    }
  
    void get_modelToWorld(mat4t &modelToWorld, int handle) {
      btRigidBody *rigid_body = rigid_bodies[handle];
      btQuaternion btq = rigid_body->getOrientation();
      btVector3 pos = rigid_body->getCenterOfMassPosition();
      quat q(btq[0], btq[1], btq[2], btq[3]);
      modelToWorld = q;
      modelToWorld[3] = vec4(pos[0], pos[1], pos[2], 1);
    }

    int num_rigid_bodies() {
      return rigid_bodies.size();
    }

    void step(float deltaTime) {
      world.stepSimulation(deltaTime, 4);
    }

    void apply_impulse(int index, const vec4 &worldSpaceImpulse) {
      btVector3 impulse(worldSpaceImpulse[0], worldSpaceImpulse[1], worldSpaceImpulse[2]);
      btRigidBody *body = rigid_bodies[index];
      body->activate();
      body->applyImpulse(impulse, btVector3(0, 0, 0));
    }

    void apply_torque_impulse(int index, const vec4 &worldSpaceTorque) {
      btVector3 torque(worldSpaceTorque[0], worldSpaceTorque[1], worldSpaceTorque[2]);
      btRigidBody *body = rigid_bodies[index];
      body->activate();
      body->applyTorqueImpulse(torque);
    }
  };
}
