//////////////////////////////////////////////////////////////////////////////////////////
//
// default frament shader for textures
//

// constant parameters
uniform vec4 lighting[17];
uniform int num_lights;
uniform samplerCube diffuse_sampler;

// inputs
varying vec2 uv_;
varying vec3 normal_;
varying vec3 model_pos_;
varying vec4 color_;

void main() {
  vec4 diffuse = textureCube(diffuse_sampler, normalize(model_pos_));
  gl_FragColor = vec4(diffuse.xyz, 1.0);
}

