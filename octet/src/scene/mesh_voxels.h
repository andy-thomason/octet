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
      for (int z = 0; z != dim; ++z) {
        for (int y = 0; y != dim; ++y) {
          uint32_t p00 = opaque[z*dim+y];
          add_lefts( p00 & ~(p00 << 1), y, z );
          add_rights( p00 & ~(p00 >> 1), y, z );
        }
      }

      for (int z = 0; z != dim; ++z) {
        add_bottoms( opaque[z*dim+0], 0, z );
        for (int y = 0; y != dim-1; ++y) {
          uint32_t p00 = opaque[z*dim+y];
          uint32_t p01 = opaque[z*dim+(y+1)];
          add_bottoms( p01 & ~p00, y, z );
          add_tops( p00 & ~p01, y, z );
        }
        add_tops( opaque[z*dim+(dim-1)], dim-1, z );
      }

      for (int y = 0; y != dim; ++y) {
        add_backs( opaque[0*dim+y], y, 0 );
        for (int z = 0; z != dim-1; ++z) {
          uint32_t p00 = opaque[z*dim+y];
          uint32_t p10 = opaque[(z+1)*dim+y];
          add_backs( p10 & ~p00, y, z );
          add_fronts( p00 & ~p10, y, z );
        }
        add_fronts( opaque[(dim-1)*dim+y], y, dim-1 );
      }
    }
  };

  class mesh_voxels : public mesh {
    enum { dim = 32 };
    uint32_t opaque[dim*dim];

    class face_counter {
    public:
      unsigned num_faces;
      face_counter() { num_faces = 0; }
      void add_lefts(uint32_t v, int, int) { num_faces += pop_count(v); }
      void add_rights(uint32_t v, int, int) { num_faces += pop_count(v); }
      void add_tops(uint32_t v, int, int) { num_faces += pop_count(v); }
      void add_bottoms(uint32_t v, int, int) { num_faces += pop_count(v); }
      void add_fronts(uint32_t v, int, int) { num_faces += pop_count(v); }
      void add_backs(uint32_t v, int, int) { num_faces += pop_count(v); }
    };

    class face_adder {
    public:
      vertex *vtx;
      uint32_t *idx;
      vec3 center;
      vec3 scale;
      vec3 dx;
      vec3 dy;
      vec3 dz;
      unsigned num_faces;

      face_adder() { num_faces = 0; }

      void add_faces(uint32_t v, const vec3 &base, const vec3 &du, const vec3 &dv, const vec3p &normal) {
        if (!v) return;

        for (int i = 0; i != 32; ++i) {
          if ((v >> i) & 1) {
            vec3 pos = base + (float)(i) * dx;
            vtx->pos = pos; vtx->normal = normal; vtx->uv = vec2p(0, 0); vtx++;
            vtx->pos = pos + du; vtx->normal = normal; vtx->uv = vec2p(1, 0); vtx++;
            vtx->pos = pos + du + dv; vtx->normal = normal; vtx->uv = vec2p(1, 1); vtx++;
            vtx->pos = pos + dv; vtx->normal = normal; vtx->uv = vec2p(0, 1); vtx++;
            idx[0] = num_faces*4 + 0;
            idx[1] = num_faces*4 + 1;
            idx[2] = num_faces*4 + 3;
            idx += 3;
            idx[0] = num_faces*4 + 1;
            idx[1] = num_faces*4 + 2;
            idx[2] = num_faces*4 + 3;
            idx += 3;
            num_faces++;
          }
        }
      }

      void add_lefts(uint32_t v, int y, int z) {
        add_faces(
          v,
          center + vec3((float)(-dim/2), (float)(y-dim/2), (float)(z-dim/2))*scale,
          dy, dz, vec3p(-1, 0, 0)
        );
      }
      void add_rights(uint32_t v, int y, int z) {
        add_faces(
          v,
          center + vec3((float)(-dim/2+1), (float)(y-dim/2+1), (float)(z-dim/2+1))*scale,
          -dy, -dz, vec3p(1, 0, 0)
        );
      }
      void add_bottoms(uint32_t v, int y, int z) {
        add_faces(
          v,
          center + vec3((float)(-dim/2), (float)(y-dim/2+1), (float)(z-dim/2))*scale,
          dx, dz, vec3p(0, -1, 0)
        );
      }
      void add_tops(uint32_t v, int y, int z) {
        add_faces(
          v,
          center + vec3((float)(-dim/2+1), (float)(y-dim/2+1), (float)(z-dim/2+1))*scale,
          -dx, -dz, vec3p(0, 1, 0)
        );
      }
      void add_backs(uint32_t v, int y, int z) {
        add_faces(
          v,
          center + vec3((float)(-dim/2), (float)(y-dim/2), (float)(z-dim/2+1))*scale,
          dx, dy, vec3p(0, 0, -1)
        );
      }
      void add_fronts(uint32_t v, int y, int z) {
        add_faces(
          v,
          center + vec3((float)(-dim/2+1), (float)(y-dim/2+1), (float)(z-dim/2+1))*scale,
          -dx, -dy, vec3p(0, 0, 1)
        );
      }
    };

  public:
    RESOURCE_META(mesh_voxels)

    mesh_voxels(const vec3 &size = vec3(1, 1, 1)) {
      memset(opaque, 0, sizeof(opaque));
      set_default_attributes();
      set_aabb(aabb(vec3(0, 0, 0), size));
      update();
    }

    void update() {
      //opaque[16*32+16] = 0x10000;
      //opaque[16*32+16] = 0x18000;
      //opaque[16*32+15] = 0x18000;
      //opaque[15*32+16] = 0x18000;
      //opaque[15*32+15] = 0x18000;
      //opaque[31*32+31] = 0x80000000;
      for (int z = 0; z != dim; ++z) {
        for (int y = 0; y != dim; ++y) {
          for (int x = 0; x != dim; ++x) {
            int d2 =
              (x - dim/2) * (x - dim/2) + 
              (y - dim/2) * (y - dim/2) + 
              (z - dim/2) * (z - dim/2)
            ;
            if (d2 < 15*15) {
              opaque[z*dim+y] |= 1<<x;
            }
          }
        }
      }

      for (int z = 0; z != dim; ++z) {
        for (int y = 0; y != dim; ++y) {
          for (int x = 0; x != dim; ++x) {
            int d2 =
              (x - dim/2) * (x - dim/2) + 
              (y - dim/2) * (y - dim/2)
            ;
            if (d2 < 7*7) {
              opaque[z*dim+y] &= ~(1<<x);
            }
          }
        }
      }

      mesh_iterate_faces<face_counter, dim> count;
      count.iterate(opaque);

      allocate(sizeof(vertex)*count.num_faces*4, sizeof(uint32_t)*count.num_faces*6);
      set_num_indices(count.num_faces*6);
      set_num_vertices(count.num_faces*4);

      mesh_iterate_faces<face_adder, dim> add;
      add.vtx = (vertex *)get_vertices()->lock();
      add.idx = (uint32_t *)get_indices()->lock();
      add.center = get_aabb().get_center();
      add.scale = get_aabb().get_half_extent()*(2.0f/dim);
      add.dx = vec3(1, 0, 0) * add.scale;
      add.dy = vec3(0, 1, 0) * add.scale;
      add.dz = vec3(0, 0, 1) * add.scale;
      add.iterate(opaque);

      assert(count.num_faces == add.num_faces);

      get_vertices()->unlock();
      get_indices()->unlock();
      dump(app_utils::log("voxels\n"));
    }

    void visit(visitor &v) {
      mesh::visit(v);
    }
  };
}
