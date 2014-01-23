////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet { namespace scene {
  /// Sphere mesh. Generate triangles for a sphere.
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
      return vec2(atan2(normal.x(), normal.z())*0.31831f, normal.y());
    }

  public:
    RESOURCE_META(mesh_sphere)

    mesh_sphere() {
      init(sphere(vec3(0, 0, 0), 1), 3);
    }

    /// Construct box from size
    mesh_sphere(const vec3 &centre, float size, int max_level_=3) {
      init(sphere(centre, size), max_level_);
    }

    void set_shape(const sphere &shape_, int max_level_=3) {
      init(shape_, max_level_);
      update();
    }

    /// Generate mesh from parameters.
    virtual void update() {
      unsigned ni = 3*4 << (max_level*2), nv = 4 << (max_level*2);
      allocate(sizeof(vertex)*nv, sizeof(uint32_t)*ni);
      set_num_indices(ni);
      set_num_vertices(nv);

      gl_resource::rwlock idx_lock(get_indices());
      gl_resource::rwlock vtx_lock(get_vertices());
      uint32_t *idx = idx_lock.u32();
      vertex *vtx = (vertex *)vtx_lock.u8();

      // todo: use this instead...
      /*const float phi = 1.61803f;
      static const float icosahedron[] = {
        0, +1, +phi,
        0, +1, -phi,
        0, -1, +phi,
        0, -1, -phi,
        +1, +phi, 0,
        +1, -phi, 0,
        -1, +phi, 0,
        -1, -phi, 0,
        +phi, 0, +1,
        -phi, 0, +1,
        +phi, 0, -1,
        -phi, 0, -1,
      };*/

      static const float tetrahedron[] = {
        1.0f,  0.0f, -0.707107f,
        -1.0f,  0.0f, -0.707107f,
        0.0f,  1.0f,  0.707107f,
        0.0f, -1.0f,  0.707107f
      };

      int ix = 0, vx = 0;
      const float *p = tetrahedron;
      for (unsigned i = 0; i != 4; ++i) {
        vec3 n = normalize(vec3(p[0], p[1], p[2]));
        vtx[vx].normal = n;
        vtx[vx].pos = pos(n);
        vtx[vx].uv = uv(n);
        vx++;
        p += 3;
      }

      add_triangle(ix, vx, idx, vtx, 0, 1, 2, 0);
      add_triangle(ix, vx, idx, vtx, 0, 1, 3, 0);
      add_triangle(ix, vx, idx, vtx, 0, 2, 3, 0);
      add_triangle(ix, vx, idx, vtx, 1, 2, 3, 0);

      assert(ix == ni);
      assert(vx == nv);

      //mesh::set_mode(GL_POINTS);

      //dump(log("sphere\n"));
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

