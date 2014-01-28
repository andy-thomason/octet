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

    dynarray<ref<polygon> > polygons;
    int max_level;
    random r;

    struct uvgen {
      static vec2 uv(vec3_in pos) { return vec2(0, 0); }
      static vec3 normal(vec3_in pos) { return vec3(0, 0, 1); }
      static vec3 pos(vec3_in pos) { return pos; }
    };

    void add_polygon(const polygon *poly, int level) {
      if (level == max_level) {
        geom->add_polygon<uvgen>(*poly);
      } else {
        polygon *left = new polygon();
        polygon *road = new polygon();
        polygon *right = new polygon();
        aabb bb = poly->get_aabb();
        uint32_t angle = level * 0x40000000; //r.get0xffff() << 16;
        angle += (r.get0xffff() << 13) - 0x10000000;
        vec3 normal(fast_cos6(angle), 0.0f, fast_sin6(angle));
        float offset = -dot(normal, bb.get_center());
        float road_half_width = 2.0f;
        half_space hs1(normal, offset - road_half_width);
        poly->clip(*left, hs1);
        half_space hs2(-normal, -offset - road_half_width);
        poly->clip(*right, hs2);

        /*char buf[512];
        log("%s l=%d r=%d\n", bb.toString(buf, sizeof(buf)), left->get_num_vertices(), right->get_num_vertices());
        log("hs1 %s\n", hs1.toString(buf, sizeof(buf)));
        log("hs2 %s\n", hs2.toString(buf, sizeof(buf)));
        for (unsigned i = 0; i != poly->get_num_vertices(); ++i) {
          vec3 pos = poly->get_vertex(i);
          bool in1 = hs1.intersects(pos);
          bool in2 = hs2.intersects(pos);
          log("%s %d %d\n", pos.toString(buf, sizeof(buf)), in1, in2);
        }*/

        //geom->add_polygon<uvgen>(*left);
        //geom->add_polygon<uvgen>(*right);

        add_polygon(left, level + 1);
        add_polygon(right, level + 1);
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

      material *red = new material(vec4(1, 0, 0, 1));

      geom = new mesh(0x10000, 0x10000);
      geom->set_mode(GL_LINES);

      scene_node *node = new scene_node();
      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, geom, red));

      polygon *top_level = new polygon();
      top_level->add_vertex(vec3(-1000, 0, -1000));
      top_level->add_vertex(vec3( 1000, 0, -1000));
      top_level->add_vertex(vec3( 1000, 0,  1000));
      top_level->add_vertex(vec3(-1000, 0,  1000));

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
