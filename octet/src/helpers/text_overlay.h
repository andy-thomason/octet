////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
//
// scene for rendering text overlays

namespace octet {
  class text_overlay {
    ref<scene> text_scene;
    ref<camera_instance> cam;
    ref<mesh_instance> msh_inst;
    ref<mesh_text> text;
    ref<bitmap_font> font;

  public:
    void init() {
      // Make a scene for the text overlay using an ortho camera
      // that works in screen pixels.
      text_scene = new scene();

      font = new bitmap_font("assets/courier_18.fnt");

      text = new mesh_text(font);

      scene_node *msh_node = text_scene->add_scene_node();
      material *mat = new material("assets/courier_18_0.gif");
      msh_inst = new mesh_instance(msh_node, text, mat);
      text_scene->add_mesh_instance(msh_inst);

      text_scene->create_default_camera_and_lights();
      cam = text_scene->get_camera_instance(0);
      cam->get_node()->access_nodeToParent().loadIdentity();
    }

    void render(bump_shader &object_shader, bump_shader &skin_shader, int vx, int vy, int frame_number) {
      cam->set_ortho((float)vx, (float)vy, 1, 0, 1);
      camera_instance *cam = text_scene->get_camera_instance(0);
      text_scene->set_dump_vertices(frame_number == 0);

      text_scene->render(object_shader, skin_shader, *cam, 1);
    }
  };
}

