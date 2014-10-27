//////////////////////////////////////////////////////////////////////////////////////////
//
// spots shader example
//

// inputs
varying vec2 uv_;

uniform float num_spots;

void main() {
  vec2 pos = fract(uv_ * num_spots);
  float r2 = dot(pos - 0.5, pos - 0.5);
  gl_FragColor = vec4(r2 < 0.05 ? 0 : 1, 0, 0, 1);
  if (r2 > 0.1) discard;
}

