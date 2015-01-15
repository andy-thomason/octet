//////////////////////////////////////////////////////////////////////////////////////////
//
// raycast shader example
//

// inputs
varying vec3 model_pos_;

uniform vec3 camera_pos;

uniform vec4 spheres[200];
uniform int num_spheres;

uniform sampler1D data;

float squared(vec3 v) { return dot(v, v); }

float calc_value(vec3 pos, inout vec4 v) {
  float sum = 0;
  for (int i = 0; i != num_spheres; ++i) {
    sum += exp(squared(vec3(spheres[i*2+0]) - pos) * spheres[i*2+0].w);
  }
  return sum;
  /*v[0] = exp(squared(vec3(spheres[0*2+0]) - pos) * spheres[0*2+0].w);
  v[1] = exp(squared(vec3(spheres[1*2+0]) - pos) * spheres[1*2+0].w);
  v[2] = exp(squared(vec3(spheres[2*2+0]) - pos) * spheres[2*2+0].w);
  v[3] = exp(squared(vec3(spheres[3*2+0]) - pos) * spheres[3*2+0].w);
  return v[0] + v[1] + v[2] + v[3];*/
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
  for (int i = 0; i != 40; ++i) {
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
  //gl_FragColor = v[0] * spheres[0*2+1] + v[1] * spheres[1*2+1] + v[2] * spheres[2*2+1] + v[3] * spheres[3*2+1];
  gl_FragColor = vec4(1, 0, 0, 1);

  if (!done) discard;
}

