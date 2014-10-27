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
      mesh::set_shape<sphere, mesh::vertex>(shape, mat4t(), max_level);
      reindex();
    }

    /// Serialise the box
    void visit(visitor &v) {
      mesh::visit(v);
      if (v.is_reader()) {
        update();
      }
    }

    #ifdef OCTET_BULLET
      /// Get a bullet shape object for this mesh
      btCollisionShape *get_bullet_shape() {
        return new btSphereShape(shape.get_radius());
      }

      /// Get a static bullet shape object for this mesh
      btCollisionShape *get_static_bullet_shape() {
        return new btSphereShape(shape.get_radius());
      }
    #endif
  };
}}

