////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Wireframe modifier. Generate a wireframe for the triangles in the source.
//

namespace octet { namespace scene {
  class wireframe : public mesh {
    // source mesh. Provides underlying geometry.
    ref<mesh> src;
  public:
    RESOURCE_META(wireframe)

    wireframe(mesh *src=0) {
      this->src = src;
      update();
    }

    void update() {
      if (!src) return;
      if (src->get_mode() != GL_TRIANGLES) return;

      *(mesh*)this = *(mesh*)src;

      set_mode(GL_LINES);

      // re-index the triangles to make lines
      unsigned index_type = src->get_index_type();
      unsigned num_indices = src->get_num_indices();
      unsigned isize = kind_size(index_type) * num_indices * 2;

      gl_resource *indices = new gl_resource(GL_ELEMENT_ARRAY_BUFFER, isize);
      void *dp = indices->lock();
      void *sp = src->get_indices()->lock();
      if (index_type == GL_UNSIGNED_SHORT) {
        uint16_t *s = (uint16_t*)sp;
        uint16_t *d = (uint16_t*)dp;
        for (unsigned i = 0; i < num_indices; i += 3) {
          d[0] = s[0]; d[1] = s[1];
          d[2] = s[1]; d[3] = s[2];
          d[4] = s[2]; d[5] = s[0];
          d += 6; s += 3;
        }
      } else { // assume GL_UNSIGNED_INT
        uint32_t *s = (uint32_t*)sp;
        uint32_t *d = (uint32_t*)dp;
        for (unsigned i = 0; i < num_indices; i += 3) {
          d[0] = s[0]; d[1] = s[1];
          d[2] = s[1]; d[3] = s[2];
          d[4] = s[2]; d[5] = s[0];
          d += 6; s += 3;
        }
      }

      indices->unlock();
      src->get_indices()->unlock();
      set_num_indices(num_indices*2);
      set_indices( indices );
    }

    void visit(visitor &v) {
      mesh::visit(v);
      v.visit(src, atom_src);
    }
  };
}}
