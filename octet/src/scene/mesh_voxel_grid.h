////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Mesh smooth modifier. Work in progress.
//

namespace octet { namespace scene {
  /// Box mesh. Generate triangles for an AABB.
  class mesh_voxel_grid : public mesh {
    struct uint8_traits_t {
      static bool is_transparent(uint8_t value) {
        return value == 0;
      }
    };
    voxel_grid<uint8_t, uint8_traits_t> shape;
    mat4t transform;

    void init(aabb_in size, ivec3 dim) {
      set_default_attributes();
      set_aabb(size);
      update();
    }

  public:
    RESOURCE_META(mesh_voxel_grid)

    /// Default constructor: 0x0x0
    mesh_voxel_grid() {
      init(aabb(), ivec3(0, 0, 0));
    }

    mesh_voxel_grid(aabb_in bb, ivec3_in dim) {
      init(bb, dim);
    }

    /// Generate mesh from parameters.
    virtual void update() {
      aabb aabb_ = get_aabb();
      mesh::set_shape<voxel_grid<uint8_t, uint8_traits_t>, mesh::vertex>(shape, transform, 1);
    }
  };
}}

