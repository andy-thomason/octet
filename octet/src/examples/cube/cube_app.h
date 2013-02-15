////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// cube example: How to draw basic 3D geometry
//
// Demonstrates:
//   Basic framework app
//   Shaders
//   Basic Matrices
//   Stock meshes
//   Phong shader
//

class cube_app : public app {
  // Matrix to transform points on our triangle to the world space
  // This allows us to move and rotate our triangle
  mat4 modelToWorld;

  // Matrix to transform points in our camera space to the world.
  // This lets us move our camera
  mat4 cameraToWorld;

  // shader to draw a shaded, textured triangle
  phong_shader phong_shader_;

  // handle for the texture
  mesh cube_mesh;

  GLuint diffuse;
  GLuint ambient;
  GLuint emission;
  GLuint specular;
public:

  // this is called when we construct the class
  cube_app(int argc, char **argv) : app(argc, argv) {
  }

  // this is called once OpenGL is initialized
  void app_init() {
    // set up the shader
    phong_shader_.init();

    // set up the matrices with a camera 5 units from the origin
    modelToWorld.loadIdentity();
    modelToWorld.rotateX90();
    cameraToWorld.loadIdentity();
    cameraToWorld.translate(0, 0, 5);

    cube_mesh.make_cube();
    //cube_mesh.make_cone();
    //cube_mesh.make_sphere();

    diffuse = ambient = resource_manager::get_texture_handle(GL_RGB, "!bricks");
    emission = resource_manager::get_texture_handle(GL_RGB, "#000000");
    specular = resource_manager::get_texture_handle(GL_RGB, "#ffffff");
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

    // improve draw speed by culling back faces - and avoid flickering edges
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    // build a projection matrix: model -> world -> camera -> projection
    // the projection space is the cube -1 <= x/w, y/w, z/w <= 1
    mat4 modelToCamera;
    mat4 worldToCamera;
    mat4 modelToProjection = mat4::build_camera_matrices(modelToCamera, worldToCamera, modelToWorld, cameraToWorld);
    float shininess = 30.0f;
    vec4 light_dir = vec4(1, 1, 1, 0).normalize() * worldToCamera;
    vec4 light_ambient = vec4(0.3f, 0.3f, 0.3f, 1);
    vec4 light_diffuse = vec4(1, 1, 1, 1);
    vec4 light_specular = vec4(1, 1, 1, 1);
    phong_shader_.render(modelToProjection, modelToCamera, light_dir, shininess, light_ambient, light_diffuse, light_specular);

    // spin the cube by rotating about X, Y and Z
    modelToWorld.rotateZ(1);
    modelToWorld.rotateX(2);
    modelToWorld.rotateY(1.5f);

    // set textures 0, 1, 2, 3 to their respective values
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, ambient);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, emission);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, specular);
    glActiveTexture(GL_TEXTURE0);

    cube_mesh.render();
  }
};
