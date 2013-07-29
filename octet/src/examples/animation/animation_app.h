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
// and so we need to use octet:: on several classes.

class animation_app : public octet::app {
  typedef octet::mat4t mat4t;
  typedef octet::animation animation;
  typedef octet::scene_node scene_node;

  // named resources loaded from collada file
  octet::resources dict;

  // scene used to draw 3d objects
  octet::ref<octet::scene> app_scene;

  // shaders to draw triangles
  octet::bump_shader object_shader;
  octet::bump_shader skin_shader;

  // helper to rotate camera about scene
  octet::mouse_ball ball;

  // helper for drawing text
  octet::text_overlay overlay;

public:
  // this is called when we construct the class
  animation_app(int argc, char **argv) : app(argc, argv), ball() {
  }

  // this is called once OpenGL is initialized
  void app_init() {
    // set up the shaders
    object_shader.init(false);
    skin_shader.init(true);

    octet::collada_builder builder;
    const char *filename = 0;

    int selector = 5;
    switch (selector) {
      case 0: filename = "assets/duck_triangulate.dae"; break;
      case 1: filename = "assets/skinning/skin_unrot.dae"; break;
      case 2: filename = "assets/jenga.dae"; break;
      case 3: filename = "assets/duck_ambient.dae"; break;
      case 4: filename = "assets/Laurana50k.dae"; break;
      case 5: filename = "assets/Arteria3dElvenMaleFREE/ElevenMaleKnight_blender.dae"; break;
    }

    if (!builder.load_xml(filename)) {
      return;
    }

    builder.get_resources(dict);
    app_scene = dict.get_scene(builder.get_default_scene());

    assert(app_scene);

    app_scene->create_default_camera_and_lights();

    app_scene->play_all_anims(dict);

    if (app_scene->num_camera_instances() != 0) {
      octet::camera_instance *cam = app_scene->get_camera_instance(0);
      scene_node *node = cam->get_node();
      mat4t cameraToWorld = node->get_nodeToParent();
      ball.init(this, cameraToWorld.w().length(), 360.0f);
    }

    overlay.init();
  }

  // this is called to draw the world
  void draw_world(int x, int y, int w, int h) {
    // set a viewport - includes whole window area
    glViewport(x, y, w, h);

    // clear the background to black
    glClearColor(0.5f, 0.5f, 0.5f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // allow Z buffer depth testing (closer objects are always drawn in front of far ones)
    glEnable(GL_DEPTH_TEST);

    if (app_scene && app_scene->num_camera_instances()) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);

      octet::camera_instance *cam = app_scene->get_camera_instance(0);
      scene_node *node = cam->get_node();
      mat4t &cameraToWorld = node->access_nodeToParent();
      ball.update(cameraToWorld);

      // update matrices. assume 30 fps.
      app_scene->update(1.0f/30);

      app_scene->render(object_shader, skin_shader, *cam, (float)vx / vy);

      overlay.render(object_shader, skin_shader, vx, vy);
    }
  }
};
