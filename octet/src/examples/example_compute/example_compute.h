////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {
  /// Scene containing a box with octet.
  class example_compute : public app {
    // scene for drawing box
    ref<visual_scene> app_scene;

    // a computer shader to generate geometry.
    ref<compute_shader> compute;

    // the mesh for our geometry
    ref<mesh> helix;

    // resolution of our geometry
    size_t num_steps;

    // the format of our vertex
    struct my_vertex {
      vec4 pos;
      vec4 color;
    };

  public:
    /// this is called when we construct the class before everything is initialised.
    example_compute(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene = new visual_scene();
      app_scene->create_default_camera_and_lights();

      compute = new compute_shader("shaders/helix.cs");

      // use a shader that just outputs the color_ attribute.
      param_shader *shader = new param_shader("shaders/default.vs", "shaders/simple_color.fs");
      material *red = new material(vec4(1, 0, 0, 1), shader);

      // create a new mesh.
      helix = new mesh();

      // number of steps in helix. Increase this to make it smoother
      num_steps = 320;

      // allocate vertices and indices into OpenGL buffers
      size_t num_vertices = num_steps * 2 + 2;
      size_t num_indices = num_steps * 6;

      helix->allocate(sizeof(my_vertex)* num_vertices, sizeof(uint32_t)* num_indices);
      helix->set_params(sizeof(my_vertex), num_indices, num_vertices, GL_TRIANGLES, GL_UNSIGNED_INT);

      {
        // this step seems to be necessary on AMD drivers
        gl_resource::wolock vl(helix->get_vertices());
        memset(vl.u8(), 0xff, helix->get_vertices()->get_size());
      }

      // describe the structure of my_vertex to OpenGL
      helix->add_attribute(attribute_pos, 3, GL_FLOAT, 0);
      helix->add_attribute(attribute_color, 4, GL_FLOAT, 16);

      {
        // make the index buffer
        gl_resource::wolock il(helix->get_indices());
        uint32_t *idx = il.u32();

        // make the triangles: note we could have just used a tristrip here...
        uint32_t vn = 0;
        for (size_t i = 0; i != num_steps; ++i) {
          idx[0] = vn + 0; idx[1] = vn + 3; idx[2] = vn + 1;
          idx += 3;

          idx[0] = vn + 0; idx[1] = vn + 2; idx[2] = vn + 3;
          idx += 3;

          vn += 2;
        }
      }

      // add a scene node with this new mesh.
      scene_node *node = new scene_node();
      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, helix, red));
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      // update matrices. assume 30 fps.
      app_scene->update(1.0f/30);

      // animate the radii
      int frame = get_frame_number();
      float radius1 = cosf(frame * 3.14159265f / 100) * 10.0f;
      float radius2 = sinf(frame * 3.14159265f / 100) * 10.0f;

      // run the compute shader to generate dynamic geometry
      compute->use();

      // copy variables to the program
      GLuint prog = compute->get_program();
      glUniform1i(glGetUniformLocation(prog, "num_steps"), (int)num_steps);
      glUniform1f(glGetUniformLocation(prog, "radius1"), radius1);
      glUniform1f(glGetUniformLocation(prog, "radius2"), radius2);

      // bind the vertex buffer to out_buf
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, helix->get_vertices()->get_buffer());

      // kick the shader.
      compute->dispatch((num_steps + 63) / 64);

      // unbind the vertex buffer
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);

      // make sure the memory writes happen before reading the vertices.
      // this is needed because the computer shader and vertex shader run asyncronously.
      glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

      // draw the scene
      app_scene->render((float)vx / vy);

      // spin the helix
      scene_node *node = app_scene->get_mesh_instance(0)->get_node();
      node->rotate(5, vec3(0, 1, 0));
    }
  };
}
