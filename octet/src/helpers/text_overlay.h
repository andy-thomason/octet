////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
//
// scene for rendering text overlays

namespace octet { namespace helpers {
  class text_overlay {
    ref<visual_scene> text_scene;
    ref<camera_instance> cam;
    ref<mesh_instance> msh_inst;
    ref<mesh_text> text;

  public:
    void init() {
      // Make a scene for the text overlay using an ortho camera
      // that works in screen pixels.
      text_scene = new visual_scene();

      image *page = new image("assets/courier_18_0.gif");
      page->load();
      bitmap_font *font = new bitmap_font(
        page->get_width(), page->get_height(), "assets/courier_18.fnt"
      );

      /*static const char sample_text[] =
        "Antonio: Where is the master, boatswain?\n"
        "Boatswain: Do you not hear him? You mar our labour: keep your cabins; you do assist the storm.\n"
        "Gonzalo: Nay, good, be patient.\n"
        "Boatswain: When the sea is. Hence! What cares these roarers for the name of king? To cabin! silence! Trouble us not.\n"
        "Gonzalo: Good, yet remember whom thou hast aboard.\n"
        "Boatswain: None that I more love than myself. You are counsellor; — if you can command these elements to silence, and work the peace of the present, we will not hand a rope more. Use your authority; if you cannot, give thanks you have liv'd so long, and make yourself ready in your cabin for the mischance of the hour, if it so hap.\n"
      ;*/

      aabb bb(vec3(0, 0, 0), vec3(64, 256, 0));
      text = new mesh_text(font, "Hello", &bb);

      scene_node *msh_node = text_scene->add_scene_node();
      material *mat = new material(page);
      msh_inst = new mesh_instance(msh_node, text, mat);
      text_scene->add_mesh_instance(msh_inst);

      text_scene->create_default_camera_and_lights();
      cam = text_scene->get_camera_instance(0);
      cam->get_node()->access_nodeToParent().loadIdentity();
    }

    void render(bump_shader &object_shader, bump_shader &skin_shader, int vx, int vy, int frame_number) {
      cam->set_ortho((float)vx, (float)vy, 1, -1, 1);
      camera_instance *cam = text_scene->get_camera_instance(0);
      text_scene->set_dump_vertices(frame_number == 0);

      text_scene->render(object_shader, skin_shader, *cam, 1);
    }
  };
}}

