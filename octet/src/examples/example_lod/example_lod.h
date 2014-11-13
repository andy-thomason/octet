////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {
  /// Scene containing a box with octet.
  class example_lod : public app {
    // scene for drawing box
    ref<visual_scene> app_scene;

    vec3 cam_pos;
  public:
    /// this is called when we construct the class before everything is initialised.
    example_lod(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();
      app_scene->get_camera_instance(0)->set_far_plane(1000);
      cam_pos = vec3(0, 2, 0);

      // material used by all spheres.
      material *mat = new material(vec4(1, 0, 0, 1));

      // second material to show effect
      material *mat2 = new material(vec4(0, 1, 0, 1));

      // four different LOD models
      mesh_sphere *spheres[] = {
        new mesh_sphere(vec3(0), 0.5f, 3),
        new mesh_sphere(vec3(0), 0.5f, 2),
        new mesh_sphere(vec3(0), 0.5f, 1),
        new mesh_sphere(vec3(0), 0.5f, 0)
      };

      // distances where LOD values change
      static const float distances[] = {
        //-1e37f, 3, 6, 18, 1e37f   // high performance, low quality
        //-1e37f, 3, 6, 18, 1e37f   // high performance, low quality
        -1e37f, 5, 15, 45, 1e37f  // medium performance and quality
        //-1e37f, 5, 25, 125, 1e37f // low performance, high quality
      };

      // materials for LODs (it is common to have simpler shaders for further objects).
      material *mats[] = {
        mat, mat, mat, mat  // show smallest lod in different cocalclour
        //mat, mat, mat, mat // regular setting
      };

      for (int x = 0; x <= 20; ++x) {
        for (int y = 0; y <= 5; ++y) {
          for (int z = 0; z <= 100; ++z) {
            scene_node *node = new scene_node();
            node->translate(vec3((x-10.0f) * 2.0f, (y - 2.5f) * 2.0f, -z * 2.0f));
            app_scene->add_child(node);
            for (int k = 0; k != 4; ++k) {
              mesh_instance *mi = new mesh_instance(node, spheres[k], mats[k]);
              mi->set_min_draw_distance(distances[k]);
              mi->set_max_draw_distance(distances[k+1]);
              mi->set_flags(mesh_instance::flag_enabled|mesh_instance::flag_lod);
              app_scene->add_mesh_instance(mi);
            }
          }
        }
      }
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);

      scene_node *cam_node = app_scene->get_camera_instance(0)->get_node();
      cam_node->loadIdentity();
      cam_node->translate(cam_pos);
      cam_pos += vec3(0, 0, -0.3f);

      app_scene->begin_render(vx, vy);

      // update matrices. assume 30 fps.
      app_scene->update(1.0f/30);

      // draw the scene
      app_scene->render((float)vx / vy);
    }
  };
}
