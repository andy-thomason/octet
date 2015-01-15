////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Text mesh source.
//

namespace octet { namespace scene {
  /// Mesh for formatting text. Uses the bitmap_font resource.
  class mesh_text : public mesh {
    typedef bitmap_font::vertex vertex;
    ref<bitmap_font> font;
    int max_quads;
    string text;
    aabb bb;
  public:
    RESOURCE_META(mesh_text)

    /// make a new formated text mesh inside the bounding box.
    mesh_text(bitmap_font *font_ = 0, const char *text_="", aabb *bb_ = 0) {
      font = font_;
      text = text_;
      bb = bb_ ? *bb_ : aabb(vec3(0, 0, 0), vec3(64, 64, 0));
      max_quads = 0;

	    add_attribute(attribute_pos, 3, GL_FLOAT, sizeof(float)*0);
	    add_attribute(attribute_uv, 2, GL_FLOAT, sizeof(float)*3);
	    add_attribute(attribute_color, 4, GL_UNSIGNED_BYTE, sizeof(float)*5);
      set_params(sizeof(vertex), 0, 0, GL_TRIANGLES, GL_UNSIGNED_INT);

      if (text.size()) update();
    }

    void clear() {
      text = "";
    }

    void format(const char *fmt, ...) {
      va_list list;
      va_start(list, fmt);
      text.vformat(fmt, list);
      va_end(list);
    }

    /// update the OpenGL geometry.
    void update() {
      if (!font) return;

      if (text.size() > max_quads) {
        max_quads = std::max(32, (text.size() + 15) & ~15); // round up to 16
	      unsigned max_vertices = max_quads * 4;
	      unsigned max_indices = max_quads * 6;
	      unsigned vsize = sizeof(vertex) * max_vertices;
	      unsigned isize = sizeof(uint32_t) * max_indices;
	      allocate(vsize, isize);
      }

      vertex *vtx = (vertex *)get_vertices()->lock();
      uint32_t *idx = (uint32_t *)get_indices()->lock();

      unsigned num_quads = font->build_mesh(
        bb, vtx, idx, max_quads,
        text.c_str(), text.c_str() + text.size()
      );

      get_vertices()->unlock();
      get_indices()->unlock();
      set_num_indices(num_quads * 6);
      set_num_vertices(num_quads * 4);
    }

    /// Serialize.
    void visit(visitor &v) {
      mesh::visit(v);
      v.visit(font, atom_font);
      v.visit(text, atom_text);
      v.visit(bb, atom_aabb);
    }
  };
}}
