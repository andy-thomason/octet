////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// obb collision testing.
//
// Level: 2
//
// Work for a paper on OBB collision

namespace octet {
  class obb_test : public app {
    typedef mat4t mat4t;
    typedef vec4 vec4;
    typedef animation animation;
    typedef scene_node scene_node;

    // shaders to draw triangles
    bump_shader object_shader;
    bump_shader skin_shader;

    mouse_ball ball;

    ref<scene> app_scene;
    ref<scene_node> nodea;
    ref<scene_node> nodeb;
    ref<mesh_instance> mia;
    ref<mesh_instance> mib;

  public:
    // this is called when we construct the class
    obb_test(int argc, char **argv) : app(argc, argv), ball() {
    }

    // this is called once OpenGL is initialized
    void app_init() {
      // set up the shaders
      object_shader.init(false);
      skin_shader.init(true);

      app_scene =  new scene();
      app_scene->create_default_camera_and_lights();
      nodea = app_scene->add_scene_node();
      nodeb = app_scene->add_scene_node();
      nodea->access_nodeToParent().translate(-2, 0, 0);
      nodeb->access_nodeToParent().translate(3, 0, 0);
      material *mata = new material(vec4(1, 0, 0, 1));
      material *matb = new material(vec4(0, 0, 1, 1));
      mesh *mesha = new mesh_voxels(vec3(8, 8, 8));
      mesh *meshb = new mesh_box(vec3(1, 1, 1));
      mia = app_scene->add_mesh_instance(new mesh_instance(nodea, mesha, mata));
      mib = app_scene->add_mesh_instance(new mesh_instance(nodeb, meshb, matb));

      camera_instance *cam = app_scene->get_camera_instance(0);
      scene_node *node = cam->get_node();
      node->access_nodeToParent().translate(0, 0, 5);
      mat4t cameraToWorld = node->get_nodeToParent();
      ball.init(this, cameraToWorld.w().length(), 360.0f);
    }

    // this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx, vy;
      get_viewport_size(vx, vy);
      // set a viewport - includes whole window area
      glViewport(0, 0, vx, vy);

      // clear the background to black
      glClearColor(0.5f, 0.5f, 0.5f, 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // allow Z buffer depth testing (closer objects are always drawn in front of far ones)
      glEnable(GL_DEPTH_TEST);

      GLint param;
      glGetIntegerv(GL_SAMPLE_BUFFERS, &param);
      if (param == 0) {
        // if multisampling is disabled, we can't use GL_SAMPLE_COVERAGE (which I think is mean)
        // Instead, allow alpha blend (transparency when alpha channel is 0)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      } else {
        // if multisampling is enabled, use GL_SAMPLE_COVERAGE instead
        glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
        glEnable(GL_SAMPLE_COVERAGE);
      }

      if (app_scene && app_scene->get_num_camera_instances()) {
        int vx = 0, vy = 0;
        get_viewport_size(vx, vy);

        camera_instance *cam = app_scene->get_camera_instance(0);
        scene_node *node = cam->get_node();
        mat4t &cameraToWorld = node->access_nodeToParent();
        ball.update(cameraToWorld);

        // update matrices. assume 30 fps.
        app_scene->update(1.0f/30);

        app_scene->render(object_shader, skin_shader, *cam, (float)vx / vy);

        const aabb &aabba = mia->get_mesh()->get_aabb();
        const aabb &aabbb = mib->get_mesh()->get_aabb();
        const mat4t &mxa = mia->get_node()->get_nodeToParent();
        const mat4t &mxb = mib->get_node()->get_nodeToParent();
        //aabba.intersects_old(aabbb, mxa, mxb);
        volatile bool b = aabba.intersects(aabbb, mxa, mxb);
        //printf("\n");

        nodea->access_nodeToParent().rotateZ(1);
        nodea->access_nodeToParent().rotateX(1);
      }
    }
  };
}
