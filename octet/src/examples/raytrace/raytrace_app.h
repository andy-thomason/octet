////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// cube example: How to draw basic 3D geometry
//
// Demonstrates:
//   Basic framework app
//   Shaders
//   Basic Matrices
//   Collada meshes
//   Transforming meshes
//   Raytracing using CPU
//

class raytrace_app : public app {
  // ray tracing renderer
  raytracer renderer;

  // size of image
  enum { image_size = 256 };

  // Matrices to transform points on our triangles to the world space
  // This allows us to move and rotate our triangles
  dynarray<mat4> modelToWorld;

  // meshes for our objects
  dynarray<mesh*> meshes;

  // materials for our objects
  dynarray<bump_material*> materials;

  // array of lights
  lighting lights;
  
  // Matrix to transform points in our camera space to the world.
  // This lets us move our camera
  mat4 cameraToWorld;

  // shader to draw the final scene
  texture_shader texture_shader_;

  // mesh for the duck
  mesh duck_mesh;

  // material for duck.
  bump_material duck_material;

  // texture for our frame buffer
  GLuint texture_handle_;

public:

  // this is called when we construct the class
  raytrace_app(int argc, char **argv) : app(argc, argv) {
  }

  // this is called once OpenGL is initialized
  void app_init() {
    renderer.init();

    // set up the shader
    texture_shader_.init();

    // set up the matrices with a camera 5 units from the origin
    cameraToWorld.loadIdentity();
    cameraToWorld.translate(0, 0, 5);

    // generate a blank texture
    glGenTextures(1, &texture_handle_);
    glBindTexture(GL_TEXTURE_2D, texture_handle_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_size, image_size, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);

    //texture_handle_ = resource_manager::get_texture_handle(GL_RGB, "assets/duckCM.gif");

    collada_builder builder;
    builder.load("assets/duck_triangulate.dae");
    duck_mesh.make_collada_mesh(builder, "LOD3spShape-lib");

    mat4 mat;
    mat.loadIdentity();
    modelToWorld.push_back(mat);
    meshes.push_back(&duck_mesh);
    materials.push_back(&duck_material);

    // the original duck is a bit too big, shrink it with a matrix
    mat4 shrink;
    shrink.loadIdentity();
    shrink.translate(0, -50, 0);
    shrink.scale(0.03f, 0.03f, 0.03f);
    duck_mesh.transform(mesh_state::attribute_pos, shrink);

    GLuint diffuse = resource_manager::get_texture_handle(GL_RGB, "assets/duckCM.gif");
    GLuint emission = resource_manager::get_texture_handle(GL_RGB, "#000000");
    GLuint specular = resource_manager::get_texture_handle(GL_RGB, "#ffffff");
    GLuint bump = resource_manager::get_texture_handle(GL_RGB, "#0000ff");

    duck_material.init(diffuse, diffuse, emission, specular, bump, 30.0f);

    lights.add_light(vec4(10, 10, 10, 1), vec4(0, 0, 1, 0).normalize(), vec4(0.3f, 0.3f, 0.3f, 1), vec4(1, 1, 1, 1), vec4(1, 1, 1, 1));
  }

  // this is called to draw the world
  void draw_world(int x, int y, int w, int h) {
    // set a viewport - includes whole window area
    glViewport(x, y, w, h);

    // clear the background to black
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ray trace the scene to the texture
    raytracer::context ctxt;
    ctxt.width = image_size;
    ctxt.height = image_size;
    ctxt.texture_handle = texture_handle_;
    ctxt.cameraToWorld = cameraToWorld;
    ctxt.near_plane = 0.125f;
    ctxt.far_plane = 128.125f;
    ctxt.near_plane_xmax = ctxt.near_plane;
    ctxt.near_plane_ymax = ctxt.near_plane;
    ctxt.lights = &lights;
    ctxt.modelToWorld = &modelToWorld[0];
    ctxt.meshes = &meshes[0];
    ctxt.materials = &materials[0];
    ctxt.num_objects = materials.size();
    renderer.ray_trace(ctxt);

    modelToWorld[0].rotateY(1);

    // set up opengl to draw textured triangles using sampler 0 (GL_TEXTURE0)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_handle_);

    // build a projection matrix: model -> world -> camera -> projection
    // this matrix is used to map the square to fill the screen
    // in this case we use an "ortho" projection.
    mat4 modelToProjection;
    modelToProjection.loadIdentity();
    modelToProjection.translate(0, 0, 1);
    modelToProjection.ortho(-1, 1, -1, 1, 0.125f, 128.125);
    texture_shader_.render(modelToProjection, 0);

    // this is an array of the positions of the corners of the texture in 3D
    static const float vertices[] = {
      -1, -1, 0,
       1, -1, 0,
       1,  1, 0,
      -1,  1, 0,
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
