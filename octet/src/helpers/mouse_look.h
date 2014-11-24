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
    int mouse_center_x;
    int mouse_center_y;
  public:
    mouse_look() {
      sensitivity = 200.0f / 360.0f;
      invert_mouse = false;
      mouse_center_x = 0;
      mouse_center_y = 0;
    }

    void init(app *the_app, float sensitivity, bool invert_mouse) {
      this->the_app = the_app;
      this->sensitivity = sensitivity;
      this->invert_mouse = invert_mouse;
      the_app->disable_cursor();
    }

    /// set this to reset the camera look (for example at the start of a level).
    void set_mouse_center(int x, int y) {
      mouse_center_x = x;
      mouse_center_y = y;
    }

    /// set this to invert the mouse movement
    void set_invert_mouse(bool value) {
      invert_mouse = value;
    }

    /// set the sensitivity of the mouse look
    /// this is in degrees per mouse pixel
    void set_sensitivity(float value) {
      sensitivity = value;
    }

    /// called every frame to update the camera
    void update(mat4t &cameraToWorld) {
      cameraToWorld.x() = vec4(1, 0, 0, 0);
      cameraToWorld.y() = vec4(0, 1, 0, 0);
      cameraToWorld.z() = vec4(0, 0, 1, 0);

      int rx = 0, ry = 0;
      the_app->get_absolute_mouse_movement(rx, ry);
      float angle_x = (float)(mouse_center_x - rx) * sensitivity;
      float angle_y = (float)(mouse_center_y - ry) * sensitivity;
      angle_y = std::max(-90.0f, std::min(angle_y, 90.0f));
      cameraToWorld.rotateY(angle_x);
      cameraToWorld.rotateX(invert_mouse ? -angle_y : angle_y);
    }
  };
}}
