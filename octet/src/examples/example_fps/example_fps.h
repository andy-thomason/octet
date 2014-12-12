////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {
  /// Scene containing a box with octet.
  class example_fps : public app {
    // scene for drawing box
    ref<visual_scene> app_scene;

    mouse_look mouse_look_helper;

    ref<scene_node> player_node;

    ref<camera_instance> the_camera;

    struct example_geometry_source : mesh_terrain::geometry_source {
      mesh::vertex vertex(vec3_in bb_min, vec3_in uv_min, vec3_in uv_delta, vec3_in pos) {
        static const vec3 bumps[] = { vec3(100, 0, 100), vec3(50, 0, 50), vec3(150, 0, 50) };

        float y =
          expf((pos - bumps[0]).squared() / (-100.0f)) * 3.0f +
          expf((pos - bumps[1]).squared() / (-100.0f)) * 4.0f +
          expf((pos - bumps[2]).squared() / (-10000.0f)) * (-20.0f) +
          (15.0f)
        ;

        //float y = sinf(xz.x() * 0.01f) * 1.0f + sinf(xz.z() * 0.03f) * 0.5f;
        float dy_dx = std::cos(pos.x() * 0.01f);
        float dy_dz = std::cos(pos.z() * 0.03f);
        vec3 p = bb_min + pos + vec3(0, y, 0);
        vec3 normal = normalize(vec3(dy_dx, 1, dy_dz));
        vec3 uv = uv_min + vec3((float)pos.x(), (float)pos.z(), 0) * uv_delta;
        return mesh::vertex(p, normal, uv);
      }
    };

    example_geometry_source source;

    void update_keys(scene_node *player_node, scene_node *camera_node) {
      float friction = 0.0f;
      if (is_key_down('A')) {
        player_node->activate();
        player_node->apply_central_force(camera_node->get_x() * (-1000.0f));
      } else if (is_key_down('D')) {
        player_node->activate();
        player_node->apply_central_force(camera_node->get_x() * (+1000.0f));
      } else if (is_key_down('W')) {
        player_node->activate();
        player_node->apply_central_force(camera_node->get_z() * (-1000.0f));
      } else if (is_key_down('S')) {
        player_node->activate();
        player_node->apply_central_force(camera_node->get_z() * (+1000.0f));
      } else {
        friction = 1.0f;
      }
      if (is_key_going_down(' ')) {
        player_node->apply_central_force(camera_node->get_y() * (+10000.0f));
      }
      player_node->set_friction(friction);
    }
  public:
    /// this is called when we construct the class before everything is initialised.
    example_fps(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      mouse_look_helper.init(this, 200.0f / 360.0f, false);
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();
      the_camera = app_scene->get_camera_instance(0);
      the_camera->get_node()->translate(vec3(0, 4, 0));
      the_camera->set_far_plane(10000);

      mat4t mat;

      mat.loadIdentity();
      mat.translate(0, -0.5f, 0);

      app_scene->add_shape(
        mat,
        new mesh_terrain(vec3(100.0f, 0.5f, 100.0f), ivec3(100, 1, 100), source),
        new material(new image("assets/grass.jpg")),
        false, 0
      );

      float player_height = 1.83f;
      float player_radius = 0.25f;
      float player_mass = 90.0f;

      mat.loadIdentity();
      mat.translate(0, player_height*0.5f, -50);

      mesh_instance *mi = app_scene->add_shape(
        mat,
        new mesh_sphere(vec3(0), player_radius),
        new material(vec4(0, 0, 1, 1)),
        true, player_mass,
        new btCapsuleShape(0.25f, player_height)
      );
      player_node = mi->get_node();

      player_node->set_friction(0);
      player_node->set_damping(0, 0);
      player_node->set_angular_factor(0);
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      scene_node *camera_node = the_camera->get_node();
      mat4t &camera_to_world = camera_node->access_nodeToParent();
      mouse_look_helper.update(camera_to_world);

      update_keys(player_node, camera_node);

      //char tmp[256];  
      //printf("player_node->get_position()=%s\n", player_node->get_position().toString(tmp, 256));
      camera_to_world.w() = (player_node->get_position() + vec3(0, 1.25f , 0) ).xyz1();

      // update matrices. assume 30 fps.
      app_scene->update(1.0f/30);

      player_node->set_angular_velocity(vec3(0, 0, 0));
      player_node->set_rotation(mat4t());
      player_node->clamp_linear_velocity(10);

      // draw the scene
      app_scene->render((float)vx / vy);
    }
  };
}
