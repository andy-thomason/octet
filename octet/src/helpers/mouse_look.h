////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// mouse ball for rotating cameras around points of interest.

namespace octet { namespace helpers {
  /// Class for rotating the camera with the mouse.
  class mouse_look {
    app *the_app;
    float sensitivity;
    bool invert_mouse;
  public:
    mouse_look() {
      sensitivity = 200;
      invert_mouse = false;
    }

    void init(app *the_app, float sensitivity, bool invert_mouse) {
      this->the_app = the_app;
      this->sensitivity = sensitivity;
      this->invert_mouse = invert_mouse;
      the_app->disable_cursor();
    }

    void update(mat4t &cameraToWorld) {
      cameraToWorld.x() = vec4(1, 0, 0, 0);
      cameraToWorld.y() = vec4(0, 1, 0, 0);
      cameraToWorld.z() = vec4(0, 0, 1, 0);

      int x = 0, y = 0;
      int vx = 0, vy = 0;
      the_app->get_mouse_pos(x, y);
      the_app->get_viewport_size(vx, vy);
      float cx = vx * 0.5f;
      float cy = vy * 0.5f;

      cameraToWorld.rotateY((cx - x) / vx * sensitivity);
      cameraToWorld.rotateX((invert_mouse ? -1 : 1) * (cy - y) / vy * sensitivity);
    }
  };
}}
