////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Material 
//
//
// Materials are represented as GL textures with solid colours as single pixel textures.
// This simplifies shader design.
//

namespace octet {
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
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, diffuse->get_gl_texture());
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, ambient->get_gl_texture());
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, emission->get_gl_texture());
      glActiveTexture(GL_TEXTURE3);
      glBindTexture(GL_TEXTURE_2D, specular->get_gl_texture());
      glActiveTexture(GL_TEXTURE4);
      glBindTexture(GL_TEXTURE_2D, bump->get_gl_texture());
      glActiveTexture(GL_TEXTURE5);
      glBindTexture(GL_TEXTURE_2D, shininess->get_gl_texture());
      glActiveTexture(GL_TEXTURE0);
    }

  public:
    RESOURCE_META(material)

    // default constructor makes a blank material.
    material() {
      diffuse = 0;
      ambient = 0;
      emission = 0;
      specular = 0;
      bump = 0;
      shininess = 0;
    }

    // don't use this too much, it creates a new image every time.
    material(const char *texture) {
      specular = diffuse = ambient = new param(new image(texture));
      emission = new param(vec4(0, 0, 0, 0));
      bump = new param(vec4(0, 0, 0, 0));
      shininess = new param(vec4(30.0f/255, 0, 0, 0));
    }

    void visit(visitor &v) {
      v.visit(diffuse, atom_diffuse);
      v.visit(ambient, atom_ambient);
      v.visit(emission, atom_emission);
      v.visit(specular, atom_specular);
      v.visit(bump, atom_bump);
      v.visit(shininess, atom_shininess);
    }

    void init(param *diffuse, param *ambient, param *emission, param *specular, param *bump, param *shininess) {
      this->diffuse = diffuse;
      this->ambient = ambient;
      this->emission = emission;
      this->specular = specular;
      this->bump = bump;
      this->shininess = shininess;
    }

    // make a solid color with a specular highlight
    void make_color(const vec4 &color, bool bumpy, bool shiny) {
      diffuse = ambient = new param(color);
      emission = new param(vec4(0, 0, 0, 0));
      specular = shiny ? new param(vec4(1, 1, 1, 0)) : new param(vec4(0, 0, 0, 0));
      bump = bumpy ? new param(new image("!bump")) : new param(vec4(0.5f, 0.5f, 1, 0));
      shininess = new param(vec4(30.0f/255, 0, 0, 0));
    }

    void render(bump_shader &shader, const mat4t &modelToProjection, const mat4t &modelToCamera, vec4 *light_uniforms, int num_light_uniforms, int num_lights) const {
      shader.render(modelToProjection, modelToCamera, light_uniforms, num_light_uniforms, num_lights);
      bind_textures();
    }

    void render_skinned(bump_shader &shader, const mat4t &cameraToProjection, const mat4t *modelToCamera, int num_nodes, vec4 *light_uniforms, int num_light_uniforms, int num_lights) const {
      shader.render_skinned(cameraToProjection, modelToCamera, num_nodes, light_uniforms, num_light_uniforms, num_lights);
      bind_textures();
    }
  };
}

