////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet { namespace scene {
  /// Sphere mesh. Generate triangles for a sphere.
  // todo: de-duplicate the vertices.
  // todo: offer alternative projections.
  class mesh_sphere : public mesh {
    sphere shape;
    int max_level;

    void init(const sphere &shape_, int max_level_) {
      max_level = max_level_;
      set_default_attributes();
      shape = shape_;
      set_aabb(shape.get_aabb());
      update();
    }

    void add_triangle(int &ix, int &vx, uint32_t *idx, vertex *vtx, int a, int b, int c, int level) {
      //log("add_triangle %d %d %d %d\n", a, b, c, level);
      if (level == max_level) {
        idx[ix++] = a;
        idx[ix++] = b;
        idx[ix++] = c;
      } else {
        //     a
        //     /\
        //    /__\
        //   /_\/_\
        // c        b
        vec3 pa = vtx[a].pos;
        vec3 pb = vtx[b].pos;
        vec3 pc = vtx[c].pos;

        int nab = vx++;
        int nbc = vx++;
        int nca = vx++;

        vec3 ab = normalize((pa + pb) * 0.5f);
        vec3 bc = normalize((pb + pc) * 0.5f);
        vec3 ca = normalize((pc + pa) * 0.5f);

        vtx[nab].pos = pos(ab); vtx[nab].normal = ab; vtx[nab].uv = uv(ab);
        vtx[nbc].pos = pos(bc); vtx[nbc].normal = bc; vtx[nbc].uv = uv(bc);
        vtx[nca].pos = pos(ca); vtx[nca].normal = ca; vtx[nca].uv = uv(ca);

        add_triangle(ix, vx, idx, vtx, a, nab, nca, level+1);
        add_triangle(ix, vx, idx, vtx, b, nbc, nab, level+1);
        add_triangle(ix, vx, idx, vtx, c, nca, nbc, level+1);
        add_triangle(ix, vx, idx, vtx, nab, nbc, nca, level+1);
      }
    }

    vec3 pos(vec3_in normal) {
      return normal * shape.get_radius();
    }

    vec2 uv(vec3_in normal) {
      // mercatoresque projection.
      return vec2(atan2(normal.x(), normal.z())*0.1591549f+0.5f, (normal.y()+1)*0.5f );
    }

    void add_shape(const float *vertices, unsigned nv, const uint8_t *indices, unsigned ni) {
      unsigned tot_ni = 3*ni << (max_level*2);
      unsigned tot_nv = nv;
      for (int i = 0; i < max_level; ++i) {
        tot_nv += (ni*3) << (i*2);
      }

      allocate(sizeof(vertex)*tot_nv, sizeof(uint32_t)*tot_ni);
      set_num_indices(tot_ni);
      set_num_vertices(tot_nv);

      gl_resource::rwlock idx_lock(get_indices());
      gl_resource::rwlock vtx_lock(get_vertices());
      uint32_t *idx = idx_lock.u32();
      vertex *vtx = (vertex *)vtx_lock.u8();

      int ix = 0, vx = 0;
      for (unsigned i = 0; i != nv; ++i) {
        vec3 n = normalize(vec3(vertices[0], vertices[1], vertices[2]));
        vtx[vx].normal = n;
        vtx[vx].pos = pos(n);
        vtx[vx].uv = uv(n);
        vx++;
        vertices += 3;
      }

      for (unsigned i = 0; i != ni; ++i) {
        add_triangle(ix, vx, idx, vtx, indices[0], indices[1], indices[2], 0);
        indices += 3;
      }

      assert(ix == tot_ni);
      assert(vx == tot_nv);
    }

  public:
    RESOURCE_META(mesh_sphere)

    mesh_sphere() {
      init(sphere(vec3(0, 0, 0), 1), 3);
    }

    /// Construct sphere of certain radius and detail level.
    mesh_sphere(const vec3 &centre, float size, int max_level_=3) {
      init(sphere(centre, size), max_level_);
    }

    void set_shape(const sphere &shape_, int max_level_=3) {
      init(shape_, max_level_);
      update();
    }

    /// Generate mesh from parameters.
    virtual void update() {
      int kind = 1;
      if (kind == 0) {
        static const float tetrahedron[] = {
          1.0f,  0.0f, -0.707107f,   -1.0f,  0.0f, -0.707107f,
          0.0f,  1.0f,  0.707107f,   0.0f, -1.0f,  0.707107f
        };

        static const uint8_t tetrahedron_indices[] = {
          0, 1, 2,   0, 1, 3,   0, 2, 3,   1, 2, 3,
        };
        add_shape(tetrahedron, 4, tetrahedron_indices, 4);
      } else {
        const float phi = 1.61803f;
        static const float icosahedron[] = {
          0, +1, +phi,    0, -1, +phi,    0, +1, -phi,    0, -1, -phi,
          +1, +phi, 0,    -1, +phi, 0,    +1, -phi, 0,    -1, -phi, 0,
          +phi, 0, +1,    +phi, 0, -1,    -phi, 0, +1,    -phi, 0, -1,
        };

        static uint8_t icosahedron_indices[] = {  
          0,  8,  1,      0,  1, 10,      0,  4,  8,      0,  5,  4,  
          0, 10,  5,      2, 11,  3,      2,  5, 11,      2,  4,  5,  
          2,  9,  4,      2,  3,  9,      1,  7, 10,      1,  6,  7,  
          1,  8,  6,      3,  6,  9,      3,  7,  6,      3, 11,  7,  
          4,  9,  8,      5, 10, 11,      6,  8,  9,      7, 11, 10
        };
        add_shape(icosahedron, 12, icosahedron_indices, 20);
      }
    }

    /// Serialise the box
    void visit(visitor &v) {
      mesh::visit(v);
      if (v.is_reader()) {
        update();
      }
    }
  };
}}

