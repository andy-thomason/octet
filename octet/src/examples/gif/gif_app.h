////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// texture example: How to draw triangles with textures
//
// Demonstrates:
//   Basic framework app
//   Shaders
//   Basic Matrices
//   Texture loaded from GIF file
//

class gif_app : public app {
  // Matrix to transform points on our triangle to the world space
  // This allows us to move and rotate our triangle
  mat4 modelToWorld;

  // Matrix to transform points in our camera space to the world.
  // This lets us move our camera
  mat4 cameraToWorld;

  // shader to draw a textured triangle
  texture_shader texture_shader_;

  // handle for the texture
  GLuint texture_handle_;

public:

  // this is called when we construct the class
  gif_app(int argc, char **argv) : app(argc, argv) {
  }

  // this is called once OpenGL is initialized
  void app_init() {
    // set up the shader
    texture_shader_.init();

    // set up the matrices with a camera 5 units from the origin
    modelToWorld.loadIdentity();
    cameraToWorld.loadIdentity();
    cameraToWorld.translate(0, 0, 3);

    // use helper function to generate an OpenGL texture
    texture_handle_ = resource_manager::get_texture_handle(GL_RGBA, "assets/stars.gif");
  }

  // this is called to draw the world
  void draw_world(int x, int y, int w, int h) {
    // set a viewport - includes whole window area
    glViewport(x, y, w, h);

    // clear the background to gray
    glClearColor(0.5f, 0.5f, 0.5f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // allow Z buffer depth testing (closer objects are always drawn in front of far ones)
    glEnable(GL_DEPTH_TEST);

    // allow alpha blend (transparency when alpha channel is 0)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // build a projection matrix: model -> world -> camera -> projection
    // the projection space is the cube -1 <= x/w, y/w, z/w <= 1
    mat4 modelToProjection = mat4::build_projection_matrix(modelToWorld, cameraToWorld);

    // spin the triangle by rotating about Z (the view direction)
    modelToWorld.rotateZ(1);

    // set up opengl to draw textured triangles using sampler 0 (GL_TEXTURE0)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_handle_);
    texture_shader_.render(modelToProjection, 0);

    // this is an array of the positions of the corners of the texture in 3D
    static const float vertices[] = {
      -2, -2, 0,
       2, -2, 0,
       2,  2, 0,
      -2,  2, 0,
    };

    // attribute_pos (=0) is position of each corner
    // each corner has 3 floats (x, y, z)
    // there is no gap between the 3 floats and hence the stride is 3*sizeof(float)
    glVertexAttribPointer(mesh_state::attribute_pos, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)vertices );
    glEnableVertexAttribArray(mesh_state::attribute_pos);
    
    // this is an array of the positions of the corners of the texture in 2D
    static const float uvs[] = {
      0, 0,
      1, 0,
      1, 1,
      0, 1,
    };

    // attribute_uv is position in the texture of each corner
    // each corner (vertex) has 2 floats (x, y)
    // there is no gap between the 2 floats and hence the stride is 2*sizeof(float)
    glVertexAttribPointer(mesh_state::attribute_uv, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)uvs );
    glEnableVertexAttribArray(mesh_state::attribute_uv);
    
    // finally, draw the texture (3 vertices)
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  }
};
