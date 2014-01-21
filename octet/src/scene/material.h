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
    // material
    ref<param> diffuse;
    ref<param> ambient;
    ref<param> emission;
    ref<param> specular;
    ref<param> bump;
    ref<param> shininess;

    void bind_textures() const {
      // set textures 0, 1, 2, 3 to their respective values
      diffuse->render(0, GL_TEXTURE_2D);
      ambient->render(1, GL_TEXTURE_2D);
      emission->render(2, GL_TEXTURE_2D);
      specular->render(3, GL_TEXTURE_2D);
      bump->render(4, GL_TEXTURE_2D);
      shininess->render(5, GL_TEXTURE_2D);
      glActiveTexture(GL_TEXTURE0);
    }

    void init(param *param_) {
      specular = diffuse = ambient = param_;
      emission = new param(vec4(0, 0, 0, 0));
      bump = new param(vec4(0, 0, 0, 0));
      shininess = new param(vec4(30.0f/255, 0, 0, 0));
    }

  public:
    RESOURCE_META(material)

    /// Default constructor makes a blank material.
    material() {
      diffuse = 0;
      ambient = 0;
      emission = 0;
      specular = 0;
      bump = 0;
      shininess = 0;
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
      v.visit(diffuse, atom_diffuse);
      v.visit(ambient, atom_ambient);
      v.visit(emission, atom_emission);
      v.visit(specular, atom_specular);
      v.visit(bump, atom_bump);
      v.visit(shininess, atom_shininess);
    }

    /// Set all the parameters.
    void init(param *diffuse, param *ambient, param *emission, param *specular, param *bump, param *shininess) {
      this->diffuse = diffuse;
      this->ambient = ambient;
      this->emission = emission;
      this->specular = specular;
      this->bump = bump;
      this->shininess = shininess;
    }

    /// make a solid color with a specular highlight
    void make_color(const vec4 &color, bool bumpy, bool shiny) {
      diffuse = ambient = new param(color);
      emission = new param(vec4(0, 0, 0, 0));
      specular = shiny ? new param(vec4(1, 1, 1, 0)) : new param(vec4(0, 0, 0, 0));
      bump = bumpy ? new param(new image("!bump")) : new param(vec4(0.5f, 0.5f, 1, 0));
      shininess = new param(vec4(30.0f/255, 0, 0, 0));
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
  };
}}

