////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
//
// scene for rendering text overlays

namespace octet { namespace helpers {
  /// Class for managing text overlays.
  class text_overlay : public resource {
    ref<visual_scene> text_scene;
    ref<bitmap_font> font;
    ref<material> mat;
    ref<scene_node> node;
  public:
    /// Create an empty text overlay.
    text_overlay() {
      image *page = new image("assets/courier_18_0.gif");
      page->load();
      font = new bitmap_font(
        page->get_width(), page->get_height(), "assets/courier_18.fnt"
      );

      // Make a scene for the text overlay using an ortho camera
      // that works in screen pixels.
      text_scene = new visual_scene();

      param_shader *shader = new param_shader("shaders/default.vs", "shaders/text.fs");

      // Make a material from the font image.
      mat = new material(page, NULL, shader);
      shader->init(mat->get_params());

      // Make a default node for the scene (no transformation)
      node = text_scene->add_scene_node();

      // Create scene defaults.
      text_scene->create_default_camera_and_lights();
    }

    /// Add a block of text
    void add_mesh_text(mesh_text *mesh) {
      mesh_instance *msh_inst = new mesh_instance(node, mesh, mat);
      text_scene->add_mesh_instance(msh_inst);
    }

    /// Render the text overlay
    void render(int vx, int vy) {
      camera_instance *cam = text_scene->get_camera_instance(0);
      cam->get_node()->loadIdentity();
      cam->set_ortho((float)vx, (float)vy, 1, -1, 1);
      text_scene->render(1);
    }

    /// Get the default font.
    bitmap_font *get_default_font() {
      return font;
    }
  };
}}

