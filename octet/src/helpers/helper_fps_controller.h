////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// mouse ball for rotating cameras around points of interest.

namespace octet { namespace helpers {
  /// Class for rotating the camera with the mouse.
  class helper_fps_controller {
    app *the_app;
  public:
    helper_fps_controller() {
    }

    void init(app *the_app) {
      this->the_app = the_app;
    }

    /// called every frame to update the player physics
    void update(scene_node *player_node, scene_node *camera_node) {
      #ifdef OCTET_BULLET
        player_node->set_damping(0, 0);
        player_node->set_angular_factor(0);
        //player_node->set_angular_velocity(vec3(0, 0, 0));
        //player_node->set_rotation(mat4t());
        player_node->clamp_linear_velocity(10);

        float friction = 0.0f;
        if (the_app->is_key_down('A')) {
          player_node->activate();
          player_node->apply_central_force(camera_node->get_x() * (-1000.0f));
        } else if (the_app->is_key_down('D')) {
          player_node->activate();
          player_node->apply_central_force(camera_node->get_x() * (+1000.0f));
        } else if (the_app->is_key_down('W')) {
          player_node->activate();
          player_node->apply_central_force(camera_node->get_z() * (-1000.0f));
        } else if (the_app->is_key_down('S')) {
          player_node->activate();
          player_node->apply_central_force(camera_node->get_z() * (+1000.0f));
        } else {
          friction = 1.0f;
        }
        if (the_app->is_key_going_down(' ')) {
          player_node->apply_central_force(camera_node->get_y() * (+10000.0f));
        }
        player_node->set_friction(friction);

        mat4t &camera_to_world = camera_node->access_nodeToParent();
        camera_to_world.w() = (player_node->get_position() + vec3(0, 1.25f , 0) ).xyz1();
      #endif
    }
  };
}}
