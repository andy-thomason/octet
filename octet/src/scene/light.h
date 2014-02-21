////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Light in a scene. Note that for shadow maps, lights are cameras.
//

namespace octet { namespace scene {
  /// Instance of a light in the scene. Comprises a scene node and light data.
  class light : public resource {
    atom_t kind;
    vec4 color;
    float constant_attenuation;
    float linear_attenuation;
    float quadratic_attenuation;
    float falloff_angle;
    float falloff_exponent;

    // todo: compute these values
    float near_plane;
    float far_plane;
  public:
    RESOURCE_META(light)

    /// Construct a light
    light(float _near_plane = 0.1f, float _far_plane = 1000.0f) {
      kind = atom_directional;
      color = vec4(1, 1, 1, 1);
      constant_attenuation = 1;
      linear_attenuation = 0;
      quadratic_attenuation = 0;
      falloff_angle = 45.0f;
      falloff_exponent = 1;
      near_plane = _near_plane;
      far_plane = _far_plane;
    }

    /// Serialize.
    void visit(visitor &v) {
      v.visit(kind, atom_kind);
      v.visit(color, atom_color);
      v.visit(constant_attenuation, atom_constant_attenuation);
      v.visit(linear_attenuation, atom_linear_attenuation);
      v.visit(quadratic_attenuation, atom_quadratic_attenuation);
      v.visit(falloff_angle, atom_falloff_angle);
      v.visit(falloff_exponent, atom_falloff_exponent);

      v.visit(near_plane, atom_near_plane);
      v.visit(far_plane, atom_far_plane);
    }

    /// Set the kind of light: atom_spot, atom_directional, atom_point etc.
    void set_kind(atom_t kind) {
      this->kind = kind;
    }

    /// Set the near and far planes of the light. (Lights get used for shadow maps).
    void set_near_far(float near_plane, float far_plane) {
      this->near_plane = near_plane;
      this->far_plane = far_plane;
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
    void get_fragment_uniforms(scene_node *node, vec4 *uniforms, const mat4t &worldToCamera) {
      mat4t lightToCamera = node->calcModelToWorld() * worldToCamera;
      uniforms[0] = lightToCamera.w();
      uniforms[1] = lightToCamera.z();
      uniforms[2] = color;
      uniforms[3] = vec4( constant_attenuation, linear_attenuation, quadratic_attenuation, falloff_exponent );
    }
  };
}}
