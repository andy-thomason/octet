////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Physics example: How to use the basics of Bullet physics
//
// Level: 2
//
// Demonstrates:
//   Basic framework app
//   Shaders
//   Basic Matrices
//   Stock meshes
//   Phong shader
//

namespace octet {
  class physics_box {
    material *mat;
    mesh *box_mesh;
    int physics_index;
  public:
    physics_box() {
    }

    void init(material *_material, mesh *_box_mesh, int _physics_index) {
      mat = _material;
      box_mesh = _box_mesh;
      physics_index = _physics_index;
    }

    void render(physics_world &world, bump_shader &shader, const mat4t &worldToCamera, const mat4t &cameraToProjection, vec4 *light_uniforms, int num_light_uniforms, int num_lights) {
      mat4t modelToWorld;
      world.get_modelToWorld(modelToWorld, physics_index);
      mat4t modelToCamera = modelToWorld * worldToCamera;
      mat4t modelToProjection = modelToCamera * cameraToProjection;
      mat->render(shader, modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights);
      box_mesh->render();
    }

    void accelerate(physics_world &world, float amount) {
      mat4t modelToWorld;
      world.get_modelToWorld(modelToWorld, physics_index);
      world.apply_impulse(physics_index, modelToWorld[2] * amount);
    }

    void turn(physics_world &world, float amount) {
      mat4t modelToWorld;
      world.get_modelToWorld(modelToWorld, physics_index);
      world.apply_torque_impulse(physics_index, modelToWorld[1] * amount);
    }

    void get_modelToWorld(physics_world &world, mat4t &modelToWorld) {
      world.get_modelToWorld(modelToWorld, physics_index);
    }
  };

  class physics_app : public octet::app {
    // physics world contains matrices and motion state for all objects
    physics_world world;

    // Matrix to transform points in our camera space to the world.
    // This lets us move our camera
    mat4t cameraToWorld;

    // shader to draw a bump mapped shaded, textured triangle
    bump_shader bump_shader_;
  
    // material for boxes
    material box_mat;
  
    // mesh for boxes
    mesh box_mesh;
  
    // material for boxes
    material floor_mat;
  
    // mesh for boxes
    mesh floor_mesh;

    // storage for boxes
    dynarray<physics_box> boxes;

  public:

    // this is called when we construct the class
    physics_app(int argc, char **argv) : app(argc, argv) {
    }

    // this is called once OpenGL is initialized
    void app_init() {
      // set up the shader
      bump_shader_.init();

    
      // make a mesh to share amoungst all boxes
      box_mesh.make_cube(0.5f);
      box_mat.make_color(vec4(1, 0, 0, 1), false, false);

      float floor_size = 50.0f;
      floor_mesh.make_aa_box(floor_size, 0.5f, floor_size);
      floor_mat.make_color(vec4(0.3f, 1, 0.1f, 1), true, false);

      // make some boxes in the physics world with meshes for drawing.
      int num_boxes = 10;
      float box_spacing = 1.5f;
      boxes.resize(num_boxes+1);

      mat4t modelToWorld;
      modelToWorld.loadIdentity();
      modelToWorld.translate(-0.5f * box_spacing * (num_boxes-1), 4.0f, 0);

      for (int i = 0; i != num_boxes; ++i) {
        int body = world.add_rigid_body(modelToWorld, vec4(0.5f, 0.5f, 0.5f, 0), true, physics_world::body_box);
        boxes[i].init(&box_mat, &box_mesh, body);
        modelToWorld.translate(box_spacing, 0, 0);
      }

      {
        modelToWorld.loadIdentity();
        modelToWorld.translate(0, -2.0f, 0);
        int body = world.add_rigid_body(modelToWorld, vec4(floor_size, 0.5f, floor_size, 0), false, physics_world::body_box);
        boxes[num_boxes].init(&floor_mat, &floor_mesh, body);
      }

      //lights.add_light(vec4(10, 10, 10, 1), vec4(0, 0, 1, 0).normalize(), vec4(0.3f, 0.3f, 0.3f, 1), vec4(1, 1, 1, 1), vec4(1, 1, 1, 1));
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

      // improve draw speed by culling back faces - and avoid flickering edges
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
      glFrontFace(GL_CW);

      // set up the matrices with a camera 5 units from the origin
      boxes[0].get_modelToWorld(world, cameraToWorld);
      cameraToWorld.translate(0, 2, 5);

      // flip cameraToWorld around to transform from world to camera
      mat4t worldToCamera;
      cameraToWorld.invertQuick(worldToCamera);

      // build a projection matrix to add perspective
      mat4t cameraToProjection;
      cameraToProjection.loadIdentity();
      float n = 0.125f, f = 256.0f;
      cameraToProjection.frustum(-n, n, -n, n, n, f);

      // the lights are in camera space
      //lights.compute(worldToCamera);
      vec4 lights[5];
      memset(lights, 0, sizeof(lights));
      lights[0] = vec4(0.3f, 0.3f, 0.3f, 50);
      lights[2] = vec4(0.707f, 0, 0.707f, 0) * worldToCamera;
      lights[3] = vec4(1, 1, 1, 1);
      lights[4] = vec4(1, 0, 0, 1);

      for (unsigned i = 0; i != boxes.size(); ++i) {
        boxes[i].render(world, bump_shader_, worldToCamera, cameraToProjection, lights, 5, 1);
      }

      if (is_key_down('W')) {
        boxes[0].accelerate(world, -0.1f);
      }
      if (is_key_down('S')) {
        boxes[0].accelerate(world, 0.1f);
      }
      if (is_key_down(key_left)) {
        boxes[0].turn(world, 0.01f);
      }
      if (is_key_down(key_right)) {
        boxes[0].turn(world, -0.01f);
      }
      world.step(0.016f);
    }
  };
}
