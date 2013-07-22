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

    struct vertex {
      float x, y, z;
      float u, v;
      unsigned color;
    };

  public:
    void init() {
      // Make a scene for the text overlay using an ortho camera
      // that works in screen pixels.
      text_scene = new scene();

      scene_node *msh_node = text_scene->add_scene_node();
      mesh *msh = new mesh();
      material *mat = new material("assets/courier_18_0.gif");
      msh_inst = new mesh_instance(msh_node, msh, mat);
      text_scene->add_mesh_instance(msh_inst);

      text_scene->create_default_camera_and_lights();
      cam = text_scene->get_camera_instance(0);
      cam->get_node()->access_nodeToParent().loadIdentity();

	    msh->add_attribute(attribute_pos, 3, GL_FLOAT, sizeof(float)*0);
	    msh->add_attribute(attribute_uv, 2, GL_FLOAT, sizeof(float)*3);
	    msh->add_attribute(attribute_color, 4, GL_UNSIGNED_BYTE, sizeof(float)*5);
	    unsigned max_vertices = 2048;
	    unsigned max_indices = 2048;
	    unsigned vsize = sizeof(vertex) * max_vertices;
	    unsigned isize = sizeof(unsigned short) * max_indices;
	    msh->allocate(vsize, isize, false);
      //msh->assign(vsize, isize, (unsigned char*)&state.vertices[0], (unsigned char*)&state.indices[0]);
      msh->set_params(sizeof(vertex), 0, 0, GL_TRIANGLES, GL_UNSIGNED_SHORT);

      // 0 1
      // 2 3
      static const vertex vtx[] = {
        {   0,   0, 0.5f,  0, 0, 0xffffffff },
        { 256,   0, 0.5f,  1, 0, 0xffffffff },
        {   0, 256, 0.5f,  0, 1, 0xffffffff },
        { 256, 256, 0.5f,  1, 1, 0xffffffff },
      };

      static const unsigned short ind[] = {
        0, 1, 2, 2, 1, 3,
      };

      msh->get_vertices().assign((void*)vtx, 0, sizeof(vtx));
      msh->get_indices().assign((void*)ind, 0, sizeof(ind));
      msh->set_num_vertices(4);
      msh->set_num_indices(6);
    }

    void render(bump_shader &object_shader, bump_shader &skin_shader, int vx, int vy) {
      cam->set_ortho(cam->get_node(), (float)vx, (float)vy, 1, 0, 1);
      camera_instance *cam = text_scene->get_camera_instance(0);
      //text_scene->render(object_shader, skin_shader, *cam);
    }
  };
}

