//////////////////////////////////////////////////////////////////////////////////////////
//
// default frament shader for solid colours
//

// constant parameters
uniform vec4 lighting[17];
uniform int num_lights;
uniform vec4 diffuse;

// inputs
varying vec2 uv_;
varying vec3 tnormal_;
varying vec3 tpos_;

void main() {
  vec3 nnormal = normalize(tnormal_);
  vec3 npos = tpos_;
  vec3 diffuse_light = lighting[0].xyz;
  for (int i = 0; i != num_lights; ++i) {
    vec3 light_pos = lighting[i * 4 + 1].xyz;
    vec3 light_direction = lighting[i * 4 + 2].xyz;
    vec3 light_color = lighting[i * 4 + 3].xyz;
    vec3 light_atten = lighting[i * 4 + 4].xyz;
    float diffuse_factor = max(dot(light_direction, nnormal), 0.0);
    diffuse_light += diffuse_factor * light_color;
  }
  gl_FragColor = vec4(diffuse.xyz * diffuse_light, 1.0);

}

