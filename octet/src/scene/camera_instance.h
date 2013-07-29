////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Scene camera
//

namespace octet {
  class camera_instance : public resource {
    // camera parameters
    ref<scene_node> node;
    bool is_ortho;

    // common to all cameras
    float nearVal;
    float farVal;

    // perspective camera
    float xfov;
    float yfov;
    float aspect_ratio;

    // ortho camera
    float xmag;
    float ymag;

    // generated matrices
    mat4t worldToCamera;
    mat4t cameraToProjection;

  public:
    RESOURCE_META(camera_instance)

    camera_instance() {
    }

    void visit(visitor &v) {
    // camera parameters
    ref<scene_node> node;
      v.visit(is_ortho, atom_is_ortho);

      // common to all cameras
      v.visit(nearVal, atom_nearVal);
      v.visit(farVal, atom_farVal);

      // perspective camera
      v.visit(xfov, atom_xfov);
      v.visit(yfov, atom_yfov);
      v.visit(aspect_ratio, atom_aspect_ratio);

      // ortho camera
      v.visit(xmag, atom_xmag);
      v.visit(ymag, atom_ymag);

      // generated matrices
      v.visit(worldToCamera, atom_worldToCamera);
      v.visit(cameraToProjection, atom_cameraToProjection);
    }

    // set the parameters as in the collada perspective element
    void set_perspective(float xfov, float yfov, float aspect_ratio, float n, float f)
    {
      this->xfov = xfov;
      this->yfov = yfov;
      this->nearVal = n;
      this->farVal = f;
      is_ortho = false;
    }

    void set_ortho(float xmag, float ymag, float aspect_ratio, float n, float f)
    {
      this->xmag = xmag;
      this->ymag = ymag;
      this->nearVal = n;
      this->farVal = f;
      is_ortho = true;
    }

    void set_node(scene_node *node) {
      this->node = node;
    }

    // call this once a frame to get the camera position
    void set_cameraToWorld(const mat4t &cameraToWorld, float aspect_ratio) {
      // flip cameraToWorld around to transform from world to camera
      cameraToWorld.invertQuick(worldToCamera);

      // build a projection matrix to add perspective
      cameraToProjection.loadIdentity();
      if (is_ortho) {
        cameraToProjection.ortho(-xmag, xmag, -ymag, ymag, nearVal, farVal);
      } else {
        float xscale = 0.5f;
        float yscale = 0.5f;
        if (yfov) {
          yscale = 1.0f / tanf(yfov * (3.14159f/180/2));
          xscale = yscale * aspect_ratio;
        } else if (xfov) {
          xscale = 1.0f / tanf(xfov * (3.14159f/180/2));
          yscale = xscale / aspect_ratio;
        }
        cameraToProjection.frustum(-nearVal * xscale, nearVal * xscale, -nearVal * yscale, nearVal * yscale, nearVal, farVal);
      }
    }

    // call this many times to build matrices for uniforms.
    void get_matrices(mat4t &modelToProjection, mat4t &modelToCamera, const mat4t &modelToWorld) const
    {
      // model -> world -> camera
      modelToCamera = modelToWorld * worldToCamera;

      // model -> world -> camera -> projection
      modelToProjection = modelToCamera * cameraToProjection;
    }

    // call this many times to build matrices for uniforms.
    const mat4t &get_cameraToProjection() const
    {
      return cameraToProjection;
    }

    // use this to get the camera from the scene
    scene_node *get_node() const {
      return node;
    }
  };
}

