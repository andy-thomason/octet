////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// The game of ping with a little pong.
//
// Demonstrates:
//   Basic framework app
//   Shaders
//   Basic Matrices
//
//

namespace octet {
  class box {
    // where is our box (overkill for a ping game!)
    mat4t modelToWorld;

    // half the width of the box
    float halfWidth;

    // half the height of the box
    float halfHeight;

    // what color is our box
    vec4 color;
  public:
    box() {
    }

    void init(const vec4 &_color, float x, float y, float w, float h) {
      modelToWorld.loadIdentity();
      modelToWorld.translate(x, y, 0);
      halfWidth = w * 0.5f;
      halfHeight = h * 0.5f;
      color = _color;
    }

    void render(color_shader &shader, mat4t &cameraToWorld) {
      // build a projection matrix: model -> world -> camera -> projection
      // the projection space is the cube -1 <= x/w, y/w, z/w <= 1
      mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

      // set up the uniforms for the shader
      shader.render(modelToProjection, color);

      // this is an array of the positions of the corners of the box in 3D
      // a straight "float" here means this array is being generated here at runtime.
      float vertices[] = {
        -halfWidth, -halfHeight, 0,
         halfWidth, -halfHeight, 0,
         halfWidth,  halfHeight, 0,
        -halfWidth,  halfHeight, 0,
      };

      // attribute_pos (=0) is position of each corner
      // each corner has 3 floats (x, y, z)
      // there is no gap between the 3 floats and hence the stride is 3*sizeof(float)
      glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)vertices );
      glEnableVertexAttribArray(attribute_pos);
    
      // finally, draw the box (4 vertices)
      glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    // move the object
    void translate(float x, float y) {
      modelToWorld.translate(x, y, 0);
    }

    // position the object relative to another.
    void set_relative(box &rhs, float x, float y) {
      modelToWorld = rhs.modelToWorld;
      modelToWorld.translate(x, y, 0);
    }

    // return true if this box collides with another.
    // note the "const"s which say we do not modify either box
    bool collides_with(const box &rhs) const {
      float dx = rhs.modelToWorld[3][0] - modelToWorld[3][0];
      float dy = rhs.modelToWorld[3][1] - modelToWorld[3][1];

      // both distances have to be under the sum of the halfwidths
      // for a collision
      return
        (fabsf(dx) < halfWidth + rhs.halfWidth) &&
        (fabsf(dy) < halfHeight + rhs.halfHeight)
      ;
    }
  };

  class ping_app : public octet::app {
    // Matrix to transform points in our camera space to the world.
    // This lets us move our camera
    mat4t cameraToWorld;

    // shader to draw a solid color
    color_shader color_shader_;

    // what state is the game in?
    enum state_t {
      state_serving_left,
      state_serving_right,
      state_playing,
      state_game_over
    };
    state_t state;

    // counters for scores
    int scores[2];

    // game objects

    // court components
    box court[4];

    // bats
    box bat[2];

    // the ball
    box ball;

    // velocity of the ball
    float ball_velocity_x;
    float ball_velocity_y;

    // move the objects before drawing
    void simulate() {
      // W and S move the left bat
      if (is_key_down('W')) {
        bat[0].translate(0, +0.1f);
      } else if (is_key_down('S')) {
        bat[0].translate(0, -0.1f);
      }

      // up and down arrow move the right bat
      if (is_key_down(key_up)) {
        bat[1].translate(0, +0.1f);
      } else if (is_key_down(key_down)) {
        bat[1].translate(0, -0.1f);
      }

      if (state == state_serving_left) {
        // if we are serving, glue the ball to the left bat
        ball.set_relative(bat[0], 0.3f, 0);
        if (is_key_down(key_space)) {
          // space serves, changing the state
          state = state_playing;
          ball_velocity_x = 0.1f;
          ball_velocity_y = 0.1f;
        }
      } else if (state == state_serving_right) {
        // if we are serving, glue the ball to the right bat
        ball.set_relative(bat[1], -0.3f, 0);
        if (is_key_down(key_space)) {
          // space serves, changing the state
          state = state_playing;
          ball_velocity_x = -0.1f;
          ball_velocity_y = 0.1f;
        }
      } else if (state == state_playing) {
        // if we are playing, move the ball
        ball.translate(ball_velocity_x, ball_velocity_y);

        // check collision with the bats
        if (ball_velocity_x > 0 && ball.collides_with(bat[1])) {
          // to avoid internal bounces, only check the bats
          ball_velocity_x = -ball_velocity_x;
        } else if (ball_velocity_x < 0 && ball.collides_with(bat[0])) {
          ball_velocity_x = -ball_velocity_x;
        }

        // check collision with the court top and bottom
        if (ball_velocity_y > 0 && ball.collides_with(court[1])) {
          ball_velocity_y = -ball_velocity_y;
        } else if (ball_velocity_y < 0 && ball.collides_with(court[0])) {
          ball_velocity_y = -ball_velocity_y;
        }

        // check collision with the court end zones
        if (ball.collides_with(court[2])) {
          scores[0]++;
          state = scores[0] >= 10 ? state_game_over : state_serving_left;
        } else if (ball.collides_with(court[3])) {
          scores[1]++;
          state = scores[1] >= 10 ? state_game_over : state_serving_right;
        }
      }
    }

  public:

    // this is called when we construct the class
    ping_app(int argc, char **argv) : app(argc, argv) {
    }

    // this is called once OpenGL is initialized
    void app_init() {
      color_shader_.init();
      cameraToWorld.loadIdentity();
      cameraToWorld.translate(0, 0, 5);

      ball.init(vec4(1, 0, 0, 1), 0, 0, 0.25f, 0.25f);
      bat[0].init(vec4(1, 1, 0, 1), -4.7f, 0, 0.2f, 1.0f);
      bat[1].init(vec4(0, 0, 1, 1),  4.7f, 0, 0.2f, 1.0f);
      court[0].init(vec4(1, 1, 1, 1), 0, -4, 10, 0.1f);
      court[1].init(vec4(1, 1, 1, 1), 0,  4, 10, 0.1f);
      court[2].init(vec4(1, 1, 1, 1), -5, 0, 0.2f, 8);
      court[3].init(vec4(1, 1, 1, 1), 5,  0, 0.2f, 8);

      state = state_serving_left;
      scores[0] = scores[1] = 0;
    }

    // this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      simulate();

      // set a viewport - includes whole window area
      glViewport(x, y, w, h);

      // clear the background to black
      glClearColor(0, 0, 0, 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // allow Z buffer depth testing (closer objects are always drawn in front of far ones)
      glEnable(GL_DEPTH_TEST);

      // draw the ball
      ball.render(color_shader_, cameraToWorld);

      // draw the bats
      for (int i = 0; i != 2; ++i) {
        bat[i].render(color_shader_, cameraToWorld);
      }

      // draw the court
      for (int i = 0; i != 4; ++i) {
        court[i].render(color_shader_, cameraToWorld);
      }
    }
  };
}
