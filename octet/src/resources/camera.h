////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Scene camera
//

class camera {
  // camera parameters
  int node_;
  bool is_ortho;
  float left;
  float right;
  float bottom;
  float top;
  float nearVal;
  float farVal;

  // generated matrices
  mat4 worldToCamera;
  mat4 cameraToProjection;

public:
  camera() {
  }

  // call this once to set up the camera
  void set_params(int node, float left, float right, float bottom, float top, float nearVal, float farVal, bool is_ortho) {
    this->left = left;
    this->right = right;
    this->bottom = bottom;
    this->top = top;
    this->nearVal = nearVal;
    this->farVal = farVal;
    this->is_ortho = is_ortho;
    node_ = node;
  }

  // call this once a frame to set up the camera position
  void set_cameraToWorld(const mat4 &cameraToWorld) {
    // flip it around to transform from world to camera
    cameraToWorld.invertQuick(worldToCamera);

    // build a projection matrix to add perspective
    cameraToProjection.loadIdentity();
    if (is_ortho) {
      cameraToProjection.ortho(left, right, bottom, top, nearVal, farVal);
    } else {
      cameraToProjection.frustum(left, right, bottom, top, nearVal, farVal);
    }
  }

  // call this many times to build matrices for uniforms.
  void get_matrices(mat4 &modelToProjection, mat4 &modelToCamera, const mat4 &modelToWorld)
  {
    // model -> world -> camera
    modelToCamera = modelToWorld * worldToCamera;

    // model -> world -> camera -> projection
    modelToProjection = modelToCamera * cameraToProjection;
  }

  // call this many times to build matrices for uniforms.
  const mat4 &get_cameraToProjection()
  {
    return cameraToProjection;
  }

  // use this to get the camera from the scene
  int node() {
    return node_;
  }
};

