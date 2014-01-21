////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Scene camera
//

namespace octet { namespace scene {
  /// Instance of a camera in a scene. Provides camera paramers and a node for transformation.
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
    mat4t cameraToWorld;
    mat4t cameraToProjection;

    // generated params
    float xscale;
    float yscale;

  public:
    RESOURCE_META(camera_instance)

    /// Constuct a camera instance.
    camera_instance() {
      is_ortho = 0;

      // common to all cameras
      nearVal = 0.1f;
      farVal = 1000;

      // perspective camera
      xfov = 0;
      yfov = 90;
      aspect_ratio = 1;

      // ortho camera
      xmag = 1;
      ymag = 1;
    }

    /// Serialize
    void visit(visitor &v) {
    // camera parameters
      v.visit(node, atom_node);
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
    }

    /// set the parameters as in the collada perspective element
    void set_perspective(float xfov, float yfov, float aspect_ratio, float n, float f)
    {
      this->xfov = xfov;
      this->yfov = yfov;
      this->nearVal = n;
      this->farVal = f;
      is_ortho = false;
    }

    /// set the parameters as in the collada ortho element
    void set_ortho(float xmag, float ymag, float aspect_ratio, float n, float f)
    {
      this->xmag = xmag;
      this->ymag = ymag;
      this->nearVal = n;
      this->farVal = f;
      is_ortho = true;
    }

    /// set the transform node
    void set_node(scene_node *node) {
      this->node = node;
    }

    /// call this once a frame to set the camera parameters.
    void set_cameraToWorld(const mat4t &cameraToWorld_, float aspect_ratio) {
      cameraToWorld = cameraToWorld_;
      // flip cameraToWorld around to transform from world to camera
      worldToCamera = cameraToWorld.inverse3x4();

      // build a projection matrix to add perspective
      cameraToProjection.loadIdentity();
      if (is_ortho) {
        xscale = 1.0f / xmag;
        yscale = 1.0f / ymag;
        cameraToProjection.ortho(-xmag*0.5f, xmag*0.5f, -ymag*0.5f, ymag*0.5f, nearVal, farVal);
      } else {
        xscale = 0.5f;
        yscale = 0.5f;
        if (yfov) {
          yscale = tanf(yfov * (3.14159f/180/2));
          xscale = yscale * aspect_ratio;
        } else if (xfov) {
          xscale = tanf(xfov * (3.14159f/180/2));
          yscale = xscale / aspect_ratio;
        }
        cameraToProjection.frustum(-nearVal * xscale, nearVal * xscale, -nearVal * yscale, nearVal * yscale, nearVal, farVal);
      }
    }

    /// call this many times to build matrices for uniforms.
    void get_matrices(mat4t &modelToProjection, mat4t &modelToCamera, const mat4t &modelToWorld) const
    {
      // model -> world -> camera
      modelToCamera = modelToWorld * worldToCamera;

      // model -> world -> camera -> projection
      modelToProjection = modelToCamera * cameraToProjection;
    }

    /// call this many times to build matrices for uniforms.
    const mat4t &get_cameraToProjection() const
    {
      return cameraToProjection;
    }

    /// return a ray from screen (x, y) to the far plane; used for picking.
    ray get_ray(float x, float y) {
      vec4 ray_start, ray_end;

      // convert projection space ray to world space
      if (is_ortho) {
        ray_start = vec4(xscale * x, yscale * y, -nearVal, 1) * cameraToWorld;
        ray_end = vec4(xscale * x, yscale * y, -farVal, 1) * cameraToWorld;
      } else {
        ray_start = vec4(xscale * nearVal * x, yscale * nearVal * y, -nearVal, 1) * cameraToWorld;
        ray_end = vec4(xscale * farVal * x, yscale * farVal * y, -farVal, 1) * cameraToWorld;
      }

      return ray(ray_start.xyz(), ray_end.xyz());
    }

    /// Get the world to projection matrix for this camera.
    mat4t get_worldToProjection() const {
      mat4t result;
      result.loadIdentity();
      if (node) {
        mat4t worldToCamera = node->calcModelToWorld().inverse3x4();
        result = worldToCamera * cameraToProjection;
      }
      return result;
    }

    /// Get the node used for the transform.
    scene_node *get_node() const {
      return node;
    }

    /// is this camera instance an ortho camera? Todo: create a separate camera object.
    bool get_is_ortho() const {
      return is_ortho;
    }

    /// Near plane
    float get_nearVal() const {
      return nearVal;
    }

    /// Far plane
    float get_farVal() const {
      return farVal;
    }

    /// x scale for ortho.
    float get_xscale() const {
      return xscale;
    }

    /// y scale for ortho.
    float get_yscale() const {
      return yscale;
    }
  };
}}

