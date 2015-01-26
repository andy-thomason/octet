//////////////////////////////////////////////////////////////////////////////////////////
//
// Vertex color shader
//

// matrices
uniform mat4 modelToProjection;
uniform mat4 modelToCamera;

// attributes from vertex buffer
attribute vec4 pos;
attribute vec4 color;

// outputs
varying vec4 color_;

void main() {
  gl_Position = modelToProjection * pos;
  color_ = color;
}

