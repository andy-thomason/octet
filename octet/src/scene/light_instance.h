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
  class light_instance : public resource {
    ref<scene_node> node;
    ref<light> light_;
  public:
    RESOURCE_META(light_instance)

    /// Construct a default light instance
    light_instance() {
    }

    light_instance(scene_node *_node, light *_light) {
      node = _node;
      light_ = _light;
    }

    /// Serialize.
    void visit(visitor &v) {
      v.visit(node, atom_node);
      v.visit(light_, atom_light);
    }

    /// Set the transform node
    void set_node(scene_node *node) {
      this->node = node;
    }

    scene_node *get_node() const {
      return node;
    }

    /// Set the transform light
    void set_light(light *_light) {
      this->light_ = _light;
    }

    light *get_light() const {
      return light_;
    }

    /// Compute parameters for a fragment shader.
    /// in the fragment shader, we give the position and direction for diffuse and specular calculation
    void get_fragment_uniforms(vec4 *uniforms, const mat4t &worldToCamera) {
      if (node && light_) {
        light_->get_fragment_uniforms(node, uniforms, worldToCamera);
      }
    }
  };
}}
