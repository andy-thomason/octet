////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2013
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

class animation_app : public app {
  scene app_scene;

  // shader to draw a shaded, textured triangle
  bump_shader object_shader;
  bump_shader skin_shader;

  int time;
public:

  // this is called when we construct the class
  animation_app(int argc, char **argv) : app(argc, argv) {
  }

  // this is called once OpenGL is initialized
  void app_init() {
    // set up the shaders
    object_shader.init(false);
    skin_shader.init(true);

    collada_builder builder;
    builder.load("assets/skinning/skin_unrot.dae");

    const char *def_scene = builder.get_default_scene();
    app_scene.make_collada_scene(builder, def_scene);

    app_scene.play_all_animations();

    time = 0;
  }

  // this is called to draw the world
  void draw_world(int x, int y, int w, int h) {
    // set a viewport - includes whole window area
    glViewport(x, y, w, h);

    // clear the background to black
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // allow Z buffer depth testing (closer objects are always drawn in front of far ones)
    glEnable(GL_DEPTH_TEST);

    if (app_scene.num_cameras() == 0) {
      return;
    }

    mat4t cameraToWorld;
    camera *cam = app_scene.get_camera(0);

    app_scene.update(time);

    app_scene.render(object_shader, skin_shader, *cam);

    //app_scene.node_to_parent(5).rotateX(1);
  }
};
