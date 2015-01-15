////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Mesh smooth modifier. Work in progress.
//

namespace octet { namespace scene {
  class smooth : public mesh {
    // source mesh. Provides underlying geometry.
    ref<mesh> src;

    // view dependent parameters
    vec3 view_pos;
    vec3 view_dir;

    // working params
    unsigned num_dest_vertices;

    dynarray<uint32_t> dest_indices;
    hash_map<uint64_t, unsigned> edges;
    dynarray<uint8_t> dest_vertices;
    unsigned pos_offset;
    unsigned normal_offset;
    unsigned uv_offset;
    int depth;

    bool split_edge(uint8_t *dest, const uint8_t *src0, const uint8_t *src1, unsigned stride) {
      const vec3p &pos0 = (const vec3p&)src0[pos_offset];
      const vec3p &pos1 = (const vec3p&)src1[pos_offset];
      const vec3p &n0 = (const vec3p&)src0[normal_offset];
      const vec3p &n1 = (const vec3p&)src1[normal_offset];
      //const vec3 &uv0 = (const vec3&)src0[uv_offset];
      //const vec3 &uv1 = (const vec3&)src1[uv_offset];

      if (is_smooth(n0, n1, depth)) {
        return false;
      }

      // Catmul-Rom spline
      vec3 diff = (vec3)pos1 - (vec3)pos0;
      vec3 t0 = cross(cross(n0, diff), n0); // bezier tangent * 3
      vec3 t1 = cross(cross(n1, diff), n1); // bezier tangent * 3

      for (unsigned i = 0; i < stride/sizeof(float); ++i) {
        ((float*)dest)[i] = (((float*)src0)[i] + ((float*)src1)[i]) * 0.5f;
      }

      vec3p &pos = (vec3p&)dest[pos_offset];
      vec3p &normal = (vec3p&)dest[normal_offset];

      pos = ((vec3)pos0 + (vec3)pos1) * 0.5f + (t0 - t1) * 0.125; // (3/8)/3 = 1/8
      normal = normalize(normal);

      return true;
    }

    unsigned add_edge(int i0, int i1) {
      if (i0 > i1) { std::swap(i0, i1); }

      unsigned &e = edges[((uint64_t)i1 << 32) | i0];

      if (e == ~0) return 0;

      if (e != 0) return e;

      unsigned stride = get_stride();

      dest_vertices.resize((num_dest_vertices + 1) * stride);

      log("%*se%d %d %d\n", depth*2, "", num_dest_vertices, i0, i1);
      bool split = split_edge(
        &dest_vertices[num_dest_vertices * stride],
        &dest_vertices[i0 * stride],
        &dest_vertices[i1 * stride],
        stride
      );

      if (split) {
        e = num_dest_vertices++;
        return e;
      } else {
        // unable to split. Don't test again.
        e = ~0;
        return 0;
      }
    }

    void add_triangle(unsigned i0, unsigned i1, unsigned i2) {
      unsigned i3 = add_edge(i0, i1);
      unsigned i4 = add_edge(i1, i2);
      unsigned i5 = add_edge(i2, i0);

      depth++;

      log("%*sat: %d %d %d %d %d %d\n", depth*2, "", i0, i1, i2, i3, i4, i5);

      switch( (i3 != 0) + (i4 != 0)*2 + (i5 != 0)*4 ) {
        case 0: {
          unsigned disize = dest_indices.size();
          dest_indices.resize(disize+3);
          dest_indices[disize+0] = i0;
          dest_indices[disize+1] = i1;
          dest_indices[disize+2] = i2;
        } break;
        case 1: {
          //    1
          //   3 
          //  0   2
          add_triangle(i3, i1, i2);
          add_triangle(i3, i2, i0);
        } break;
        case 2: {
          //    1
          //     4 
          //  0   2
          add_triangle(i4, i0, i1);
          add_triangle(i4, i2, i0);
        } break;
        case 3: {
          //    1
          //   3 4
          //  0   2
          add_triangle(i3, i1, i4);
          add_triangle(i3, i4, i0);
          add_triangle(i4, i2, i0);
        } break;
        case 4: {
          //    1
          //      
          //  0 5 2
          add_triangle(i5, i0, i1);
          add_triangle(i5, i1, i2);
        } break;
        case 5: {
          //    1
          //   3  
          //  0 5 2
          add_triangle(i5, i0, i3);
          add_triangle(i5, i3, i2);
          add_triangle(i3, i1, i2);
        } break;
        case 6: {
          //    1
          //     4
          //  0 5 2
          add_triangle(i4, i2, i5);
          add_triangle(i5, i0, i4);
          add_triangle(i4, i0, i1);
        } break;
        case 7: {
          //    1
          //   3 4
          //  0 5 2
          add_triangle(i1, i4, i3);
          add_triangle(i3, i4, i5);
          add_triangle(i3, i5, i0);
          add_triangle(i4, i2, i5);
        } break;
      }
      depth--;
    }
          
  public:
    RESOURCE_META(smooth)

    smooth(mesh *src=0) {
      this->src = src;
      view_pos = vec3(0, 0, 0);
      update();
    }

    void update() {
      if (!src) return;
      if (src->get_mode() != GL_TRIANGLES) return;
      if (src->get_index_type() != GL_UNSIGNED_INT) return;

      *(mesh*)this = *(mesh*)src;

      unsigned pos_slot = get_slot(attribute_pos);
      unsigned normal_slot = get_slot(attribute_normal);
      unsigned uv_slot = get_slot(attribute_uv);

      // needs pos, normal and uv map
      if (pos_slot == ~0 || normal_slot == ~0 || uv_slot == ~0) {
        return;
      }

      pos_offset = get_offset(pos_slot);
      normal_offset = get_offset(normal_slot);
      uv_offset = get_offset(uv_slot);

      dest_indices.reserve(get_num_indices() * 4);
      dest_vertices.reserve(get_num_vertices() * get_stride() * 4);
      dest_vertices.resize(get_num_vertices() * get_stride());

      // copy vertices for existing triangles
      const void *sp = src->get_vertices()->lock_read_only();
      memcpy(&dest_vertices[0], sp, get_num_vertices() * get_stride());
      src->get_vertices()->unlock_read_only();
      num_dest_vertices = get_num_vertices();

      const void *sip = src->get_indices()->lock_read_only();
      depth = 0;
      for (unsigned i = 0; i+2 < get_num_indices(); i += 3) {
        uint32_t *tp = ((uint32_t*)sip) + i;
        add_triangle(tp[0], tp[1], tp[2]);
      }
      src->get_indices()->unlock_read_only();

      unsigned isize = dest_indices.size() * sizeof(dest_indices[0]);
      unsigned vsize = dest_vertices.size() * sizeof(dest_vertices[0]);
      gl_resource *indices = new gl_resource(GL_ELEMENT_ARRAY_BUFFER, isize);
      gl_resource *vertices = new gl_resource(GL_ARRAY_BUFFER, vsize);
      indices->assign(&dest_indices[0], 0, isize);
      vertices->assign(&dest_vertices[0], 0, vsize);

      set_indices(indices);
      set_vertices(vertices);
      set_num_vertices(num_dest_vertices);
      set_num_indices(dest_indices.size());

      dump(log("dump"));
    }

    void visit(visitor &v) {
      mesh::visit(v);
      v.visit(src, atom_src);
      v.visit(view_pos, atom_view_pos);
    }

    virtual bool is_smooth(const vec3 &n0, const vec3 &n1, int depth) {
      return true;
      float dotp = dot(n0, n1);
      log("%*s  dotp=%f\n", depth*2, "", dotp);
      return (dotp >= 0.9f) || depth >= 4;
    }
  };
}}
