
//////////////////////////////////////////////////////////////////////////////////////////
//
// text shader example
//

// inputs
varying vec2 uv_;
uniform sampler2D diffuse_sampler;

void main() {
  gl_FragColor = texture2D(diffuse_sampler, uv_);
  if (gl_FragColor.w < 0.05) discard;
}


