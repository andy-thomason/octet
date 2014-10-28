////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {
  /// Scene containing a box with octet.
  class example_multitexture : public app {
    // scene for drawing box
    ref<visual_scene> app_scene;

    ref<material> custom_mat;

    ref<image> jupiter;
    ref<image> jupiter_mask;

  public:
    /// this is called when we construct the class before everything is initialised.
    example_multitexture(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();

      jupiter = new image("assets/NASA-Jupiter-512.jpg");
      jupiter_mask = new image("assets/NASA-Jupiter-512-mask.gif");

      param_shader *shader = new param_shader("shaders/default.vs", "shaders/multitexture.fs");
      custom_mat = new material(vec4(1, 1, 1, 1), shader);
      custom_mat->add_sampler(0, app_utils::get_atom("jupiter"), jupiter, new sampler());
      custom_mat->add_sampler(1, app_utils::get_atom("jupiter_mask"), jupiter_mask, new sampler());

      mesh_box *box = new mesh_box(vec3(4));
      scene_node *node = new scene_node();
      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, box, custom_mat));
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
      scene_node *node = app_scene->get_mesh_instance(0)->get_node();
      node->rotate(1, vec3(1, 0, 0));
      node->rotate(1, vec3(0, 1, 0));
    }
  };
}
