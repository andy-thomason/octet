////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {
  /// Scene containing a duck with octet.
  class example_duck : public app {
    // scene for drawing duck
    ref<visual_scene> app_scene;

    collada_builder loader;
  public:
    /// this is called when we construct the class before everything is initialised.
    example_duck(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene =  new visual_scene();

      resource_dict dict;
      loader.load_xml("assets/duck_triangulate.dae");
      loader.get_resources(dict);

      dynarray<resource*> meshes;
      dict.find_all(meshes, atom_mesh);

      if (meshes.size()) {
        material *mat = new material(new image("assets/duckCM.gif"));
        //material *mat = new material(vec4(1, 0, 0, 1));
        mesh *duck = meshes[0]->get_mesh();
        scene_node *node = new scene_node();
        node->translate(vec3(-50, -50, 0));
        app_scene->add_child(node);
        app_scene->add_mesh_instance(new mesh_instance(node, duck, mat));
      }

      app_scene->create_default_camera_and_lights();
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

      // tumble the duck  (there is only one mesh instance)
      scene_node *node = app_scene->get_mesh_instance(0)->get_node();
      node->rotate(1, vec3(1, 0, 0));
      node->rotate(1, vec3(0, 1, 0));
    }
  };
}
