////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Scene Lighting
//

class lighting {
  // lights in world space
  dynarray<vec4> lights;

  // lights in camera space
  dynarray<vec4> transformed_lights;

public:

  lighting() {
  }

  void add_light(const vec4 &position, const vec4 &direction, const vec4 &ambient, const vec4 &diffuse, const vec4 &specular) {
    lights.push_back(position);
    lights.push_back(direction);
    lights.push_back(ambient);
    lights.push_back(diffuse);
    lights.push_back(specular);
    transformed_lights.resize(lights.size());
  }

  // compute camera relative light information
  // todo: only include lights in range
  void compute(const mat4 &worldToCamera) {
    for (unsigned i = 0; i != lights.size(); ++i) {
      transformed_lights[i] = lights[i];
    }
    for (unsigned i = 0; i != lights.size(); i += 5) {
      transformed_lights[i+0] = lights[i+0] * worldToCamera;
      transformed_lights[i+1] = lights[i+1] * worldToCamera;
      //printf("%s -> %s\n", transformed_lights[i+1].toString(), lights[i+1].toString());
    }
  }

  // raw light data for shaders
  vec4 *data() {
    assert(transformed_lights.size() && "must have lights!");
    return &transformed_lights[0];
  }
};

