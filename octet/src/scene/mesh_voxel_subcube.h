////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Voxel subcube for the mesh_voxel component
//

namespace octet { namespace scene {
  template <class interface_t, int dim> class mesh_iterate_faces : public interface_t {
  public:
    void iterate(const uint32_t *opaque) {
      for (int z = 0; z != dim; ++z) {
        for (int y = 0; y != dim; ++y) {
          uint32_t p00 = opaque[z*dim+y];
          interface_t::add_lefts( p00 & ~(p00 << 1), y, z );
          interface_t::add_rights( p00 & ~(p00 >> 1), y, z );
        }
      }

      for (int z = 0; z != dim; ++z) {
        interface_t::add_bottoms( opaque[z*dim+0], 0, z );
        for (int y = 0; y != dim-1; ++y) {
          uint32_t p00 = opaque[z*dim+y];
          uint32_t p01 = opaque[z*dim+(y+1)];
          interface_t::add_bottoms( p01 & ~p00, y, z );
          interface_t::add_tops( p00 & ~p01, y, z );
        }
        interface_t::add_tops( opaque[z*dim+(dim-1)], dim-1, z );
      }

      for (int y = 0; y != dim; ++y) {
        interface_t::add_backs( opaque[0*dim+y], y, 0 );
        for (int z = 0; z != dim-1; ++z) {
          uint32_t p00 = opaque[z*dim+y];
          uint32_t p10 = opaque[(z+1)*dim+y];
          interface_t::add_backs( p10 & ~p00, y, z );
          interface_t::add_fronts( p00 & ~p10, y, z );
        }
        interface_t::add_fronts( opaque[(dim-1)*dim+y], y, dim-1 );
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
        dy, dz, vec3p(-1.0f, 0.0f, 0.0f)
      );
    }
    void add_rights(uint32_t v, int y, int z) {
      if (v) add_faces(
        v,
        origin + vec3((float)(1), (float)(y+1), (float)(z+1))*voxel_size,
        -dy, -dz, vec3p(1.0f, 0.0f, 0.0f)
      );
    }
    void add_bottoms(uint32_t v, int y, int z) {
      if (v) add_faces(
        v,
        origin + vec3((float)(0), (float)(y+1), (float)(z))*voxel_size,
        dx, dz, vec3p(0.0f, -1.0f, 0.0f)
      );
    }
    void add_tops(uint32_t v, int y, int z) {
      if (v) add_faces(
        v,
        origin + vec3((float)(1), (float)(y+1), (float)(z+1))*voxel_size,
        -dx, -dz, vec3p(0.0f, 1.0f, 0.0f)
      );
    }
    void add_backs(uint32_t v, int y, int z) {
      if (v) add_faces(
        v,
        origin + vec3((float)(0), (float)(y), (float)(z+1))*voxel_size,
        dx, dy, vec3p(0.0f, 0.0f, -1.0f)
      );
    }
    void add_fronts(uint32_t v, int y, int z) {
      if (v) add_faces(
        v,
        origin + vec3((float)(1), (float)(y+1), (float)(z+1))*voxel_size,
        -dx, -dy, vec3p(0.0f, 0.0f, 1.0f)
      );
    }
  };

  /// experimental voxel world subcube class.
  class mesh_voxel_subcube : public resource {
    enum {
      // dimension of subcube
      dim = 32,
      // lod offsets
      d16 = 0,
      d8 = d16 + 32*32/8,
      d4 = d8 + 32*32/8/8,
      d2 = d4 + 32*32/8/8/8,
      num_lod = d2 + 1
    };

    uint32_t opaque[dim*dim];
    uint32_t any_opaque[num_lod];
    uint32_t all_opaque[num_lod];


    static unsigned off32(unsigned x, unsigned y, unsigned z) { return z*32+y; }
    static unsigned off16(unsigned x, unsigned y, unsigned z) { return d16+z*8+y/2; }
    static unsigned off8(unsigned x, unsigned y, unsigned z) { return d8+z*2+y/4; }
    static unsigned off4(unsigned x, unsigned y, unsigned z) { return d4+z/2; }
    static unsigned off2(unsigned x, unsigned y, unsigned z) { return d2; }

    static unsigned shift32(unsigned x, unsigned y, unsigned z) { return x; }
    static unsigned shift16(unsigned x, unsigned y, unsigned z) { return x+(y&1)*16; }
    static unsigned shift8(unsigned x, unsigned y, unsigned z) { return x+(y&3)*8; }
    static unsigned shift4(unsigned x, unsigned y, unsigned z) { return x+y*4+(z&1)*16; }
    static unsigned shift2(unsigned x, unsigned y, unsigned z) { return x+y*2+z*4; }

    unsigned get32(uint32_t *src, unsigned x, unsigned y, unsigned z) { return (opaque[z*32+y] >> x) & 1; }
    static unsigned get16(uint32_t *src, unsigned x, unsigned y, unsigned z) { return (src[off16(x,y,z)] >> shift16(x,y,z)) & 1; }
    static unsigned get8(uint32_t *src, unsigned x, unsigned y, unsigned z) { return (src[off8(x,y,z)] >> shift8(x,y,z)) & 1; }
    static unsigned get4(uint32_t *src, unsigned x, unsigned y, unsigned z) { return (src[off4(x,y,z)] >> shift4(x,y,z)) & 1; }
    static unsigned get2(uint32_t *src, unsigned x, unsigned y, unsigned z) { return (src[off2(x,y,z)] >> shift2(x,y,z)) & 1; }

    static unsigned rot(unsigned x, unsigned amount) { return (x << amount) | (x >> (32-amount)); }

    // abcd -> acbd
    static unsigned cswap(unsigned x) { return (x & 0xff0000ff) | ( x >> 8 ) & 0xff00 | ( x << 8 ) & 0xff0000; }

  public:
    RESOURCE_META(mesh_voxel_subcube)

    mesh_voxel_subcube() {
      memset(opaque, 0, sizeof(opaque));
      //update_lod();
    }

    void update_lod() {
      uint32_t *any = any_opaque + d16;
      uint32_t *all = all_opaque + d16;

      // make 16x16x16 bits = 16x8x32
      uint32_t *any_src = opaque;
      uint32_t *all_src = opaque;
      for (int z = 0; z != 32; z += 2) {
        for (int y = 0; y != 32; y += 4) {
          unsigned any0 = any_src[z*32+y+0] | any_src[z*32+y+1] | any_src[z*32+y+32+0] | any_src[z*32+y+32+1];
          unsigned any1 = any_src[z*32+y+2] | any_src[z*32+y+3] | any_src[z*32+y+32+2] | any_src[z*32+y+32+3];
          *any++ = even_bits(any0|any0>>1) | (even_bits(any1|any1>>1) << 16);
          unsigned all0 = all_src[z*32+y+0] & all_src[z*32+y+1] & all_src[z*32+y+32+0] & all_src[z*32+y+32+1];
          unsigned all1 = all_src[z*32+y+2] & all_src[z*32+y+3] & all_src[z*32+y+32+2] & all_src[z*32+y+32+3];
          *all++ = even_bits(all0&all0>>1) | (even_bits(all1&all1>>1) << 16);
        }
      }
      assert(any - any_opaque == d8);

      // make 8x8x8 bits = 8x2x32
      any_src = any_opaque + d16;
      all_src = all_opaque + d16;
      for (int z = 0; z != 16; z += 2) {
        for (int y = 0; y != 8; y += 4) {
          unsigned any0 = any_src[z*8+y+0] | any_src[z*8+y+8+0];
          unsigned any1 = any_src[z*8+y+1] | any_src[z*8+y+8+1];
          unsigned any2 = any_src[z*8+y+2] | any_src[z*8+y+8+2];
          unsigned any3 = any_src[z*8+y+3] | any_src[z*8+y+8+3];
          unsigned anya = ((any0|any0>>16) & 0xffff) | ((any1|any1<<16) & 0xffff0000);
          unsigned anyb = ((any2|any2>>16) & 0xffff) | ((any3|any3<<16) & 0xffff0000);
          *any++ = even_bits(anya|anya>>1) | (even_bits(anyb|anyb>>1) << 16);
          unsigned all0 = all_src[z*8+y+0] & all_src[z*8+y+8+0];
          unsigned all1 = all_src[z*8+y+1] & all_src[z*8+y+8+1];
          unsigned all2 = all_src[z*8+y+2] & all_src[z*8+y+8+2];
          unsigned all3 = all_src[z*8+y+3] & all_src[z*8+y+8+3];
          unsigned alla = ((all0&all0>>16) & 0xffff) | ((all1&all1<<16) & 0xffff0000);
          unsigned allb = ((all2&all2>>16) & 0xffff) | ((all3&all3<<16) & 0xffff0000);
          *all++ = even_bits(alla&alla>>1) | (even_bits(allb&allb>>1) << 16);
        }
      }
      assert(any - any_opaque == d4);

      // make 4x4x4 bits = 2x32
      any_src = any_opaque + d8;
      all_src = all_opaque + d8;
      for (int z = 0; z != 8; z += 4) {
        unsigned any0 = any_src[z*2+0] | any_src[z*2+2];
        unsigned any1 = any_src[z*2+1] | any_src[z*2+3];
        unsigned any2 = any_src[z*2+4] | any_src[z*2+6];
        unsigned any3 = any_src[z*2+5] | any_src[z*2+7];
        unsigned anya = ((any0|any0>>8) & 0x00ff00ff) | ((any1|any1<<8) & 0xff00ff00);
        unsigned anyb = ((any2|any2>>8) & 0x00ff00ff) | ((any3|any3<<8) & 0xff00ff00);
        anya = cswap(anya);
        anyb = cswap(anyb);
        *any++ = even_bits(anya|anya>>1) | even_bits(anyb|anyb>>1) << 16;
        unsigned all0 = all_src[z*2+0] & all_src[z*2+2];
        unsigned all1 = all_src[z*2+1] & all_src[z*2+3];
        unsigned all2 = all_src[z*2+4] & all_src[z*2+6];
        unsigned all3 = all_src[z*2+5] & all_src[z*2+7];
        unsigned alla = ((all0&all0>>8) & 0x00ff00ff) | ((all1&all1<<8) & 0xff00ff00);
        unsigned allb = ((all2&all2>>8) & 0x00ff00ff) | ((all3&all3<<8) & 0xff00ff00);
        alla = cswap(alla);
        allb = cswap(allb);
        *all++ = even_bits(alla&alla>>1) | even_bits(allb&allb>>1) << 16;
      }
      assert(any - any_opaque == d2);

      any_src = any_opaque + d4;
      all_src = all_opaque + d4;
      {
        unsigned any0 = ((any_src[0] | any_src[0] >> 16) & 0xffff) | ((any_src[1] << 16 | any_src[1]) & 0xffff0000);
        unsigned anya = low_nibbles(any0|any0>>4);
        *any++ = even_bits(anya|anya>>1);
        unsigned all0 = ((all_src[0] & all_src[0] >> 16) & 0xffff) | ((all_src[1] << 16 & all_src[1]) & 0xffff0000);
        unsigned alla = low_nibbles(all0&all0>>4);
        *all++ = even_bits(alla&alla>>1);
      }
      assert(any - any_opaque == num_lod);
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

    void dump_lod(FILE *fp, const char *label, uint32_t *src) {
      fprintf(fp, "LOD %s\n", label);
      for (int z = 0; z != 16; z++) {
        fprintf(fp, "z16=%2d ", z);
        for (int y = 0; y != 8; y++) {
          fprintf(fp, "%08x ", src[d16+z*8+y]);
        }
        fprintf(fp, "\n");
      }
      fprintf(fp, "\n");

      for (int z = 0; z != 8; z++) {
        fprintf(fp, "z8=%2d ", z);
        for (int y = 0; y != 2; y++) {
          fprintf(fp, "%08x ", src[d8+z*2+y]);
        }
        fprintf(fp, "\n");
      }
      fprintf(fp, "\n");

      for (int z = 0; z != 2; z++) {
        fprintf(fp, "z4=%2d ", z);
        fprintf(fp, "%08x ", src[d4+z]);
        fprintf(fp, "\n");
      }
      fprintf(fp, "\n");

      fprintf(fp, "z2= 0 %02x\n", src[d2]);
    }


    void dump(FILE *fp) {
      for (int z = 0; z != dim; ++z) {
        fprintf(fp, "z=%2d ", z);
        for (int y = 0; y != dim; ++y) {
          fprintf(fp, " %08x", opaque[z*dim+y]);
        }
        fprintf(fp, "\n");
      }
      dump_lod(fp, "any", any_opaque);
      dump_lod(fp, "all", all_opaque);
    }

    // unit test for update_lod function
    bool test_update_lod() {
      random r;
      for (int i = 0; i != 100; ++i) {
        memset(opaque, 0, sizeof(opaque));
        for (int z = 0; z != 32; z ++) {
          for (int y = 0; y != 32; y ++) {
            //unsigned density = z >= 16 ? (y >= 16 ? 0x10 : 0xfff0) : (y >= 16 ? 0x0 : 0x10000);
            for (int x = 0; x != 32; x ++) {
              unsigned density = x < 16 ? (y < 16 ? 0x10 : 0xfff0) : (y < 16 ? 0x0 : 0x10000);
              if (r.get0xffff() < density) {
                opaque[off32(x, y, z)] |= 1 << shift32(x, y, z);
              }
            }
          }
        }

        // do it the hard way!
        uint32_t any_test[num_lod];
        uint32_t all_test[num_lod];
        memset(any_test, 0, sizeof(any_test));
        memset(all_test, 0, sizeof(all_test));

        // this macro tries the LOD the slow way for comparison
        #define OCTET_VOXEL_LOD(ANY, ALL, A, B) \
          for (int z = 0; z != A; z += 2) { \
            for (int y = 0; y != A; y += 2) { \
              for (int x = 0; x != A; x += 2) { \
                unsigned any = \
                  get##A(ANY, x, y, z) | get##A(ANY, x+1, y, z) | get##A(ANY, x, y+1, z) | get##A(ANY, x+1, y+1, z) | \
                  get##A(ANY, x, y, z+1) | get##A(ANY, x+1, y, z+1) | get##A(ANY, x, y+1, z+1) | get##A(ANY, x+1, y+1, z+1) \
                ;\
                ANY[off##B(x/2, y/2, z/2)] |= any << shift##B(x/2, y/2, z/2); \
                /*log("%2d %2d %2d: %d %08x %d\n", x, y, z, any, ANY[off##B(x/2, y/2, z/2)], shift##B(x/2, y/2, z/2));*/ \
                unsigned all = \
                  get##A(ALL, x, y, z) & get##A(ALL, x+1, y, z) & get##A(ALL, x, y+1, z) & get##A(ALL, x+1, y+1, z) & \
                  get##A(ALL, x, y, z+1) & get##A(ALL, x+1, y, z+1) & get##A(ALL, x, y+1, z+1) & get##A(ALL, x+1, y+1, z+1) \
                ; \
                ALL[off##B(x/2, y/2, z/2)] |= all << shift##B(x/2, y/2, z/2); \
              } \
            } \
          }
        OCTET_VOXEL_LOD(any_test, all_test, 32, 16)
        OCTET_VOXEL_LOD(any_test, all_test, 16, 8)
        OCTET_VOXEL_LOD(any_test, all_test, 8, 4)
        OCTET_VOXEL_LOD(any_test, all_test, 4, 2)

        update_lod();

        if (
          memcmp(any_opaque, any_test, sizeof(any_test)) ||
          memcmp(all_opaque, all_test, sizeof(all_test))
        ) {
          FILE *fp = log("test failure\n");

          for (int z = 0; z != dim; ++z) {
            fprintf(fp, "z=%2d ", z);
            for (int y = 0; y != dim; ++y) {
              fprintf(fp, " %08x", opaque[z*dim+y]);
            }
            fprintf(fp, "\n");
          }

          dump_lod(fp, "test any", any_test);
          dump_lod(fp, "update_lod any", any_opaque);

          dump_lod(fp, "test all", all_test);
          dump_lod(fp, "update_lod all", all_opaque);
          return false;
        }
        //printf("%08x %08x %02x\n", any_opaque[d4], any_opaque[d4+1], any_opaque[d2]);
        //printf("%08x %08x %02x\n", all_opaque[d4], all_opaque[d4+1], all_opaque[d2]);
      }
      return true;
    }

    unsigned is_any(ivec3_in pos, int level) {
      switch(level) {
        case 0: return get32(opaque, pos.x(), pos.y(), pos.z());
        case 1: return get16(any_opaque, pos.x(), pos.y(), pos.z());
        case 2: return get8(any_opaque, pos.x(), pos.y(), pos.z());
        case 3: return get4(any_opaque, pos.x(), pos.y(), pos.z());
        case 4: return get2(any_opaque, pos.x(), pos.y(), pos.z());
        case 5: return any_opaque[d2] != 0;
        default: assert(0 && "only 0-5"); return 1;
      }
    }

    unsigned is_all(ivec3_in pos, int level) {
      switch(level) {
        case 0: return get32(opaque, pos.x(), pos.y(), pos.z());
        case 1: return get16(all_opaque, pos.x(), pos.y(), pos.z());
        case 2: return get8(all_opaque, pos.x(), pos.y(), pos.z());
        case 3: return get4(all_opaque, pos.x(), pos.y(), pos.z());
        case 4: return get2(all_opaque, pos.x(), pos.y(), pos.z());
        case 5: return all_opaque[d2] == 0xff;
        default: assert(0 && "only 0-5"); return 1;
      }
    }
  };
}}
