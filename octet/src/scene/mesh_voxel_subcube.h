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
    vec3 origin;
    vec3 dx;
    vec3 dy;
    vec3 dz;
    mesh::vertex *vtx;
    uint32_t *idx;
    float voxel_size;
    unsigned num_faces;

    face_adder() { num_faces = 0; }

    void add_faces(uint32_t v, vec3_in base, vec3_in du, vec3_in dv, const vec3p &normal) {
      unsigned idx_val = num_faces * 4;
      for (int i = 0; i < 32; v >>= 1, i++) {
        if ((v & 0xff) == 0) { v >>= 8; i += 8; }
        if ((v & 0x3) == 0) { v >>= 2; i += 2; }
        if (v & 1) {
          vec3 pos = base + (float)(i) * dx;
          vtx->pos = pos; vtx->normal = normal; vtx->uv = vec2p(0, 0); vtx++;
          vtx->pos = pos + du; vtx->normal = normal; vtx->uv = vec2p(1, 0); vtx++;
          vtx->pos = pos + du + dv; vtx->normal = normal; vtx->uv = vec2p(1, 1); vtx++;
          vtx->pos = pos + dv; vtx->normal = normal; vtx->uv = vec2p(0, 1); vtx++;
          idx[0] = idx_val + 0;
          idx[3] = idx[1] = idx_val + 1;
          idx[5] = idx[2] = idx_val + 3;
          idx[4] = idx_val + 2;
          idx += 6;
          num_faces++;
          idx_val += 4;
        }
      }
    }

    void add_lefts(uint32_t v, int y, int z) {
      if (v) add_faces(
        v,
        origin + vec3((float)(0), (float)(y), (float)(z))*voxel_size,
        dy, dz, vec3p(-1, 0, 0)
      );
    }
    void add_rights(uint32_t v, int y, int z) {
      if (v) add_faces(
        v,
        origin + vec3((float)(1), (float)(y+1), (float)(z+1))*voxel_size,
        -dy, -dz, vec3p(1, 0, 0)
      );
    }
    void add_bottoms(uint32_t v, int y, int z) {
      if (v) add_faces(
        v,
        origin + vec3((float)(0), (float)(y+1), (float)(z))*voxel_size,
        dx, dz, vec3p(0, -1, 0)
      );
    }
    void add_tops(uint32_t v, int y, int z) {
      if (v) add_faces(
        v,
        origin + vec3((float)(1), (float)(y+1), (float)(z+1))*voxel_size,
        -dx, -dz, vec3p(0, 1, 0)
      );
    }
    void add_backs(uint32_t v, int y, int z) {
      if (v) add_faces(
        v,
        origin + vec3((float)(0), (float)(y), (float)(z+1))*voxel_size,
        dx, dy, vec3p(0, 0, -1)
      );
    }
    void add_fronts(uint32_t v, int y, int z) {
      if (v) add_faces(
        v,
        origin + vec3((float)(1), (float)(y+1), (float)(z+1))*voxel_size,
        -dx, -dy, vec3p(0, 0, 1)
      );
    }
  };

  class mesh_voxel_subcube : public resource {
    enum { dim = 32 };

    uint32_t opaque[dim*dim]; // 32x32x32

    // lods
    uint32_t some_opaque1[dim*dim/8]; // 16x16x16 = 16x8x2x16
    uint32_t all_opaque1[dim*dim/8];
    uint32_t some_opaque2[dim*dim/8/8]; // 8x8x8 = 8x2x4x8
    uint32_t all_opaque2[dim*dim/8/8];
    uint32_t some_opaque3[dim*dim/8/8/8]; // 4x4x4 = 1x2x32
    uint32_t all_opaque3[dim*dim/8/8/8];
    uint8_t some_opaque4;
    uint8_t all_opaque4;

    void update_one_lod(const uint32_t *some_src, const uint32_t *all_src, uint32_t *some, uint32_t *all, unsigned zmax, unsigned stride, uint32_t mask, int shift) {
      for (int z = 0; z != zmax; z += 2) {
        for (int y = 0; y != zmax; y += 2) {
          unsigned some0 =
            some_src[(z+0)*stride+(y+0)] |
            some_src[(z+0)*stride+(y+1)] |
            some_src[(z+1)*stride+(y+0)] |
            some_src[(z+1)*stride+(y+1)]
          ;
          unsigned some2 =
            some_src[(z+0)*stride+(y+zmax)] |
            some_src[(z+0)*stride+(y+zmax+1)] |
            some_src[(z+1)*stride+(y+zmax)] |
            some_src[(z+1)*stride+(y+zmax+1)]
          ;
          *some++ =
            (( some0 & mask) << 0 ) |
            (( some0 >> shift ) & mask) |
            (( some2 & mask ) << shift) |
            ( some2 & (mask << shift) )
          ;
          // rlrlrlrl -> rrllrrll -> rrrrllll
          unsigned all0 =
            all_src[(z+0)*stride+(y+0)] &
            all_src[(z+0)*stride+(y+1)] &
            all_src[(z+1)*stride+(y+0)] &
            all_src[(z+1)*stride+(y+1)]
          ;
          unsigned all2 =
            all_src[(z+0)*stride+(y+zmax)] &
            all_src[(z+0)*stride+(y+zmax+1)] &
            all_src[(z+1)*stride+(y+zmax)] &
            all_src[(z+1)*stride+(y+zmax+1)]
          ;
          *all++ =
            (( all0 & mask) << 0 ) &
            (( all0 >> shift ) & mask) &
            (( all2 & mask ) << shift) &
            ( all2 & (mask << shift) )
          ;
        }
      }
    }


  public:
    RESOURCE_META(mesh_voxel_subcube)

    mesh_voxel_subcube() {
      memset(opaque, 0, sizeof(opaque));
      update_lod();
    }

    void update_lod() {
      return;
      update_one_lod(opaque, opaque, some_opaque1, all_opaque1, dim, dim, 0x55555555, 1);
      update_one_lod(some_opaque1, all_opaque1, some_opaque2, all_opaque2, dim/4, dim/8, 0x33333333, 2);
      update_one_lod(some_opaque2, all_opaque2, some_opaque3, all_opaque3, dim/4/4, dim/8/8, 0x0f0f0f0f, 4);
      uint32_t some =
        (( some_opaque3[0] & 0x00ff00ff) << 0 ) |
        (( some_opaque3[0] >> 8 ) & 0x00ff00ff) |
        (( some_opaque3[1] & 0x00ff00ff ) << 8) |
        ( some_opaque3[1] & (0x00ff00ff << 8) )
      ;
      some = some & (some >> 16);
      some = some & (some >> 8);
      some_opaque4 = some;

      FILE *pf = fopen("/tmp/3.txt", "wb");
      for (int z = 0; z != dim/2; z++) {
        for (int y = 0; y != dim/4; y++) {
          fprintf(pf, "%08x ", some_opaque1[z*(dim/8)+y]);
        }
        fprintf(pf, "\n");
      }
      fprintf(pf, "\n");

      for (int z = 0; z != dim/4; z++) {
        for (int y = 0; y != dim/16; y++) {
          fprintf(pf, "%08x ", some_opaque2[z*(dim/8/8)+y]);
        }
        fprintf(pf, "\n");
      }
      fprintf(pf, "\n");

      for (int z = 0; z != dim/8; z++) {
        for (int y = 0; y != dim/64; y++) {
          fprintf(pf, "%08x ", some_opaque2[z*(dim/8/8)+y]);
        }
        fprintf(pf, "\n");
      }
      fprintf(pf, "\n");

      fprintf(pf, "%02x\n", some_opaque4);
      fclose(pf);
    }

    void count_faces(mesh_iterate_faces<face_counter, dim> &count) {
      count.iterate(opaque);
    }

    void add_faces(mesh_iterate_faces<face_adder, dim> &add) {
      add.iterate(opaque);
    }

    template <class set> void add_voxels(mat4t_in voxelToWorld, const set &set_in) {
      for (int z = 0; z != dim; ++z) {
        for (int y = 0; y != dim; ++y) {
          for (int x = 0; x != dim; ++x) {
            vec3 txyz = vec3(x, y, z) * voxelToWorld;
            if (set_in.intersects(txyz)) {
              opaque[z*dim+y] |= 1 << x;
            }
          }
        }
      }
    }

    void dump(FILE *fp) {
      for (int z = 0; z != dim; ++z) {
        fprintf(fp, "z=%2d", z);
        for (int y = 0; y != dim; ++y) {
          fprintf(fp, " %08x", opaque[z*dim+y]);
        }
        fprintf(fp, "\n");
      }
    }
  };
}
