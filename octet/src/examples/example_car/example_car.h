////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {
  /// Scene using bullet for physics effects.
  class example_car : public app {
    // scene for drawing box
    ref<visual_scene> app_scene;

    btDefaultCollisionConfiguration config;       /// setup for the world
    btCollisionDispatcher *dispatcher;            /// handler for collisions between objects
    btDbvtBroadphase *broadphase;                 /// handler for broadphase (rough) collision
    btSequentialImpulseConstraintSolver *solver;  /// handler to resolve collisions
    btDiscreteDynamicsWorld *world;             /// physics world, contains rigid bodies

    btRigidBody *add_rigid_body(mat4t_in modelToWorld, vec3_in size, mesh *msh, material *mat, bool is_dynamic=true) {
      btMatrix3x3 matrix(get_btMatrix3x3(modelToWorld));
      btVector3 pos(get_btVector3(modelToWorld[3].xyz()));

      btCollisionShape *shape = new btBoxShape(get_btVector3(size));

      btTransform transform(matrix, pos);

      btDefaultMotionState *motionState = new btDefaultMotionState(transform);
      btScalar mass = is_dynamic ? 1.0f : 0.0f;
      btVector3 inertiaTensor;
   
      shape->calculateLocalInertia(mass, inertiaTensor);
    
      btRigidBody * rigid_body = new btRigidBody(mass, motionState, shape, inertiaTensor);
      world->addRigidBody(rigid_body);

      scene_node *node = new scene_node(modelToWorld, atom_);
      rigid_body->setUserPointer(node);

      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, msh, mat));

      return rigid_body;
    }

    class car : public resource {
    public:
      // construct a car using the simple btRaycastVehicle vehicle model.
      car(example_car *app, mat4t_in modelToWorld, mesh *chassis_mesh, material *chassis_mat, mesh *wheel_mesh, material *wheel_mat) {
        btRaycastVehicle::btVehicleTuning tuning;
        chassis = app->add_rigid_body(modelToWorld, chassis_mesh->get_aabb().get_half_extent(), chassis_mesh, chassis_mat);
        raycaster = new btDefaultVehicleRaycaster(app->world);
        vehicle = new btRaycastVehicle(tuning, chassis, raycaster);
        float wr = wheel_mesh->get_aabb().get_half_extent().y();
        wr = 0.3f;

        vehicle->addWheel(btVector3(-2, -1, 2), btVector3(0, -1, 0), btVector3(-1, 0, 0), 0.5f, wr, tuning, true);
        vehicle->addWheel(btVector3( 2, -1, 2), btVector3(0, -1, 0), btVector3( 1, 0, 0), 0.5f, wr, tuning, true);
        vehicle->addWheel(btVector3(-2, -1,-2), btVector3(0, -1, 0), btVector3(-1, 0, 0), 0.5f, wr, tuning, false);
        vehicle->addWheel(btVector3( 2, -1,-2), btVector3(0, -1, 0), btVector3( 1, 0, 0), 0.5f, wr, tuning, false);

        for (int i = 0; i != 4; ++i) {
          scene_node *node = new scene_node(mat4t(), atom_);
          app->app_scene->add_child(node);
          app->app_scene->add_mesh_instance(new mesh_instance(node, wheel_mesh, wheel_mat));
          wheel_nodes.push_back(node);
        }

        app->world->addAction(vehicle);
      }

      void update() {
        for (int i = 0; i != 4; ++i) {
          btWheelInfo &wi = vehicle->getWheelInfo(i);
          btScalar m[16];
          wi.m_worldTransform.getOpenGLMatrix(m);
          mat4t modelToWorld(
            vec4(m[0], m[1], m[2], m[3]),
            vec4(m[4], m[5], m[6], m[7]),
            vec4(m[8], m[9], m[10], m[11]),
            vec4(m[12], m[13], m[14], m[15])
          );
          wheel_nodes[i]->access_nodeToParent() = modelToWorld;
        }
      }

      ~car() {
        delete vehicle;
        delete raycaster;
      }
    private:
      btRigidBody *chassis;
      btRaycastVehicle *vehicle;
      btDefaultVehicleRaycaster *raycaster;
      dynarray<ref<scene_node> > wheel_nodes;
    };

    dynarray<ref<car> > cars;
  public:
    /// this is called when we construct the class before everything is initialised.
    example_car(int argc, char **argv) : app(argc, argv) {
      dispatcher = new btCollisionDispatcher(&config);
      broadphase = new btDbvtBroadphase();
      solver = new btSequentialImpulseConstraintSolver();
      world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, &config);
    }

    ~example_car() {
      delete world;
      delete solver;
      delete broadphase;
      delete dispatcher;
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();
      app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, 5, 0));

      mat4t modelToWorld;
      material *floor_mat = new material(vec4(0, 1, 0, 1));

      // add the ground (as a static object)
      mesh *floor = new mesh_box(vec3(200.0f, 0.5f, 200.0f));
      add_rigid_body(modelToWorld, vec3(200.0f, 0.5f, 200.0f), floor, floor_mat, false);

      mesh *chassis_mesh = new mesh_box(vec3(1.0f, 0.5f, 1.5f));
      mesh *wheel_mesh = new mesh_box(vec3(0.3f, 0.3f, 0.1f));
      material *chassis_mat = new material(vec4(1, 0, 0, 1));
      material *wheel_mat = new material(vec4(0, 0, 0, 1));

      modelToWorld.loadIdentity();
      modelToWorld.translate(0, 10, 0);
      cars.push_back(new car(this, modelToWorld, chassis_mesh, chassis_mat, wheel_mesh, wheel_mat));
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      // update rigid bodies.
      world->stepSimulation(1.0f/30);

      // update scene nodes with rigid body matrices.
      btCollisionObjectArray &a = world->getCollisionObjectArray();
      for (int i = 0; i != a.size(); ++i) {
        btCollisionObject *object = a[i];
        if (scene_node *node = (scene_node *)object->getUserPointer()) {
          btScalar m[16];
          object->getWorldTransform().getOpenGLMatrix(m);
          mat4t modelToWorld(
            vec4(m[0], m[1], m[2], m[3]),
            vec4(m[4], m[5], m[6], m[7]),
            vec4(m[8], m[9], m[10], m[11]),
            vec4(m[12], m[13], m[14], m[15])
          );
          node->access_nodeToParent() = modelToWorld;
        }
      }

      for (int i = 0; i != cars.size(); ++i) {
        cars[i]->update();
      }

      // update matrices. assume 30 fps.
      app_scene->update(1.0f/30);

      // draw the scene
      app_scene->render((float)vx / vy);
    }
  };
}
