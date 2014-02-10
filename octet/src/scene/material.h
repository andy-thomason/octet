////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Material 
//
//
// Materials are represented as GL textures with solid colours as single pixel textures.
// This simplifies shader design.
//

namespace octet { namespace scene {
  /// Material class for representing lambert, blinn and phong.
  /// This class sets the uniforms for the shader.
  /// Each parameter of the shader can be a color or an image. We would also like to support functions.
  class material : public resource {
    ref<shader> custom_shader;

    dynarray<ref<param> > params;

    void bind_textures() const {
      for (unsigned i = 0; i != params.size(); ++i) {
        params[i]->render(i);
      }
      glActiveTexture(GL_TEXTURE0);
    }

    void init(param *param_) {
      param *emission = new param(vec4(0, 0, 0, 0));
      param *bump = new param(vec4(0, 0, 0, 0));
      param *specular = new param(vec4(0, 0, 0, 0));
      param *shininess = new param(vec4(30.0f/255, 0, 0, 0));
      init(param_, param_, emission, specular, bump, shininess);
    }

  public:
    RESOURCE_META(material)

    /// Default constructor makes a blank material.
    material() {
    }

    /// Alternative constructor; Don't use this too much, it creates a new image every time.
    material(const char *texture, sampler *sampler_ = 0) {
      init(new param(new image(texture), sampler_));
    }

    /// Alternative constructor; Don't use this too much, it creates a new image every time.
    material(const vec4 &color) {
      init(new param(color));
    }

    /// create a material from an existing image
    material(image *img) {
      init(new param(img));
    }

    /// Serialize.
    void visit(visitor &v) {
    }

    /// Set all the parameters.
    void init(param *diffuse, param *ambient, param *emission, param *specular, param *bump, param *shininess) {
      params.reserve(6);
      params.push_back(diffuse);
      params.push_back(ambient);
      params.push_back(emission);
      params.push_back(specular);
      params.push_back(bump);
      params.push_back(shininess);
    }

    /// make a solid color with a specular highlight
    void make_color(const vec4 &color, bool bumpy, bool shiny) {
      param *diffuse = new param(color);
      param *ambient = diffuse;
      param *emission = new param(vec4(0, 0, 0, 0));
      param *specular = shiny ? new param(vec4(1, 1, 1, 0)) : new param(vec4(0, 0, 0, 0));
      param *bump = bumpy ? new param(new image("!bump")) : new param(vec4(0.5f, 0.5f, 1, 0));
      param *shininess = new param(vec4(30.0f/255, 0, 0, 0));
      init(diffuse, diffuse, emission, specular, bump, shininess);
    }

    /// Set the uniforms for this material.
    void render(bump_shader &shader, const mat4t &modelToProjection, const mat4t &modelToCamera, vec4 *light_uniforms, int num_light_uniforms, int num_lights) const {
      shader.render(modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights);
      bind_textures();
    }

    /// Set the uniforms for this material on skinned meshes.
    void render_skinned(bump_shader &shader, const mat4t &cameraToProjection, const mat4t *modelToCamera, int num_nodes, vec4 *light_uniforms, int num_light_uniforms, int num_lights) const {
      shader.render_skinned(cameraToProjection, modelToCamera, num_nodes, light_uniforms, num_light_uniforms, num_lights);
      bind_textures();
    }

    /// Custom build a shader for this material
    /*void render(const mat4t &modelToProjection, const mat4t &modelToCamera, vec4 *light_uniforms, int num_light_uniforms, int num_lights) const {
      custom_shader->render(modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights);
      bind_textures();
    }*/

  };
}}

