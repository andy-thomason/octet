////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {
  /// Scene containing a box with octet.
  class example_opencl : public app {
    // scene for drawing box
    ref<visual_scene> app_scene;
    GLuint texture;
    int width, height, time, workgroup_size;
    ref<opencl::mem> image_mem;
    ref<opencl::kernel> kernel;
    dynarray<uint32_t> pixels;
    image *img;

    opencl cl;
  public:
    /// this is called when we construct the class before everything is initialised.
    example_opencl(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    /// (breaking RAII!)
    void app_init() {
      width = 512;
      height = 512;
      workgroup_size = 16; // if the program blows up, reduce this.
      time = 0;

      char defines[256];
      sprintf(defines, "#define max_width %d\n#define max_workgroup_size %d\n", width, workgroup_size);
      cl.init("src/examples/example_opencl/example.cl", "NVIDIA", defines);
      const char *err = cl.get_error();
      if (err) {
        fputs(err, stderr);
      }

      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();

      glGenTextures(1, &texture);
      glBindTexture(GL_TEXTURE_2D, texture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

      img = new image(GL_TEXTURE_2D, texture, width, height);

      // NOTE: we are using the dma kernel here. If the sample does not run, 
      image_mem = new opencl::mem(cl, CL_MEM_WRITE_ONLY, 0, width*height*4);
      kernel = new opencl::kernel(cl, "dma_kernel");
      pixels.resize(width * height);

      material *mat = new material(img);
      mesh_box *box = new mesh_box(vec3(8));
      scene_node *node = new scene_node();
      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, box, mat));
    }

    /// update the texture using opencl
    void update_texture() {
      // call the kernel
      kernel->begin();
      kernel->push(image_mem->get_obj());
      kernel->push(width);
      kernel->push(height);
      kernel->push(time);
      cl_event call = kernel->call(height, workgroup_size, 0, true);

      // read into CPU memory (after call is finished)
      cl_event read = image_mem->read(width*height*4, pixels.data(), call, true);

      // pause the CPU (until read is finished)
      cl.wait(read);

      // wait for CL to finish
      cl.finish();

      // use glTexSubImage to reload the pixels from the CPU memory.
      // would love to use cl/gl integration, but have problems...
      img->reload(GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

      time = (time + 1) & 0x3f;
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      update_texture();
      if (cl.get_error()) {
        return;
      }

      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      // update matrices. assume 30 fps.
      app_scene->update(1.0f/30);

      // draw the scene
      app_scene->render((float)vx / vy);

      // tumble the box  (there is only one mesh instance)
      scene_node *node = app_scene->get_mesh_instance(0)->get_node();
      node->rotate(1, vec3(1, 0, 0));
      node->rotate(1, vec3(0, 1, 0));
    }
  };
}
