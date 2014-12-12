////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Mesh smooth modifier. Work in progress.
//

namespace octet { namespace scene {
  class mesh_terrain : public mesh {
  public:
    /// override this to generate terrain.
    /// note: this doesn't need to be a heightfield.
    struct geometry_source {
      virtual mesh::vertex vertex(vec3_in bb_min, vec3_in uv_min, vec3_in uv_delta, vec3_in pos) = 0;
    };

  private:
    ivec3 dimensions;
    geometry_source &source;

  public:

    /// unity-style terrain mesh
    mesh_terrain(vec3_in size, ivec3_in dimensions, geometry_source &source) : mesh(), dimensions(dimensions), source(source) {
      set_default_attributes();
      set_aabb(aabb(vec3(0, 0, 0), size));
      update();
    }

    // override the update function to draw different geometry.
    void update() {
      dynarray<mesh::vertex> vertices;
      dynarray<uint32_t> indices;

      vertices.reserve((dimensions.x()+1) * (dimensions.z()+1));

      vec3 dimf = (vec3)(dimensions);
      aabb bb = get_aabb();
      vec3 bb_min = bb.get_min();
      vec3 bb_delta = bb.get_half_extent() / dimf * 2.0f;
      vec3 uv_min = vec3(0);
      vec3 uv_delta = vec3(30.0f/dimf.x(), 30.0f/dimf.z(), 0);
      for (int x = 0; x <= dimensions.x(); ++x) {
        for (int z = 0; z <= dimensions.z(); ++z) {
          vec3 xz = vec3((float)x, 0, (float)z) * bb_delta;
          vertices.push_back(source.vertex(bb_min, uv_min, uv_delta, xz));
        }
      }

      indices.reserve(dimensions.x() * dimensions.z() * 6);

      int stride = dimensions.x() + 1;
      for (int x = 0; x < dimensions.x(); ++x) {
        for (int z = 0; z < dimensions.z(); ++z) {
          // 01 11
          // 00 10
          indices.push_back((x+0) + (z+0)*stride);
          indices.push_back((x+0) + (z+1)*stride);
          indices.push_back((x+1) + (z+0)*stride);
          indices.push_back((x+1) + (z+0)*stride);
          indices.push_back((x+0) + (z+1)*stride);
          indices.push_back((x+1) + (z+1)*stride);
        }
      }

      set_vertices(vertices);
      set_indices(indices);
    }
  };
}}

