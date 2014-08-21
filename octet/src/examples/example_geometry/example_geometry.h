////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {
  /// Scene containing a helix with octet.
  class example_geometry : public app {
    // scene for drawing helix
    ref<visual_scene> app_scene;

    // this is the vertex format used in this sample.
    // we have 12 bytes of float position (x, y, z) and four bytes of color (r, g, b, a)
    struct my_vertex {
      vec3p pos;
      uint32_t color;
    };

    // this function converts three floats into a RGBA 8 bit color
    static uint32_t make_color(float r, float g, float b) {
      return 0xff000000 + ((int)(r*255.0f) << 0) + ((int)(g*255.0f) << 8) + ((int)(b*255.0f) << 16);
    }
  public:
    /// this is called when we construct the class before everything is initialised.
    example_geometry(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();

      // use a shader that just outputs the color_ attribute.
      param_shader *shader = new param_shader("shaders/default.vs", "shaders/simple_color.fs");
      material *red = new material(vec4(1, 0, 0, 1), shader);

      // create a new mesh.
      mesh *helix = new mesh();

      // number of steps in helix
      size_t num_steps = 320;

      // allocate vertices and indices into OpenGL buffers
      size_t num_vertices = num_steps * 2 + 2;
      size_t num_indices = num_steps * 6;
      helix->allocate(sizeof(my_vertex) * num_vertices, sizeof(uint32_t) * num_indices);
      helix->set_params(sizeof(my_vertex), num_indices, num_vertices, GL_TRIANGLES, GL_UNSIGNED_INT);

      // describe the structure of my_vertex to OpenGL
      helix->add_attribute(attribute_pos, 3, GL_FLOAT, 0);
      helix->add_attribute(attribute_color, 4, GL_UNSIGNED_BYTE, 12, GL_TRUE);

      {
        // these write-only locks give access to the vertices and indices.
        // they will be released at the next } (the end of the scope)
        gl_resource::wolock vl(helix->get_vertices());
        my_vertex *vtx = (my_vertex *)vl.u8();
        gl_resource::wolock il(helix->get_indices());
        uint32_t *idx = il.u32();

        // make the vertices
        float radius1 = 1.0f;
        float radius2 = 7.0f;
        float height = 24.0f;
        float num_twists = 4.0f;
        for (size_t i = 0; i != num_steps+1; ++i) {
          float r = 0.0f, g = 1.0f * i / num_steps, b = 1.0f;
          float y = i * (height / num_steps) - height * 0.5f;
          float angle = i * (num_twists * 2.0f * 3.14159265f / num_steps);
          vtx->pos = vec3p(cosf(angle) * radius1, y, sinf(angle) * radius1);
          vtx->color = make_color(r, g, b);
          log("%f %f %f %08x\n", r, g, b, vtx->color);
          vtx++;
          vtx->pos = vec3p(cosf(angle) * radius2, y, sinf(angle) * radius2);
          vtx->color = make_color(r, g, b);
          vtx++;
        }

        // make the triangles
        uint32_t vn = 0;
        for (size_t i = 0; i != num_steps; ++i) {
          // 0--2
          // | \|
          // 1--3
          idx[0] = vn + 0;
          idx[1] = vn + 3;
          idx[2] = vn + 1;
          idx += 3;

          idx[0] = vn + 0;
          idx[1] = vn + 2;
          idx[2] = vn + 3;
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

      // draw the scene
      app_scene->render((float)vx / vy);

      // tumble the helix  (there is only one mesh instance)
      scene_node *node = app_scene->get_mesh_instance(0)->get_node();
      //node->rotate(1, vec3(1, 0, 0));
      node->rotate(1, vec3(0, 1, 0));
    }
  };
}
