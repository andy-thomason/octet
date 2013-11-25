////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Mesh smooth modifier. Work in progress.
//

namespace octet {
  template <class interface_t, int dim> class mesh_iterate_faces : public interface_t {
  public:
    void iterate(const uint32_t *opaque) {
      for (unsigned j = 0; j != dim; ++j) {
        for (unsigned i = 0; i != dim; ++i) {
          uint32_t p00 = opaque[j*dim+i];
          add_lefts( ~(p00 >> 1) & p00, i, j );
          add_rights( p00 & ~(p00 << 1), i, j );
        }
      }

      for (unsigned j = 0; j != dim; ++j) {
        add_bottoms( opaque[j*dim+0], j, 0 );
        add_tops( opaque[j*dim+(dim-1)], j, dim-1 );
        for (unsigned i = 0; i != dim-1; ++i) {
          uint32_t p00 = opaque[j*dim+i];
          uint32_t p01 = opaque[j*dim+(i+1)];
          add_bottoms( p00 & ~p01, i, j );
          add_tops( p01 & ~p00, i, j );
        }
      }

      for (unsigned i = 0; i != dim-1; ++i) {
        add_fronts( opaque[0*dim+i], 0, i );
        add_backs( opaque[(dim-1)*dim+i], dim-1, i );
        for (unsigned j = 0; j != dim-1; ++j) {
          uint32_t p00 = opaque[j*dim+i];
          uint32_t p10 = opaque[(j+1)*dim+i];
          add_fronts( p00 & ~p10, i, j );
          add_backs( p10 & ~p00, i, j );
        }
      }
    }
  };

  class mesh_voxels : public mesh_box {
    enum { dim = 32 };
    uint32_t opaque[dim*dim];

    class face_counter {
    public:
      unsigned num_faces;
      face_counter() { num_faces = 0; }
      void add_lefts(uint32_t v, unsigned, unsigned) { num_faces += pop_count(v); }
      void add_rights(uint32_t v, unsigned, unsigned) { num_faces += pop_count(v); }
      void add_tops(uint32_t v, unsigned, unsigned) { num_faces += pop_count(v); }
      void add_bottoms(uint32_t v, unsigned, unsigned) { num_faces += pop_count(v); }
      void add_fronts(uint32_t v, unsigned, unsigned) { num_faces += pop_count(v); }
      void add_backs(uint32_t v, unsigned, unsigned) { num_faces += pop_count(v); }
    };

    class face_adder {
    public:
      vertex *vtx;
      uint32_t *idx;
      vec3 center;
      vec3 scale;
      unsigned num_faces;

      face_adder() { num_faces = 0; }

      void add_faces(uint32_t v, const vec3 &base, const vec3 &du, const vec3 &dv, const vec3 &dw, const vec3p &normal) {
        for (int i = 0; i != 32; ++i) {
          if ((v >> i) & 1) {
            vec3 pos = base + (float)(i-16) * dw;
            vtx->pos = pos; vtx->normal = normal; vtx->uv = vec2p(0, 0); vtx++;
            vtx->pos = pos + du; vtx->normal = normal; vtx->uv = vec2p(1, 0); vtx++;
            vtx->pos = pos + du + dv; vtx->normal = normal; vtx->uv = vec2p(1, 1); vtx++;
            vtx->pos = pos + dv; vtx->normal = normal; vtx->uv = vec2p(0, 1); vtx++;
          }
          num_faces++;
        }
      }

      void add_lefts(uint32_t v, unsigned y, unsigned z) {
        add_faces(
          v,
          center + vec3((float)-dim/2, (float)y, (float)z)*scale,
          vec3(0, 1, 0)*scale,
          vec3(0, 0, 1)*scale,
          vec3(1, 0, 0)*scale,
          vec3p(-1, 0, 0)
        );
      }
      void add_rights(uint32_t v, unsigned y, unsigned z) {
        add_faces(
          v,
          center + vec3((float)-dim/2+1, (float)y, (float)z)*scale,
          vec3(0, 1, 0)*scale,
          vec3(0, 0, 1)*scale,
          vec3(1, 0, 0)*scale,
          vec3p(1, 0, 0)
        );
      }
      void add_bottoms(uint32_t v, unsigned y, unsigned z) {
        add_faces(
          v,
          center + vec3((float)z, (float)-dim/2, (float)y)*scale,
          vec3(1, 0, 0)*scale,
          vec3(0, 0, 1)*scale,
          vec3(0, 1, 0)*scale,
          vec3p(0, -1, 0)
        );
      }
      void add_tops(uint32_t v, unsigned y, unsigned z) {
        add_faces(
          v,
          center + vec3((float)z, (float)-dim/2+1, (float)y)*scale,
          vec3(1, 0, 0)*scale,
          vec3(0, 0, 1)*scale,
          vec3(0, 1, 0)*scale,
          vec3p(0, 1, 0)
        );
      }
      void add_fronts(uint32_t v, unsigned y, unsigned z) {
        add_faces(
          v,
          center + vec3((float)y, (float)z, (float)-dim/2)*scale,
          vec3(1, 0, 0)*scale,
          vec3(0, 1, 0)*scale,
          vec3(0, 0, 1)*scale,
          vec3p(0, 0, 1)
        );
      }
      void add_backs(uint32_t v, unsigned y, unsigned z) {
        add_faces(
          v,
          center + vec3((float)y, (float)z, (float)-dim/2+1)*scale,
          vec3(1, 0, 0)*scale,
          vec3(0, 1, 0)*scale,
          vec3(0, 0, 1)*scale,
          vec3p(0, 0, -1)
        );
      }
    };
  public:
    RESOURCE_META(mesh_voxels)

    mesh_voxels() {
      init(aabb(vec3(0, 0, 0), vec3(1, 1, 1)));
    }

    mesh_voxels(const vec3 &size) {
      init(aabb(vec3(0, 0, 0), size));
    }

    void update() {
      mesh_iterate_faces<face_counter, dim> count;
      count.iterate(opaque);

      allocate(sizeof(vertex)*count.num_faces*4, sizeof(uint32_t)*count.num_faces*6);

      mesh_iterate_faces<face_adder, dim> add;
      add.vtx = (vertex *)get_vertices()->lock();
      add.idx = (uint32_t *)get_indices()->lock();
      add.center = get_aabb().get_center();
      add.scale = get_aabb().get_half_extent()*(2.0f/dim);
      add.iterate(opaque);

      get_vertices()->unlock();
      get_indices()->unlock();
      set_num_indices(6 * 6);
      set_num_vertices(4 * 6);
      //dump(app_utils::log("box\n"));
    }

    void visit(visitor &v) {
      mesh_box::visit(v);
    }
  };
}
