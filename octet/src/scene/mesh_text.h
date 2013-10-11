////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Text mesh source.
//

namespace octet {
  class mesh_text : public mesh {
    typedef bitmap_font::vertex vertex;
    ref<bitmap_font> font;
  public:
    RESOURCE_META(mesh_text)

    mesh_text(bitmap_font *font_ = 0) {
      font = font_;
      update();
    }

    void update() {
      if (!font) return;

	    add_attribute(attribute_pos, 3, GL_FLOAT, sizeof(float)*0);
	    add_attribute(attribute_uv, 2, GL_FLOAT, sizeof(float)*3);
	    add_attribute(attribute_color, 4, GL_UNSIGNED_BYTE, sizeof(float)*5);
      unsigned max_quads = 16384;
	    unsigned max_vertices = max_quads * 4;
	    unsigned max_indices = max_quads * 6;
	    unsigned vsize = sizeof(vertex) * max_vertices;
	    unsigned isize = sizeof(uint32_t) * max_indices;
	    allocate(vsize, isize);
      set_params(sizeof(vertex), 0, 0, GL_TRIANGLES, GL_UNSIGNED_INT);

      vertex *vtx = (vertex *)get_vertices()->lock();
      uint32_t *idx = (uint32_t *)get_indices()->lock();

      char text[] = "hello";
      //char arabic_text[] = "أخبار الوطن العربي";

      int xdraw = 0;
      int ydraw = 0;

      unsigned num_quads = font->build_mesh(xdraw, ydraw, vtx, idx, max_quads, text, text + strlen(text));

      get_vertices()->unlock();
      get_indices()->unlock();
      set_num_indices(num_quads * 6);
      set_num_vertices(num_quads * 4);
    }

    void visit(visitor &v) {
      mesh::visit(v);
      v.visit(font, atom_font);
    }
  };
}
