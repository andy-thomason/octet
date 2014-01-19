////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Mesh smooth modifier. Work in progress.
//

namespace octet { namespace scene {
  /// Box mesh. Generate triangles for an AABB.
  class mesh_box : public mesh {

    void init(const aabb &size) {
      set_default_attributes();
      set_aabb(size);
      update();
    }

    static const float *box_vertices() {
      static const float data[4 * 6 * 8] = {
        // front
        #undef OCTET_FACE
        #define OCTET_FACE(X, Y, U, V) X, Y, 1,  0, 0, 1, U, V,
        OCTET_FACE( 1,  1,  1, 1 ) OCTET_FACE( 1, -1,  1, 0 ) OCTET_FACE(-1, -1,  0, 0 ) OCTET_FACE(-1,  1,  0, 1 )

        // back
        #undef OCTET_FACE
        #define OCTET_FACE(X, Y, U, V) -(X), -(Y), -1,  0, 0, -1, U, V,
        OCTET_FACE( 1,  1,  1, 1 ) OCTET_FACE( 1, -1,  1, 0 ) OCTET_FACE(-1, -1,  0, 0 ) OCTET_FACE(-1,  1,  0, 1 )

        // top
        #undef OCTET_FACE
        #define OCTET_FACE(X, Y, U, V) Y, 1, X,  0, 1, 0, U, V,
        OCTET_FACE( 1,  1,  1, 1 ) OCTET_FACE( 1, -1,  1, 0 ) OCTET_FACE(-1, -1,  0, 0 ) OCTET_FACE(-1,  1,  0, 1 )

        // bottom
        #undef OCTET_FACE
        #define OCTET_FACE(X, Y, U, V) -(Y), -1, -(X),  0, 0, -1, U, V,
        OCTET_FACE( 1,  1,  1, 1 ) OCTET_FACE( 1, -1,  1, 0 ) OCTET_FACE(-1, -1,  0, 0 ) OCTET_FACE(-1,  1,  0, 1 )

        // right
        #undef OCTET_FACE
        #define OCTET_FACE(X, Y, U, V) 1, X, Y,  1, 0, 0, U, V,
        OCTET_FACE( 1,  1,  1, 1 ) OCTET_FACE( 1, -1,  1, 0 ) OCTET_FACE(-1, -1,  0, 0 ) OCTET_FACE(-1,  1,  0, 1 )

        // left
        #undef OCTET_FACE
        #define OCTET_FACE(X, Y, U, V) -1, -(X), -(Y),  -1, 0, 0, U, V,
        OCTET_FACE( 1,  1,  1, 1 ) OCTET_FACE( 1, -1,  1, 0 ) OCTET_FACE(-1, -1,  0, 0 ) OCTET_FACE(-1,  1,  0, 1 )

        #undef OCTET_FACE
      };

      return data;
    }

    static const uint32_t *box_indices() {
      // 3 0
      // 2 1
      static const uint32_t data[6 * 6] = {
        0+4*0, 1+4*0, 3+4*0,  1+4*0, 2+4*0, 3+4*0,
        0+4*1, 1+4*1, 3+4*1,  1+4*1, 2+4*1, 3+4*1,
        0+4*2, 1+4*2, 3+4*2,  1+4*2, 2+4*2, 3+4*2,
        0+4*3, 1+4*3, 3+4*3,  1+4*3, 2+4*3, 3+4*3,
        0+4*4, 1+4*4, 3+4*4,  1+4*4, 2+4*4, 3+4*4,
        0+4*5, 1+4*5, 3+4*5,  1+4*5, 2+4*5, 3+4*5,
      };
      return data;
    }
  public:
    RESOURCE_META(mesh_box)

    /// Default constructor: 1x1x1
    mesh_box() {
      init(aabb(vec3(0, 0, 0), vec3(1, 1, 1)));
    }

    /// Construct box from size
    mesh_box(const vec3 &size) {
      init(aabb(vec3(0, 0, 0), size));
    }

    /// Construct box from aabb
    mesh_box &set_size(const aabb &size) {
      init(size);
      update();
    }

    /// Generate mesh from parameters.
    virtual void update() {
      allocate(sizeof(vertex)*4*6, sizeof(uint32_t)*6*6);
      vertex *vtx = (vertex *)get_vertices()->lock();
      uint32_t *idx = (uint32_t *)get_indices()->lock();

      const float *fs = box_vertices();
      vec3 center = get_aabb().get_center();
      vec3 half_extent = get_aabb().get_half_extent();
      for (unsigned i = 0; i != 4*6; ++i) {
        vtx->pos = center + vec3(fs[0], fs[1], fs[2]) * half_extent;
        vtx->normal = vec3(fs[3], fs[4], fs[5]);
        vtx->uv = vec2(fs[6], fs[7]);
        vtx++;
        fs += 8;
      }
      assert((int)fs - (int)box_vertices() == get_vertices()->get_size());

      memcpy(idx, box_indices(), sizeof(uint32_t)*6*6);

      get_vertices()->unlock();
      get_indices()->unlock();
      set_num_indices(6 * 6);
      set_num_vertices(4 * 6);
      //dump(log("box\n"));
    }

    /// Serialise the box
    void visit(visitor &v) {
      mesh::visit(v);
      v.visit(size, atom_size);
      if (v.is_reader()) {
        update();
      }
    }
  };
}}

