////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// mouse ball for rotating cameras around points of interest.

namespace octet { namespace helpers {
  /// Class for rotating the camera around the center of the world.
  class mouse_ball {
    app *the_app;
    float distance;
    bool was_mouse_down;
    int prev_x;
    int prev_y;
    float sensitivity;
  public:
    mouse_ball() {
      was_mouse_down = false;
      sensitivity = 200;
    }

    void init(app *the_app, float distance, float sensitivity) {
      this->the_app = the_app;
      this->distance = distance;
      this->sensitivity = sensitivity;
    }

    void update(mat4t &cameraToWorld) {
      bool is_mouse_down = the_app->is_key_down(key_lmb) && the_app->is_key_down(key_alt);
      if (is_mouse_down) {
        int x = 0, y = 0;
        int vx = 0, vy = 0;
        the_app->get_mouse_pos(x, y);
        the_app->get_viewport_size(vx, vy);
        if (was_mouse_down && vx && vy) {
          // spin the camera around the target
          float cx = vx * 0.5f;
          float cy = vy * 0.5f;
          float pfx = ( prev_x - cx ) / vx;
          float pfy = ( prev_y - cy ) / vy;
          float fx = ( x - cx ) / vx;
          float fy = ( y - cy ) / vy;
          vec4 dxy(pfy - fy, pfx - fx, 0, 0);
          float len2 = dxy.squared();
          if (len2 > 0) {
            vec4 norm = dxy.normalize();
            cameraToWorld.translate(0, 0, -distance);
            cameraToWorld.rotate(sqrtf(len2) * sensitivity, norm.x(), norm.y(), 0);
            cameraToWorld.translate(0, 0, distance);
          }

          // allow small camera rolls
          /*float r2 = fx * fx + fy * fy;
          if (r2 > 0.1f) {
            float prev_angle = atan2f(pfy, pfx) * (180.0f/3.141592653f);
            float angle = atan2f(fy, fx) * (180.0f/3.141592653f);
            if (fabsf(angle - prev_angle) < 30) {
              cameraToWorld.rotateZ(angle - prev_angle);
            }
          }*/
        }
        prev_x = x;
        prev_y = y;
      }
      was_mouse_down = is_mouse_down;
    }
  };
}}
