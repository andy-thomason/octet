////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Mesh smooth modifier. Work in progress.
//

namespace octet { namespace scene {
  class mesh_points : public mesh {
    dynarray<vec3p> points;

    void init() {
      set_default_attributes();
      set_params(32, 0, 0, GL_POINTS, 0);
      update();
    }

  public:
    RESOURCE_META(mesh_points)

    mesh_points() {
      init();
    }

    void add_point(vec3_in pos, vec4_in color) {
      points.push_back(pos);
    }

    void update() {
      allocate(sizeof(vertex)*points.size(), 0);

      gl_resource::rwlock vtx_lock(get_vertices());
      vertex *vtx = (vertex *)vtx_lock.u8();

      for (unsigned i = 0; i != points.size(); ++i) {
        vtx->pos = points[i];
        vtx->normal = vec3p(0, 0, 1);
        vtx->uv = vec2(0, 0);
        vtx++;
      }

      set_num_indices(0);
      set_num_vertices(points.size());
    }

    void visit(visitor &v) {
      mesh::visit(v);
    }
  };
}}
