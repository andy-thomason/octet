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

	int numPlanks = 9;
	btRigidBody *rb_bridge[11];
	 
	mat4t mat;

	material *red;
	material *green;
	material *blue;
	material *grey;
	material *black;
	material *white;

	btVector3 ballForce = btVector3(0, 0, 0);
	float ballMass = 1;

	mesh_instance *m_ball;
	btRigidBody *rb_ball;

	btRigidBody *rb_swing;

	btTransform ctWorldTransform;

  public:
    example_shapes(int argc, char **argv) : app(argc, argv) {
    }

    ~example_shapes() {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();
      app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, 4, 0));

	  red = new material(vec4((1.0f, 0.0f, 0.0f, 1.0f)));
	  green = new material(vec4((0.0f, 1.0f, 0.0f, 1.0f)));
	  blue = new material(vec4((0.0f, 0.0f, 1.0f, 1.0f)));
	  grey = new material(vec4((0.5f, 0.5f, 0.5f, 1.0f)));
	  black = new material(vec4((1.0f, 1.0f, 1.0f, 1.0f)));
	  white = new material(vec4((0.0f, 0.0f, 0.0f, 1.0f)));

	  //West Side of bridge
	  mat.loadIdentity();
	  mat.translate(-14, 0, 0);
	  mesh_instance *west_side = app_scene->add_shape(mat, new mesh_box(vec3(1, 1, 20), 1), green, false);
	  rb_bridge[0] = west_side->get_node()->get_rigid_body();

	  //Run through and draw planks of bridge
	  for (int i = 1; i < numPlanks+1; i++) {
		  mat.loadIdentity();
		  mat.translate(-12.0f + (i * 2), 0.0f, 0.0f);
		  mesh_instance *p = app_scene->add_shape(mat, new mesh_box(vec3(0.8f, 0.1f, 10), 1), grey, false);
		  rb_bridge[i] = p->get_node()->get_rigid_body();
	  }

	  //East Side of bridge
	  mat.loadIdentity();
	  mat.translate(14, 0, 0);
	  mesh_instance *east_side = app_scene->add_shape(mat, new mesh_box(vec3(1, 1, 20), 1), green, false);
	  rb_bridge[numPlanks + 1] = west_side->get_node()->get_rigid_body();

	  //Constraints
	  btTransform tran1A = btTransform::getIdentity();
	  tran1A.setOrigin(btVector3(0.8f, 0.0f, 18.0f));

	  btTransform tran1B = btTransform::getIdentity();
	  tran1B.setOrigin(btVector3(-0.8f, 0.0f, 18.0f));

	  btTransform tran2A = btTransform::getIdentity();
	  tran2A.setOrigin(btVector3(0.8f, 0.0f, -18.0f));

	  btTransform tran2B = btTransform::getIdentity();
	  tran2B.setOrigin(btVector3(-0.8f, 0.0f, -18.0f));

	  //Apply the spring to the rb_bridge array populated above

	  for (int i = 0; i < numPlanks ; i++) {
		  app_scene->addSpring(rb_bridge[i], rb_bridge[i + 1], tran1A, tran1B);
		  app_scene->addSpring(rb_bridge[i], rb_bridge[i + 1], tran2A, tran2B);
	  }


	  rb_swing = drawBox(mat, vec3(0, -2, 0), vec3(1, 1, 1), vec4(0, 1, 1, 1), true);

	  app_scene->addHinge(rb_swing, btVector3(0, 6, 0), btVector3(0, 0, 1));
	  // rb_swing->applyTorque(btVector3(0, 0, 10));


	  //draw the water under the bridge
	  drawBox(mat, vec3(0, -7, 0), vec3(50, 1, 50), vec4(0.2f, 0.2f, 1.0f, 1), false); //water

    }

	btRigidBody* drawBox(mat4t _mat, vec3 _pos, vec3 _size, material _col, bool isMovable) {
		_mat.loadIdentity();
		_mat.translate(_pos);
		material *locMat = new material(_col);
		mesh_instance *m = app_scene->add_shape(_mat, new mesh_box(_size), locMat, isMovable);
		btRigidBody *rb = m->get_node()->get_rigid_body();

		return rb;
	}

	void drawSphere(mat4t _mat, vec3 _pos, vec3 _size, float _radius, material _col, bool isMovable) {
		_mat.loadIdentity();
		_mat.translate(_pos);
		material *locMat = new material(_col);
		m_ball = app_scene->add_shape(_mat, new mesh_sphere(_size, _radius), locMat, isMovable, ballMass);
		rb_ball = m_ball->get_node()->get_rigid_body();
	}

	void move_Ball() {
		//printf("%f %f %f\n", ballForce.x(), ballForce.y(), ballForce.z());

		// left and right arrows, moving along the X axis
		if (is_key_down(key_left)) {
			ballForce += btVector3(-0.3f, 0, 0);
		}
		else if (is_key_down(key_right)) {
			ballForce += btVector3(0.3f, 0, 0);
		}
		//Up and Down Arrows, moving along the Z axis
		else if (is_key_down(key_up)) {
			ballForce += btVector3(0, 0, -0.1f);
		}
		else if (is_key_down(key_down)) {
			ballForce += btVector3(0, 0, 0.1f);
		}
		else {
			ballForce = btVector3(0, 0, 0);
		}
		rb_ball->applyForce(ballForce, btVector3(0, 0, 0));
	}

	
	

	void spawnPlayer() {
		if (is_key_down(key_S) || is_key_down(key_s)) {
			mat.loadIdentity();
			mat.translate(11, 2, 0);
			app_scene->add_shape(mat, new mesh_sphere(vec3(0.5f, 0.5f, 0.5f), 1), black, true);
		}
	}



    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
		
		move_Ball();
		
		spawnPlayer();

		//moveSwing();
		
		int vx = 0, vy = 0;
		get_viewport_size(vx, vy);
		app_scene->begin_render(vx, vy);

      // update matrices. assume 30 fps.
      app_scene->update(1.0f/30);

      // draw the scene
      app_scene->render((float)vx / vy);
    }
  };
}
