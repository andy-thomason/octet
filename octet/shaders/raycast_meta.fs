//////////////////////////////////////////////////////////////////////////////////////////
//
// raycast shader example
//

// inputs
varying vec3 model_pos_;

uniform vec3 camera_pos;
const vec4 spheres[] = { vec4(0, 0, 0, -0.3), vec4(-3, 0, 0, -0.6), vec4(0, 3, 0, -0.3), vec4(0, -3, 0, -0.4) };
const vec4 colours[] = { vec4(1, 0, 0, 1), vec4(0, 1, 0, 1), vec4(0, 0, 1, 1), vec4(0, 1, 1, 1) };


float squared(vec3 v) { return dot(v, v); }

float calc_value(vec3 pos, inout vec4 v) {
  v[0] = exp(squared(vec3(spheres[0]) - pos) * spheres[0].w);
  v[1] = exp(squared(vec3(spheres[1]) - pos) * spheres[1].w);
  v[2] = exp(squared(vec3(spheres[2]) - pos) * spheres[2].w);
  v[3] = exp(squared(vec3(spheres[3]) - pos) * spheres[3].w);
  return v[0] + v[1] + v[2] + v[3];
}

// note that a more sophisticated system might pick the four closest spheres
// to the ray and hence be able to use many more spheres. A BVH will make this
// much faster.
void main() {
  vec3 ray_start = model_pos_;
  vec3 ray_direction = normalize(model_pos_ - camera_pos);
  vec3 pos = ray_start;
  vec3 best_pos = ray_start;
  vec3 delta = ray_direction * 0.5;
  vec4 v;
  float prev_value = calc_value(pos, v);
  const float threshold = 0.5;
  bool done = false;
  for (int i = 0; i != 20; ++i) {
    pos += delta;
    float value = calc_value(pos, v);
    if (value > threshold && !done) {
      best_pos = pos - delta * (value - threshold) / (value - prev_value);
      // note that it is tempting to add a "break" here. Some GPUs may go faster others will go slower
      // because some CUs will have to still run.
      done = true;
    }
    prev_value = value;
  }

  float value = calc_value(best_pos, v);
  v = normalize(v);
  gl_FragColor = v[0] * colours[0] + v[1] * colours[1] + v[2] * colours[2] + v[3] * colours[3];

  if (!done) discard;
}

