////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {
  /// Example cellular automaton - based on Andy Howe's space dust.
  class example_cellular : public app {
    // scene for drawing box
    ref<visual_scene> app_scene;

    ref<image> img;

    enum { dim = 512 };
    dynarray<uint32_t> values;
    //uint32_t *values;[dim*dim];
    random r;
  public:
    /// this is called when we construct the class before everything is initialised.
    example_cellular(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      values.resize(dim * dim);

      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();


      GLuint gl_texture;
      glGenTextures(1, &gl_texture);
      glBindTexture(GL_TEXTURE_2D, gl_texture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dim, dim, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

      img = new image(GL_TEXTURE_2D, gl_texture, dim, dim, 1);
      material *red = new material(img);
      mesh_box *box = new mesh_box(vec3(8));
      scene_node *node = new scene_node();
      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, box, red));

      for (int y = 1; y != dim-1; ++y) {
        values[y*dim+0] = 0;
        for (int x = 1; x != dim-1; ++x) {
          values[y*dim+x] = y > dim*3/4 ? 0xff0000ff : 0xffff0000;
        }
        values[y*dim+dim-1] = 0;
      }
      for (int x = 0; x != dim; ++x) {
        values[0*dim+x] = 0;
        values[(dim-1)*dim+x] = 0;
      }
      for (int x = 0; x != dim; ++x) {
        if ((x-32) & 0x60) values[(dim/2)*dim+x] = 0xff000000;
      }
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      scene_node *target = app_scene->get_mesh_instance(0)->get_node();
      vec3 target_pos = target->access_nodeToParent().w().xyz();
      app_scene->get_camera_instance(0)->get_node()->access_nodeToParent().lookat(target_pos);
      mat4t m = app_scene->get_camera_instance(0)->get_node()->access_nodeToParent();
      m.lookat(target_pos);

      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      static const int off[] = {
        -dim - 1, -dim, -dim + 1,
        -1,             1,
         dim - 1,  dim,  dim + 1,

         dim, dim, dim, dim,
         dim, dim, dim, dim,
         dim, dim, dim, dim,
         dim, dim, dim, dim,
         dim, dim, dim, dim,
         dim, dim, dim, dim,
      };

      for (int y = 1; y != dim-1; ++y) {
        for (int x = 1; x != dim-1; ++x) {
          uint32_t &dest = values[y*dim+x];
          if (dest == 0xffff0000) {
            int rval = r.get0xffff() & 31;
            int ofs = off[rval];
            uint32_t &src = values[y*dim+x+ofs];
            if (src == 0xff0000ff) {
              std::swap(src, dest);
            }
          }
        }
      }

      glBindTexture(GL_TEXTURE_2D, img->get_gl_texture());
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, dim, dim, GL_RGBA, GL_UNSIGNED_BYTE, values.data());

      // update matrices. assume 30 fps.
      app_scene->update(1.0f/30);

      // draw the scene
      app_scene->render((float)vx / vy);

      // tumble the box  (there is only one mesh instance)
      scene_node *node = app_scene->get_mesh_instance(0)->get_node();
      //node->rotate(1, vec3(1, 0, 0));
      //node->rotate(1, vec3(0, 1, 0));
    }
  };
}
