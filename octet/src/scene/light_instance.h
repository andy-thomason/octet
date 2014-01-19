////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Light in a scene. Note that for shadow maps, lights are cameras.
//

namespace octet { namespace scene {
  /// Instance of a light in the scene. Comprises a scene node and light data.
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

    /// Construct a default light instance
    light_instance() {
      nearVal = 0.1f;
      farVal = 1000.0f;
    }

    /// Serialize.
    void visit(visitor &v) {
      v.visit(node, atom_node);

      v.visit(kind, atom_kind);
      v.visit(color, atom_color);
      v.visit(constant_attenuation, atom_constant_attenuation);
      v.visit(linear_attenuation, atom_linear_attenuation);
      v.visit(quadratic_attenuation, atom_quadratic_attenuation);
      v.visit(falloff_angle, atom_falloff_angle);
      v.visit(falloff_exponent, atom_falloff_exponent);

      v.visit(nearVal, atom_nearVal);
      v.visit(farVal, atom_farVal);
    }

    /// Set the transform node
    void set_node(scene_node *node) {
      this->node = node;
    }

    /// Set the kind of light: atom_spot, atom_directional, atom_point etc.
    void set_kind(atom_t kind) {
      this->kind = kind;
    }

    /// Set the near and far planes of the light. (Lights get used for shadow maps).
    void set_near_far(float nearVal, float farVal) {
      this->nearVal = nearVal;
      this->farVal = farVal;
    }

    /// Set the color of the light,
    void set_color(const vec4 &color) {
      this->color = color;
    }

    /// Set the distance attenuation of the light. 1/(c + lx + qx^2)
    void set_attenuation(float constant_attenuation, float linear_attenuation, float quadratic_attenuation) {
      this->constant_attenuation = constant_attenuation;
      this->linear_attenuation = linear_attenuation;
      this->quadratic_attenuation = quadratic_attenuation;
    }

    /// Set the angular fall-off for spot lights.
    void set_falloff(float falloff_angle, float falloff_exponent) {
      this->falloff_angle = falloff_angle;
      this->falloff_exponent = falloff_exponent;
    }

    /// get the kind of the light. atom_spot etc.
    atom_t get_kind() {
      return kind;
    }

    /// get the color of the light.
    vec4 get_color() {
      return color;
    }

    /// Compute parameters for a fragment shader.
    /// in the fragment shader, we give the position and direction for diffuse and specular calculation
    void get_fragment_uniforms(vec4 *uniforms, const mat4t &worldToCamera) {
      if (node) {
        mat4t lightToCamera = node->calcModelToWorld() * worldToCamera;
        uniforms[0] = lightToCamera.w();
        uniforms[1] = lightToCamera.z();
        uniforms[2] = color;
        uniforms[3] = vec4( constant_attenuation, linear_attenuation, quadratic_attenuation, falloff_exponent );
      }
    }

    /// Compute parameters for the vertex shader.
    /// in the vertex shader, we treat the light like a camera with a perspective transform.
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
}}
