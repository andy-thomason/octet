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
  class mesh_cylinder : public mesh {
    zcylinder cylinder;
    mat4t transform;
    int steps;

    void init(zcylinder_in cylinder, mat4t_in transform, int steps) {
      this->transform = transform;
      this->cylinder = cylinder;
      this->steps = steps;
      set_default_attributes();
      set_aabb(cylinder.get_aabb().get_transform(transform));
      update();
    }

  public:
    RESOURCE_META(mesh_cylinder)

    /// Construct cylinder mesh from shape
    mesh_cylinder(zcylinder_in cylinder=zcylinder(), mat4t_in transform = mat4t(), int steps=32) {
      init(cylinder, transform, steps);
    }

    void set_size(zcylinder_in cylinder=zcylinder(), mat4t_in transform = mat4t(), int steps=32) {
      init(cylinder, transform, steps);
    }

    /// Generate mesh from parameters.
    virtual void update() {
      mesh::set_shape<zcylinder, mesh::vertex>(cylinder, transform, steps);
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
        return new btCylinderShapeZ(btVector3(cylinder.get_radius(), cylinder.get_radius(), cylinder.get_half_extent()));
      }

      /// Get a static bullet shape object for this mesh
      btCollisionShape *get_static_bullet_shape() {
        return new btCylinderShapeZ(btVector3(cylinder.get_radius(), cylinder.get_radius(), cylinder.get_half_extent()));
      }
    #endif
  };
}}

