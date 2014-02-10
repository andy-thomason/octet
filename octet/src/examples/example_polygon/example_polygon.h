////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {
  /// Scene demonstrating operations on polygon objects.
  class example_polygon : public app {
    // scene for drawing box
    ref<visual_scene> app_scene;

    ref<mesh> geom;

    int max_level;
    random r;

    polygon tmp;
    polygon left[16];
    polygon road[16];
    polygon right[16];

    struct grass_uvgen {
      static vec2 uv(vec3_in pos) { return vec2(5.0f/32, 31.0f/32); }
      static vec3 normal(vec3_in pos) { return vec3(0, 0, 1); }
      static vec3 pos(vec3_in pos) { return pos; }
    };

    struct pavement_uvgen {
      static vec2 uv(vec3_in pos) { return vec2(3.0f/32, 31.0f/32); }
      static vec3 normal(vec3_in pos) { return vec3(0, 0, 1); }
      static vec3 pos(vec3_in pos) { return pos; }
    };

    struct road_uvgen {
      static vec2 uv(vec3_in pos) { return vec2(1.0f/32, 31.0f/32); }
      static vec3 normal(vec3_in pos) { return vec3(0, 0, 1); }
      static vec3 pos(vec3_in pos) { return pos - vec3(0, 0.2f, 0); }
    };

    void add_polygon(const polygon &poly, int level) {
      if (level == max_level) {
        poly.grow(tmp, vec3(0, 1, 0), 0.2f);
        geom->add_polygon<pavement_uvgen>(tmp);
      } else {
        vec3 center = poly.calc_aabb().get_center();
        uint32_t angle = level * 0x40000000;
        angle += (r.get0xffff() << 13) - 0x10000000;
        vec3 normal(fast_cos6(angle), 0.0f, fast_sin6(angle));
        float offset = -dot(normal, center);
        float road_half_width = 2.0f;
        half_space hs1(normal, offset - road_half_width);
        poly.clip(left[level], hs1);
        half_space hs2(-normal, -offset - road_half_width);
        poly.clip(right[level], hs2);

        hs1.flip();
        hs2.flip();

        poly.clip(tmp, hs1);
        tmp.clip(road[level], hs2);

        geom->add_polygon<road_uvgen>(road[level]);

        add_polygon(left[level], level + 1);
        add_polygon(right[level], level + 1);
      }
    }
  public:
    /// this is called when we construct the class before everything is initialised.
    example_polygon(int argc, char **argv) : app(argc, argv) {
      max_level = 6;
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();
      scene_node *cam_node = app_scene->get_camera_instance(0)->get_node();
      if (1) {
        cam_node->translate(vec3(0, 2, 0));
      } else {
        cam_node->translate(vec3(0, 2000, 0));
        cam_node->rotate(-90, vec3(1, 0, 0));
      }
      app_scene->get_camera_instance(0)->set_far_plane(20000);
      app_scene->get_camera_instance(0)->set_near_plane(1);

      material *mat = new material(new image("assets/roads.gif"));

      geom = new mesh(0x10000, 0x10000);
      //geom->set_mode(GL_LINES);

      scene_node *node = new scene_node();
      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, geom, mat));

      polygon top_level;
      top_level.add_vertex(vec3(-1000, 0, -1000));
      top_level.add_vertex(vec3( 1000, 0, -1000));
      top_level.add_vertex(vec3( 1000, 0,  1000));
      top_level.add_vertex(vec3(-1000, 0,  1000));

      r.set_seed(0x12345678);
      add_polygon(top_level, 0);
      //geom->dump(log("poly\n"));
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      {
        scene_node *cam_node = app_scene->get_camera_instance(0)->get_node();
        if (app::is_key_down('W')) {
          cam_node->translate(vec3(0, 0, -1));
        }
        if (app::is_key_down('S')) {
          cam_node->translate(vec3(0, 0,  1));
        }
        if (app::is_key_down(key_left)) {
          cam_node->rotate( 5, vec3(0, 1, 0));
        }
        if (app::is_key_down(key_right)) {
          cam_node->rotate(-5, vec3(0, 1, 0));
        }
      }

      // update matrices. assume 30 fps.
      app_scene->update(1.0f/30);

      // draw the scene
      app_scene->render((float)vx / vy);

      // tumble the box  (there is only one mesh instance)
      scene_node *node = app_scene->get_mesh_instance(0)->get_node();
      //node->rotate(1, vec3(1, 0, 0));
      //node->rotate(1, vec3(0, 1, 0));
    }
  };
}
