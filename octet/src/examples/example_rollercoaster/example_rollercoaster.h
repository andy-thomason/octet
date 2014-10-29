////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {
  /// Scene containing a box with octet.
  class example_rollercoaster : public app {
    // scene for drawing box
    ref<visual_scene> app_scene;
    collada_builder loader;
  public:
    /// this is called when we construct the class before everything is initialised.
    example_rollercoaster(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();
      scene_node *cam = app_scene->get_camera_instance(0)->get_node();
      cam->loadIdentity();
      cam->translate(vec3(0, 20, 4));
      cam->rotate(-90, vec3(1, 0, 0));

      if (!loader.load_xml("assets/rollercoaster.dae")) {
        printf("failed to load file!\n");
        exit(1);
      }
      resource_dict dict;
      loader.get_resources(dict);

      // note that this call will dump the code below to log.txt
      dict.dump_assets(log(""));

      scene_node *trough = dict.get_scene_node("trough");
      scene_node *Camera = dict.get_scene_node("Camera");
      material *default_material = dict.get_material("default_material");
      material *Material_material = dict.get_material("Material-material");
      mesh *trough_mesh_Material_material = dict.get_mesh("trough-mesh+Material-material");
      visual_scene *Scene = dict.get_visual_scene("Scene");
      scene_node *Lamp = dict.get_scene_node("Lamp");
      scene_node *start = dict.get_scene_node("start");

      mat4t location;
      location.translate(vec3(0, 0, 0));
      location.rotateX90();
      material *red = new material(vec4(1, 0, 0, 1));
      app_scene->add_shape(location, trough_mesh_Material_material, red, false);

      material *blue = new material(vec4(0, 0, 1, 1));
      mesh_sphere *sphere = new mesh_sphere(vec3(0, 0, 0), 0.2f);
      for (int i = -10; i <= 10; ++i) {
        for (int j = -10; j <= 10; ++j) {
          mat4t location;
          location.translate((float)i, 5, (float)j);
          app_scene->add_shape(location, sphere, blue, true);
        }
      }
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
    }
  };
}
