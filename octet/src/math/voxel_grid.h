////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Axis aligned bounding box
//

namespace octet { namespace math {
  /// voxel grid
  template <class elem_t, class elem_traits_t> class voxel_grid {
    aabb bb;
    dynarray<elem_t> elems;
    dynarray<uint32_t> vertices_exist;
    dynarray<uint32_t> opaque;
    ivec3 dim;
  public:
    /// Default constructor: 16x16x16
    voxel_grid(aabb_in bb=aabb(), ivec3_in dim=ivec3(0, 0, 0)) : bb(bb), dim(dim) {
      elems.resize(dim.x() * dim.y() * dim.z());
      opaque.resize( (dim.x() * dim.y() * dim.z() + 31 ) / 32 );
      vertices_exist.resize( ( (dim.x()+1) * (dim.y()+1) * (dim.z()+1) + 31 ) / 32 );
    }

    int get_is_opaque(int x, int y, int z) const {
      if ((unsigned)x >= (unsigned)dim.x() || (unsigned)y >= (unsigned)dim.y() || (unsigned)z >= (unsigned)dim.z()) {
        return 0;
      } else {
        size_t addr = x + ( y + dim.y() * z ) * dim.x();
        return (opaque[addr>>5] >> (addr & 0x1f)) & 1;
      }
    }

    const elem_t &get_elem(int x, int y, int z) const {
      return elems[x + (y + z * dim.y()) * dim.x()];
    }

	  template <class sink_t> void add_face(sink_t &sink, vec3_in delta, vec3_in offset, ivec3_in addr, ivec3_in du, ivec3_in dv) {
      int v0 = (int)sink.add_vertex(addr * delta + offset, vec3(1, 0, 0), vec3(0, 0, 0));
      int v1 = (int)sink.add_vertex((addr + du) * delta + offset, vec3(1, 0, 0), du);
      int v2 = (int)sink.add_vertex((addr + dv) * delta + offset, vec3(1, 0, 0), dv);
      int v3 = (int)sink.add_vertex((addr + du + dv) * delta + offset, vec3(1, 0, 0), du+dv);
      // 0 1
      // 2 3
      sink.add_triangle(v0, v1, v3);
      sink.add_triangle(v0, v3, v2);
    }


	  template <class sink_t> void get_geometry(sink_t &sink, int) {
      vec3 delta(bb.get_half_extent() * 2.0f / (vec3)dim);
      vec3 offset = bb.get_center() - bb.get_half_extent();
      memset(opaque.data(), 0, opaque.size() * sizeof(opaque[0]));

      for (int k = 0; k != dim.z(); ++k) {
        for (int j = 0; j != dim.y(); ++j) {
          for (int i = 0; i != dim.x(); ++i) {
            if (!elem_traits_t::is_transparent(get_elem(i, j, k))) {
              size_t addr = i + dim.y() * ( j + dim.z() * k );
              opaque[addr>>5] |= 1 << (addr & 0x1f);
            }
          }
        }
      }

      for (int k = 0; k != dim.z(); ++k) {
        for (int j = 0; j != dim.y(); ++j) {
          for (int i = 0; i != dim.x(); ++i) {
            if (get_is_opaque(i, j, k) && !get_is_opaque(i+1, j, k)) {
              add_face(sink, delta, offset, ivec3(i, j, k), ivec3(0, 1, 0), ivec3(0, 0, 1));
            }
          }
        }
      }
    }
  };
} }

