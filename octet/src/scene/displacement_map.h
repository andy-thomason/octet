////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// displacement map modifier. Generate a view-dependent mesh that can be displaced by a function.
//

namespace octet {
  class displacement_map : public mesh {
    // source mesh. Provides underlying geometry.
    ref<mesh> src;
    vec3 view_pos;
    vec3 view_dir;
  public:
    RESOURCE_META(displacement_map)

    displacement_map(mesh *src=0) {
      this->src = src;
      view_pos = vec3(0, 0, 0);
      update();
    }

    void update() {
      if (!src) return;
      if (src->get_mode() != GL_TRIANGLES) return;
      if (src->get_index_type() != GL_UNSIGNED_INT) return;

      *(mesh*)this = *(mesh*)src;

      hash_map<uint64_t, unsigned> edges;

      unsigned num_floats = get_stride() / sizeof(float);
      unsigned num_triangles = get_num_indices() / 3;
      unsigned num_vertices = get_num_vertices();
      unsigned vsize = get_stride() * get_num_vertices() * 2;
      unsigned isize = sizeof(uint32_t) * get_num_indices() * 4;

      gl_resource *vertices = new gl_resource(GL_ARRAY_BUFFER, vsize);
      gl_resource *indices = new gl_resource(GL_ELEMENT_ARRAY_BUFFER, isize);
      const void *sp = src->get_vertices()->lock_read_only();
      void *dp = vertices->lock();
      const void *sip = src->get_vertices()->lock_read_only();
      void *dip = indices->lock();
      int normal_slot = src->get_slot(attribute_normal);

      memcpy(dp, sp, get_num_vertices() * get_stride());

      unsigned num_dest_vertices = num_vertices;
      uint32_t *di = (uint32_t*)dip;

      for (unsigned i = 0; i != num_triangles; ++i) {
        unsigned i0 = ((uint32_t*)sip)[i*3+2];
        // 0 1 2 (2,0) (0,1) (1,2)
        unsigned ind[6];
        for (unsigned j = 0; j != 3; ++j) {
          unsigned i1 = ((uint32_t*)sip)[i*3+j];
          ind[j] = i1;
          
          uint64_t key = i0 < i1 ?
            ((uint64_t)i0 << 32) | i1 :
            ((uint64_t)i1 << 32) | i0
          ;

          unsigned &e = edges[key];
          if (e == 0) {
            e = num_dest_vertices++;
          }
          ind[j+3] = e;

          i0 = i1;
        }

        // 0 1 2 (2,0) (0,1) (1,2)
        //    1
        //   4 5
        //  0 3 2
        di[0] = ind[0]; di[1] = ind[4]; di[0] = ind[3];
        di[3] = ind[4]; di[4] = ind[1]; di[5] = ind[5];
        di[6] = ind[3]; di[7] = ind[5]; di[8] = ind[2];
        di[9] = ind[4]; di[10] = ind[5]; di[11] = ind[3];
        di += 12;
      }

      for (unsigned i = 0; i != edges.size(); ++i) {
        uint64_t key = edges.key(i);
        unsigned i0 = (unsigned)(key >> 32);
        unsigned i1 = (unsigned)key;
        unsigned idx = edges.value(i);
        //blend(idx, i0, i1, 0.5f);
      }

      vertices->unlock();
      src->get_vertices()->unlock_read_only();

      set_vertices(vertices);
      set_num_vertices(num_dest_vertices);
    }

    void visit(visitor &v) {
      mesh::visit(v);
      v.visit(src, atom_src);
      v.visit(view_pos, atom_view_pos);
    }
  };
}
