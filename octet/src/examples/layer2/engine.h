////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// animation example: Drawing an jointed figure with animation
//
// Level: 2
//
// Demonstrates:
//   Collada meshes
//   Collada nodes
//   Collada animation
//
// note this app is not in the octet namespace as it is not part of octet
// and so we need to use  on several classes.

namespace octet {
  class engine : public app {
    typedef mat4t mat4t;
    typedef vec4 vec4;
    typedef animation animation;
    typedef scene_node scene_node;

    // named resources loaded from collada file
    resources dict;

    // shaders to draw triangles
    bump_shader object_shader;
    bump_shader skin_shader;

    // helper to rotate camera about scene
    mouse_ball ball;

    // helper for drawing text
    text_overlay overlay;

    // helper for debugging by web browser
    http_server server;

    // helper for picking objects on the screen
    object_picker picker;

    // test c/c++ parser
    //cpp_parser parser;

    void load_file(const char *filename) {
      FILE *file = fopen(app_utils::get_path(filename), "rb");
      char buf[8];
      scene *app_scene = 0;
      if (file && fread(buf, 1, sizeof(buf), file) && !memcmp(buf, "octet", 5)) {
        fseek(file, 0, SEEK_SET);
        binary_reader r(file);
        dict.visit(r);
        fclose(file);
        app_scene = dict.get_active_scene();
      } else {
        if (file) fclose(file);
        collada_builder builder;
        if (!builder.load_xml(filename)) {
          printf("\nERROR: could not open %s\nThis is likely a problem with paths.", filename);
          return;
        }

        builder.get_resources(dict);
        app_scene = dict.get_scene(builder.get_default_scene());

        assert(app_scene);

        app_scene->create_default_camera_and_lights();

        dict.set_active_scene(app_scene);
      }

      app_scene->play_all_anims(dict);

      // add modifiers here to test them.
      for (unsigned i = 0; i != app_scene->get_num_mesh_instances(); ++i) {
        mesh_instance *mi = app_scene->get_mesh_instance(i);
        //mi->set_mesh(new wireframe(new displacement_map(mi->get_mesh())));
        //mi->set_mesh(new smooth(mi->get_mesh()));
        //mi->set_mesh(mi->get_mesh());
        mi->set_mesh(new indexer(mi->get_mesh()));
      }

      if (app_scene->get_num_camera_instances() != 0) {
        camera_instance *cam = app_scene->get_camera_instance(0);
        scene_node *node = cam->get_node();
        mat4t cameraToWorld = node->get_nodeToParent();
        ball.init(this, cameraToWorld.w().length(), 360.0f);
      }
    }
  public:
    // this is called when we construct the class
    engine(int argc, char **argv) : app(argc, argv), ball() {

      // test the c++ parser
      //parser.parse("int x = 1;");
    }

    // this is called once OpenGL is initialized
    void app_init() {
      // set up the shaders
      object_shader.init(false);
      skin_shader.init(true);

      const char *filename = 0;

      /*video_capture *vc = get_video_capture();
      int vco = vc->open();
      if (vco >= 0) printf("vc w=%d h=%d\n", vc->width(), vc->height());
      dynarray<uint8_t> pixels;
      pixels.resize(vc->width() * vc->height() * vc->bits_per_pixel() / 8);
      vc->read(&pixels[0], pixels.size());
      vc->read(&pixels[0], pixels.size());
      vc->read(&pixels[0], pixels.size());
      vc->read(&pixels[0], pixels.size());
      vc->close();*/

      // note that you can also drag and drop files onto an active octet window.
      int selector = 0;
      switch (selector) {
        case 0: filename = "assets/duck_triangulate.dae"; break;
        case 1: filename = "assets/skinning/skin_unrot.dae"; break;
        case 2: filename = "assets/jenga.dae"; break;
        case 3: filename = "assets/duck_ambient.dae"; break;
        case 4: filename = "assets/Laurana50k.dae"; break;
        case 5: filename = "external/Arteria3d/ElvenMale/ElevenMaleKnight_blender.dae"; break;
        case 6: filename = "external/Arteria3d/arteria3d_tropicalpack/flowers/flower%202.dae"; break; 
        case 7: filename = "assets/plane.dae"; break;
      }

      load_file(filename);

      overlay.init();
      server.init(&dict);
      picker.init(this);
    }

    // this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx, vy;
      get_viewport_size(vx, vy);
      // set a viewport - includes whole window area
      glViewport(0, 0, vx, vy);

      // clear the background to black
      glClearColor(0.5f, 0.5f, 0.5f, 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // allow Z buffer depth testing (closer objects are always drawn in front of far ones)
      glEnable(GL_DEPTH_TEST);

      GLint param;
      glGetIntegerv(GL_SAMPLE_BUFFERS, &param);
      if (param == 0) {
        // if multisampling is disabled, we can't use GL_SAMPLE_COVERAGE (which I think is mean)
        // Instead, allow alpha blend (transparency when alpha channel is 0)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // this is a rather brutal alpha test that cuts off anything with a small alpha.
        #ifndef SN_TARGET_PSP2
          //glEnable(GL_ALPHA_TEST);
          //glAlphaFunc(GL_GREATER, 0.9f);
        #endif
      } else {
        // if multisampling is enabled, use GL_SAMPLE_COVERAGE instead
        glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
        glEnable(GL_SAMPLE_COVERAGE);
      }

      // poll web server
      server.update();

      // ctrl-s: save file
      if (is_key_down('S') && is_key_down(key_ctrl)) {
        FILE *file = fopen("c:/tmp/save.oct", "wb");
        binary_writer b(file);
        dict.visit(b);
        fclose(file);

        /*resources loaded;
        file = fopen("save.oct", "rb");
        binary_reader r(file);
        loaded.visit(r);
        fclose(file);*/

      }

      // drag and drop file loading
      dynarray<string> &queue = access_load_queue();
      if (queue.size()) {
        // replace scene
        dict.reset();
        string url;
        url.urlencode(queue[0]);
        load_file(url);
        queue.resize(0);
      }

      scene *app_scene = dict.get_active_scene();
      if (app_scene && app_scene->get_num_camera_instances()) {
        int vx = 0, vy = 0;
        get_viewport_size(vx, vy);

        camera_instance *cam = app_scene->get_camera_instance(0);
        scene_node *node = cam->get_node();
        mat4t &cameraToWorld = node->access_nodeToParent();
        ball.update(cameraToWorld);
        picker.update(app_scene);

        // update matrices. assume 30 fps.
        app_scene->update(1.0f/30);

        app_scene->render(object_shader, skin_shader, *cam, (float)vx / vy);

        overlay.render(object_shader, skin_shader, vx, vy, get_frame_number());
      }
    }
  };
}
