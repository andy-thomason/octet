////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// mouse ball for rotating cameras around points of interest.

namespace octet { namespace helpers {
  /// Class for picking objects using mouse clicks.
  class object_picker {
    app *the_app;
    dynarray<ref<mesh_instance> > objects;
  public:
    object_picker() {
    }

    void init(app *the_app) {
      this->the_app = the_app;
    }

    void update(visual_scene *the_scene) {
      bool is_mouse_down = the_app->is_key_down(key_lmb);
      if (is_mouse_down) {
        int mx = 0, my = 0;
        int vx = 0, vy = 0;
        the_app->get_mouse_pos(mx, my);
        the_app->get_viewport_size(vx, vy);
        float x = (float)( mx - vx/2 ) * 2.0f / vx;
        float y = (float)( vy/2 - my ) * 2.0f / vy;

        if (the_scene->get_num_camera_instances() == 0) {
          return;
        }
        camera_instance *cam = the_scene->get_camera_instance(0);

        ray the_ray = cam->get_ray(x, y);
        //the_scene->add_debug_line(the_ray.get_start(), the_ray.get_end());

        visual_scene::cast_result res;
        the_scene->cast_ray(res, the_ray);
        if (res.mi) {
          //printf("%s\n", res.depth.toString());
        }
      }
    }
  };
}}
