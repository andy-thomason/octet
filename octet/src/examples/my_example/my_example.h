////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {
  /// Scene containing a box with octet.
  class my_example : public app {
    // scene for drawing box
    ref<visual_scene> app_scene;
  public:
    /// this is called when we construct the class before everything is initialised.
    my_example(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();
	  
	  if (true) // TRUE = topDown  - FALSE = side
	  {
		  app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, 60, 0));
		  app_scene->get_camera_instance(0)->get_node()->rotate(-90, vec3(1, 0, 0));
	  }
	  else
	  {
		  app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, 0, 50));
	  }
	  mat4t mat;
	  material *green = new material(vec4(0, 1, 0, 1));
	  material *darkgreen = new material(vec4(0, 0.5f, 0, 1));
	  material *red = new material(vec4(1, 0, 0, 1));
	  material *blue = new material(vec4(0, 0, 1, 1));
      

	  mat.loadIdentity();
	  mat.translate(0, -1, 35);
	  app_scene->add_shape(mat, new mesh_box(vec3(16, 1, 20)), green, false);


	  mat.loadIdentity();
	  mat.translate(15, -1, 35);
	  app_scene->add_shape(mat, new mesh_box(vec3(1, 3, 21)), darkgreen, false);
	  mat.loadIdentity();
	  mat.translate(-15, -1, 35);
	  app_scene->add_shape(mat, new mesh_box(vec3(1, 3, 21)), darkgreen, false);
	  mat.loadIdentity();
	  mat.translate(0, -1, 15);
	  app_scene->add_shape(mat, new mesh_box(vec3(16, 3, 1)), darkgreen, false);
	  mat.loadIdentity();
	  mat.translate(0, -1, 55);
	  app_scene->add_shape(mat, new mesh_box(vec3(16, 3, 1)), darkgreen, false);

	  mat.loadIdentity();
	  mat.translate(0, 0, 35);
	  app_scene->add_shape(mat, new mesh_sphere(vec3(0), 1), blue, true);

	  mat.loadIdentity();
	  mat.translate(12, 0, 32);
	  app_scene->add_shape(mat, new mesh_sphere(vec3(0), 1), red, true);

	  mat.loadIdentity();
	  mat.translate(5, 0, 41);
	  app_scene->add_shape(mat, new mesh_sphere(vec3(0), 1), red, true);

	  mat.loadIdentity();
	  mat.translate(1, 0, 27);
	  app_scene->add_shape(mat, new mesh_sphere(vec3(0), 1), red, true);


	  scene_node *ground = app_scene->get_mesh_instance(0)->get_node();
	  scene_node *wall = app_scene->get_mesh_instance(1)->get_node();
	  scene_node *wall2 = app_scene->get_mesh_instance(2)->get_node();
	  scene_node *wall3 = app_scene->get_mesh_instance(3)->get_node();
	  scene_node *wall4 = app_scene->get_mesh_instance(4)->get_node();
	  scene_node *ball = app_scene->get_mesh_instance(5)->get_node();
	  scene_node *redBall1 = app_scene->get_mesh_instance(6)->get_node();
	  scene_node *redBall2 = app_scene->get_mesh_instance(7)->get_node();
	  scene_node *redBall3 = app_scene->get_mesh_instance(8)->get_node();


	  ball->set_linear_velocity(vec3(getRandomFloat(30), 0, getRandomFloat(30)));
	  ball->set_friction(0.1f);
	  ball->set_resitution(1.0f);

	  redBall1->set_linear_velocity(vec3(getRandomFloat(30), 0, getRandomFloat(30)));
	  redBall1->set_friction(0.1f);
	  redBall1->set_resitution(1.0f);
	  redBall2->set_linear_velocity(vec3(getRandomFloat(30), 0, getRandomFloat(30)));
	  redBall2->set_friction(0.1f);
	  redBall2->set_resitution(1.0f);
	  redBall3->set_linear_velocity(vec3(getRandomFloat(30), 0, getRandomFloat(30)));
	  redBall3->set_friction(0.1f);
	  redBall3->set_resitution(1.0f);


	  ground->set_resitution(1.0f);
	  wall->set_resitution(1.0f);
	  wall2->set_resitution(1.0f);
	  wall3->set_resitution(1.0f);
	  wall4->set_resitution(1.0f);

    }

	float getRandomFloat(int max)
	{
		return rand() % max;
	}

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      // update matrices. assume 30 fps.
      app_scene->update(1.0f/30);

	  

      // draw the scene
      app_scene->render((float)vx / vy);
	  

      // tumble the box  (there is only one mesh instance)
    }
  };
}
