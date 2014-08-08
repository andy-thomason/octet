//////////////////////////////////////////////////////////////////////////////////////////
//
// Default vertex shader for materials. Extend this to deal with bump mapping, defered rendering, shadows etc.
//

// matrices
uniform mat4 modelToProjection;
uniform mat4 modelToCamera;

// attributes from vertex buffer
attribute vec4 pos;
attribute vec2 uv;
attribute vec3 normal;

// outputs
varying vec3 tnormal_;
varying vec3 tpos_;
varying vec2 tuv_;

void main() {
  gl_Position = modelToProjection * pos;
  vec3 tnormal = (modelToCamera * vec4(normal, 0.0)).xyz;
  vec3 tpos = (modelToCamera * pos).xyz;
  tnormal_ = tnormal;
  tpos_ = tpos;
  tuv_ = uv;
}

