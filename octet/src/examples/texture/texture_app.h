////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// texture example: How to draw triangles with textures
//
// Level: 0
//
// Demonstrates:
//   Basic framework app
//   Shaders
//   Basic Matrices
//   Textures
//

class texture_app : public app {
  mat4t modelToWorld;              // this matrix converts from model space to world space
  mat4t cameraToWorld;             // this matrix converts from camera space to world space
  texture_shader texture_shader_; // this builds a shader to use with textures
  GLuint texture_handle_;         // this is an OpenGL texture handle which is used to draw the image.

public:

  // this is called when we construct the class
  texture_app(int argc, char **argv) : app(argc, argv) {
  }

  // this is called once OpenGL is initialized
  void app_init() {
    // set up the shader
    texture_shader_.init();

    // set up the matrices with a camera 5 units from the origin
    modelToWorld.loadIdentity();
    cameraToWorld.loadIdentity();
    cameraToWorld.translate(0, 0, 5);

    // use helper function to generate an OpenGL texture
    texture_handle_ = resources::get_texture_handle(GL_RGB, "!bricks");
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

    // build a projection matrix: model -> world -> camera -> projection
    // the projection space is the cube -1 <= x/w, y/w, z/w <= 1
    mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

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
    };

    // attribute_pos (=0) is position of each corner
    // each corner has 3 floats (x, y, z)
    // there is no gap between the 3 floats and hence the stride is 3*sizeof(float)
    glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)vertices );
    glEnableVertexAttribArray(attribute_pos);
    
    // this is an array of the positions of the corners of the texture in 2D
    static const float uvs[] = {
      -4, -4,
       4, -4,
       4,  4,
    };

    // attribute_uv is position in the texture of each corner
    // each corner (vertex) has 2 floats (x, y)
    // there is no gap between the 2 floats and hence the stride is 2*sizeof(float)
    glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)uvs );
    glEnableVertexAttribArray(attribute_uv);
    
    // finally, draw the texture (3 vertices)
    glDrawArrays(GL_TRIANGLES, 0, 3);
  }
};
