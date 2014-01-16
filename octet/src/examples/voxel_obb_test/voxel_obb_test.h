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
  class and_not {
    bool result;
  public:
    and_not(bool a_in, bool b_in) { result = a_in; }
    operator bool() { return result; }
  };

  class deathstar {
    sphere a;
    sphere b;
  public:
    //deathstar() : csg(sphere(vec3(0, 0, 0), 16), sphere(vec3(8, 0, 0), 12)) {}
    //bool intersects(const vec3 &rhs) const { return csg::intersects(rhs); }
    deathstar() : a(vec3(0, 0, 0), 16), b(vec3(8, 0, 0), 12) {
    }

    bool intersects(vec3_in pos) const {
      return a.intersects(pos) && !b.intersects(pos);
    }
  };

  class voxel_obb_test : public app {
    typedef mat4t mat4t;
    typedef vec4 vec4;
    typedef animation animation;
    typedef scene_node scene_node;

    // shaders to draw triangles
    bump_shader object_shader;
    bump_shader skin_shader;

    mouse_ball ball;

    ref<visual_scene> app_scene;
    ref<scene_node> nodea;
    ref<scene_node> nodeb;
    ref<mesh_instance> mia;
    ref<mesh_instance> mib;

  public:
    // this is called when we construct the class
    voxel_obb_test(int argc, char **argv) : app(argc, argv), ball() {
    }

    // this is called once OpenGL is initialized
    void app_init() {
      // set up the shaders
      object_shader.init(false);
      skin_shader.init(true);

      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();
      nodea = app_scene->add_scene_node();
      nodeb = app_scene->add_scene_node();
      //nodea->access_nodeToParent().translate(-2, 0, 0);
      //nodeb->access_nodeToParent().translate(3, 0, 0);
      material *mata = new material(vec4(1, 0, 0, 1));
      material *matb = new material(vec4(0, 0, 1, 1));

      mat4t mx;
      mx.loadIdentity();
      mesh_voxels *mesha = new mesh_voxels(1.0f/8);
      mesha->draw(mx, aabb(vec3(0, 0, 0), vec3(16, 16, 16)));
      mesha->update();

      mesh_voxels *meshb = new mesh_voxels(1.0f/8);
      meshb->draw(mx, aabb(vec3(0, 0, 0), vec3(16, 16, 16)));
      meshb->update();

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
      if (app_scene && app_scene->get_num_camera_instances()) {
        int vx = 0, vy = 0;
        get_viewport_size(vx, vy);
        app_scene->begin_render(vx, vy);

        camera_instance *cam = app_scene->get_camera_instance(0);
        scene_node *node = cam->get_node();
        mat4t &cameraToWorld = node->access_nodeToParent();
        ball.update(cameraToWorld);

        // update matrices. assume 30 fps.
        app_scene->update(1.0f/30);

        app_scene->render(object_shader, skin_shader, *cam, (float)vx / vy);

        mesh_voxels *mesha = mia->get_mesh()->get_mesh_voxels();
        mesh_voxels *meshb = mib->get_mesh()->get_mesh_voxels();

        const aabb &aabba = mia->get_mesh()->get_aabb();
        const aabb &aabbb = mib->get_mesh()->get_aabb();
        const mat4t &mxa = mia->get_node()->get_nodeToParent();
        const mat4t &mxb = mib->get_node()->get_nodeToParent();
        //aabba.intersects_old(aabbb, mxa, mxb);
        //volatile bool b = mesha->intersects(*meshb, mxa, mxb);
        //printf("\n");

        //nodea->access_nodeToParent().rotateZ(1);
        //nodea->access_nodeToParent().rotateX(1);
      }
    }
  };
}
