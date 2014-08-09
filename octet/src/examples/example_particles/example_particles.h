////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {
  /// Scene containing a particle system
  class example_particles : public app {
    // scene for drawing box
    ref<visual_scene> app_scene;

    // particle system
    ref<mesh_particle_system> system;

    random r;
  public:
    /// this is called when we construct the class before everything is initialised.
    example_particles(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();

      material *sprites = new material(new image("assets/particles.gif"));
      system = new mesh_particle_system();

      scene_node *node = new scene_node();
      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, system, sprites));
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      camera_instance *ci = app_scene->get_camera_instance(0);

      mesh_particle_system::billboard_particle p;
      memset(&p, 0, sizeof(p));
      p.pos = vec3p(0, 0, 0);
      p.size = vec2p(0.5f, 0.5f);
      p.uv_bottom_left = vec2p(0, 1);
      p.uv_top_right = vec2p(0.125f, 1-0.125f);
      p.enabled = true;
      int pidx = system->add_billboard_particle(p);

      mesh_particle_system::particle_animator pa;
      memset(&pa, 0, sizeof(pa));
      pa.link = pidx;
      pa.acceleration = vec3p(0, -9.8f, 0);
      pa.vel = vec3p(r.get(-3.0f, 3.0f), r.get(5.0f, 15.0f), 0.0f);
      pa.lifetime = 50;
      system->add_particle_animator(pa);

      system->set_cameraToWorld(ci->get_node()->calcModelToWorld());
      system->animate(1.0f/30);
      system->update();

      // update matrices. assume 30 fps.
      app_scene->update(1.0f/30);

      // draw the scene
      app_scene->render((float)vx / vy);
    }
  };
}
