////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {
  /// Scene containing a box with octet.
  class example_shapes : public app {
    // scene for drawing box
    ref<visual_scene> app_scene;

    btDefaultCollisionConfiguration config;       /// setup for the world
    btCollisionDispatcher *dispatcher;            /// handler for collisions between objects
    btDbvtBroadphase *broadphase;                 /// handler for broadphase (rough) collision
    btSequentialImpulseConstraintSolver *solver;  /// handler to resolve collisions
    btDiscreteDynamicsWorld *world;             /// physics world, contains rigid bodies

    void add_shape(mat4t_in mat, mesh *msh, material *mtl, bool is_dynamic) {
      scene_node *node = new scene_node();
      node->access_nodeToParent() = mat;
      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, msh, mtl));

      btMatrix3x3 matrix(get_btMatrix3x3(mat));
      btVector3 pos(get_btVector3(mat[3].xyz()));

      btCollisionShape *shape = is_dynamic ? msh->get_bullet_shape() : msh->get_static_bullet_shape();
      if (shape) {
        btTransform transform(matrix, pos);

        btDefaultMotionState *motionState = new btDefaultMotionState(transform);
        btScalar mass = is_dynamic ? 1.0f : 0.0f;
        btVector3 inertiaTensor;
   
        if (is_dynamic) shape->calculateLocalInertia(mass, inertiaTensor);
    
        btRigidBody * rigid_body = new btRigidBody(mass, motionState, shape, inertiaTensor);
        world->addRigidBody(rigid_body);
        rigid_body->setUserPointer(node);
      }
    }

  public:
    example_shapes(int argc, char **argv) : app(argc, argv) {
      dispatcher = new btCollisionDispatcher(&config);
      broadphase = new btDbvtBroadphase();
      solver = new btSequentialImpulseConstraintSolver();
      world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, &config);
    }

    ~example_shapes() {
      delete world;
      delete solver;
      delete broadphase;
      delete dispatcher;
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();
      app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, 4, 0));

      material *red = new material(vec4(1, 0, 0, 1));
      material *green = new material(vec4(0, 1, 0, 1));
      material *blue = new material(vec4(0, 0, 1, 1));

      mat4t mat;
      mat.translate(-3, 6, 0);
      add_shape(mat, new mesh_sphere(vec3(2, 2, 2), 2), red, true);

      mat.loadIdentity();
      mat.translate(0, 3, 0);
      add_shape(mat, new mesh_box(vec3(2, 2, 2)), red, true);

      mat.loadIdentity();
      mat.translate( 3, 6, 0);
      add_shape(mat, new mesh_cylinder(zcylinder(vec3(0, 0, 0), 2, 4)), blue, true);

      // ground
      mat.loadIdentity();
      mat.translate(0, -1, 0);
      add_shape(mat, new mesh_box(vec3(200, 1, 200)), green, false);
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      world->stepSimulation(1.0f/30, 1, 1.0f/30);
      btCollisionObjectArray &array = world->getCollisionObjectArray();
      for (int i = 0; i != array.size(); ++i) {
        btCollisionObject *co = array[i];
        scene_node *node = (scene_node *)co->getUserPointer();
        if (node) {
          mat4t &mat = node->access_nodeToParent();
          co->getWorldTransform().getOpenGLMatrix(mat.get());
        }
      }

      // update matrices. assume 30 fps.
      app_scene->update(1.0f/30);

      // draw the scene
      app_scene->render((float)vx / vy);
    }
  };
}
