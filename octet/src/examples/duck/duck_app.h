////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// duck example: How to load a mesh from a collada file
//
// Level: 1
//
// Demonstrates:
//   Basic framework app
//   Shaders
//   Basic Matrices
//   Collada meshes
//   Phong shader
//   Transforming meshes
//

class duck_app : public app {
  // Matrix to transform points on our triangle to the world space
  // This allows us to move and rotate our triangle
  mat4 modelToWorld;

  // Matrix to transform points in our camera space to the world.
  // This lets us move our camera
  mat4 cameraToWorld;

  // shader to draw a shaded, textured triangle
  phong_shader phong_shader_;

  // mesh for the duck
  mesh duck_mesh;

  // mesh to show normals
  //mesh duck_normals;

  // handles for the textures
  GLuint diffuse;
  GLuint ambient;
  GLuint emission;
  GLuint specular;
public:

  // this is called when we construct the class
  duck_app(int argc, char **argv) : app(argc, argv) {
  }

  // this is called once OpenGL is initialized
  void app_init() {
    // set up the shader
    phong_shader_.init();

    // set up the matrices with a camera 5 units from the origin
    modelToWorld.loadIdentity();
    cameraToWorld.loadIdentity();
    cameraToWorld.translate(0, 0, 5);

    collada_builder builder;
    builder.load("assets/duck_triangulate.dae");
    duck_mesh.make_collada_mesh(builder, "LOD3spShape-lib");

    // the original duck is a bit too big, shrink it with a matrix
    mat4 shrink;
    shrink.loadIdentity();
    shrink.translate(0, -50, 0);
    shrink.scale(0.03f, 0.03f, 0.03f);
    duck_mesh.transform(mesh_state::attribute_pos, shrink);

    //duck_normals.make_normal_visualizer(duck_mesh, 0.1f);

    diffuse = ambient = resource_manager::get_texture_handle(GL_RGB, "assets/duckCM.gif");
    //diffuse = ambient = resource_manager::get_texture_handle(GL_RGB, "!bump");
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
    glFrontFace(GL_CCW);

    // build a projection matrix: model -> world -> camera -> projection
    // the projection space is the cube -1 <= x/w, y/w, z/w <= 1
    mat4 modelToCamera;
    mat4 worldToCamera;
    mat4 modelToProjection = mat4::build_camera_matrices(modelToCamera, worldToCamera, modelToWorld, cameraToWorld);
    //printf("m2c=%s\n", modelToCamera.toString());
    //printf("w2c=%s\n", worldToCamera.toString());
    float shininess = 30.0f;
    vec4 light_dir = vec4(1, 1, 1, 0).normalize() * worldToCamera;
    //printf("ld=%s\n", light_dir.toString());
    vec4 light_ambient = vec4(0.3f, 0.3f, 0.3f, 1);
    vec4 light_diffuse = vec4(1, 1, 1, 1);
    vec4 light_specular = vec4(1, 1, 1, 1);
    phong_shader_.render(modelToProjection, modelToCamera, light_dir, shininess, light_ambient, light_diffuse, light_specular);

    // spin the model by rotating about X, Y and Z
    modelToWorld.rotateZ(1);
    modelToWorld.rotateX(2);
    modelToWorld.rotateY(3);

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

    duck_mesh.render();
    //duck_normals.render();
  }
};
