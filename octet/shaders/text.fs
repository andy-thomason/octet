
//////////////////////////////////////////////////////////////////////////////////////////
//
// text shader example
//

// inputs
varying vec2 uv_;
varying vec4 color_;
uniform sampler2D diffuse_sampler;

void main() {
  gl_FragColor = vec4(color_.xyz, texture2D(diffuse_sampler, uv_).w);
  if (gl_FragColor.w < 0.05) discard;
}


