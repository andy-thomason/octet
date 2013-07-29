////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Light in a scene. Note that for shadow maps, lights are cameras.
//

namespace octet {
  class light_instance : public resource {
    ref<scene_node> node;

    atom_t kind;
    vec4 color;
    float constant_attenuation;
    float linear_attenuation;
    float quadratic_attenuation;
    float falloff_angle;
    float falloff_exponent;

    // todo: compute these values
    float nearVal;
    float farVal;
  public:
    RESOURCE_META(light_instance)

    light_instance() {
      nearVal = 0.1f;
      farVal = 1000.0f;
    }

    void set_node(scene_node *node) {
      this->node = node;
    }

    void set_kind(atom_t kind) {
      this->kind = kind;
    }

    void set_near_far(float nearVal, float farVal) {
      this->nearVal = nearVal;
      this->farVal = farVal;
    }

    void set_color(const vec4 &color) {
      this->color = color;
    }

    void set_attenuation(float constant_attenuation, float linear_attenuation, float quadratic_attenuation) {
      this->constant_attenuation = constant_attenuation;
      this->linear_attenuation = linear_attenuation;
      this->quadratic_attenuation = quadratic_attenuation;
    }

    void set_falloff(float falloff_angle, float falloff_exponent) {
      this->falloff_angle = falloff_angle;
      this->falloff_exponent = falloff_exponent;
    }

    atom_t get_kind() {
      return kind;
    }

    vec4 get_color() {
      return color;
    }

    void visit(visitor &v) {
    }

    // in the fragment shader, we give the position and direction for diffuse and specular calculation
    void get_fragment_uniforms(vec4 *uniforms, const mat4t &worldToCamera) {
      if (node) {
        mat4t lightToCamera = node->calcModelToWorld() * worldToCamera;
        uniforms[0] = lightToCamera.w();
        uniforms[1] = lightToCamera.z();
        uniforms[2] = color;
        uniforms[3] = vec4( constant_attenuation, linear_attenuation, quadratic_attenuation, falloff_exponent );
      }
    }

    // in the vertex shader, we treat the light like a camera with a perspective transform.
    void get_vertex_uniforms(vec4 *uniforms, const mat4t &worldToCamera) {
      //mat4t lightToCamera = node->calcModelToWorld() * worldToCamera;
      //mat4t cameraToLight = lightToCamera.inverse4x4();
      //mat4t lightToPerspective = lightToCamera;
      /*if (kind == atom_directional) {
        lightToPerspective.ortho(
      } else if (kind == atom_spot) {
        lightToPerspective.frustum(
      } else {
        lightToPerspective.frustum(
      }*/
    }
  };
}
