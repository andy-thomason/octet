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
    ref<param_shader> custom_shader;

    // Parameters connect colors and other values to uniform buffers.
    dynarray<ref<param> > params;

    //dynarray<uint8_t> static_buffer;
    dynarray<uint8_t> buffer;

    // create the parameters that change frequently such as the matrices and lighting
    void create_dynamic_params() {
      buffer.reserve(0x200);
      param_buffer_info dynamic_pbi(buffer);

      params.push_back(new param_uniform(dynamic_pbi, NULL, atom_modelToProjection, GL_FLOAT_MAT4, 1, param::stage_vertex));
      params.push_back(new param_uniform(dynamic_pbi, NULL, atom_modelToCamera, GL_FLOAT_MAT4, 1, param::stage_vertex));
      params.push_back(new param_uniform(dynamic_pbi, NULL, atom_lighting, GL_FLOAT_VEC4, ambient_size + max_lights * light_size, param::stage_fragment));
      params.push_back(new param_uniform(dynamic_pbi, NULL, atom_num_lights, GL_INT, 1, param::stage_fragment));
    }

    // create the attribute parameters
    void create_attribute_params() {
      params.push_back(new param_attribute(atom_pos, GL_FLOAT_VEC4));
      params.push_back(new param_attribute(atom_uv, GL_FLOAT_VEC2));
      params.push_back(new param_attribute(atom_normal, GL_FLOAT_VEC3));
    }

  public:
    RESOURCE_META(material)

    enum {
      ambient_size = 1,
      max_lights = 4,
      light_size = 4,
    };

    /// Default constructor makes a blank material.
    material() {
    }

    /// Alternative constructor.
    material(const vec4 &color, param_shader *shader = NULL) {
      // materials are constructed from parameters which build the final shader.
      // this allows us to use OpenGLES2 (uniforms) and 3 (buffers) as well as new shader features.
      params.reserve(16);

      create_dynamic_params();
      create_attribute_params();

      param_buffer_info static_pbi(buffer);
      params.push_back(new param_color(static_pbi, color, atom_diffuse, param::stage_fragment));

      if (shader == NULL) {
        shader = new param_shader("shaders/default.vs", "shaders/default_solid.fs");
      }
      shader->init(params);
      custom_shader = shader;
    }

    /// create a material from an existing image
    material(image *img, sampler *smpl = NULL, param_shader *shader = NULL) {
      if (!smpl) smpl = new sampler();

      params.reserve(16);

      create_dynamic_params();
      create_attribute_params();

      param_buffer_info static_pbi(buffer);
      params.push_back(new param_sampler(static_pbi, atom_diffuse_sampler, img, smpl, param::stage_fragment));

      if (shader == NULL) {
        shader = new param_shader("shaders/default.vs", "shaders/default_textured.fs");
        shader->init(params);
      }
      custom_shader = shader;
    }

    material(param *diffuse, param *ambient, param *emission, param *specular, param *bump, param *shininess) {
    }

    /// Serialize.
    void visit(visitor &v) {
    }

    /// Set the uniforms for this material.
    void render(const mat4t &modelToProjection, const mat4t &modelToCamera, vec4 *light_uniforms, int num_light_uniforms, int num_lights) {
      /*char tmp[256];
      log("lu[0] = %s\n", light_uniforms[0].toString(tmp, sizeof(tmp)));
      log("lu[1] = %s\n", light_uniforms[1].toString(tmp, sizeof(tmp)));
      log("lu[2] = %s\n", light_uniforms[2].toString(tmp, sizeof(tmp)));
      log("lu[3] = %s\n", light_uniforms[3].toString(tmp, sizeof(tmp)));*/
      {
        // matrices and lighting go in the dynamic uniform buffer
        param_uniform *modelToProjection_param = get_param_uniform(atom_modelToProjection);
        if (modelToProjection_param) modelToProjection_param->set_value(buffer.data(), modelToProjection.get(), sizeof(modelToProjection));

        param_uniform *modelToCamera_param = get_param_uniform(atom_modelToCamera);
        if (modelToCamera_param) modelToCamera_param->set_value(buffer.data(), modelToCamera.get(), sizeof(modelToCamera));

        param_uniform *lighting_param = get_param_uniform(atom_lighting);
        if (lighting_param) lighting_param->set_value(buffer.data(), light_uniforms, sizeof(vec4) * num_light_uniforms);

        param_uniform *num_lights_param = get_param_uniform(atom_num_lights);
        if (num_lights_param) num_lights_param->set_value(buffer.data(), &num_lights, sizeof(int32_t));
      }

      custom_shader->render();

      {
        // colours and textures go in the static uniform buffer
        for (unsigned i = 0; i != params.size(); ++i) {
          param_uniform *pu = params[i]->get_param_uniform();
          if (pu) {
            //printf("%s: %d off=%x\n", app_utils::get_atom_name(pu->get_name()), pu->get_uniform_buffer_index(), pu->get_offset());
            pu->render(buffer.data());
          }
        }
      }
    }

    /// Set the uniforms for this material on skinned meshes.
    void render_skinned(const mat4t &cameraToProjection, const mat4t *modelToCamera, int num_nodes, vec4 *light_uniforms, int num_light_uniforms, int num_lights) const {
      //shader.render_skinned(cameraToProjection, modelToCamera, num_nodes, light_uniforms, num_light_uniforms, num_lights);
      //bind_textures();
    }

    /// get a named parameter
    param *get_param(atom_t name) {
      for (unsigned i = 0; i != params.size(); ++i) {
        if (params[i]->get_name() == name) {
          return params[i];
        }
      }
      return NULL;
    }

    /// get a named parameter that is a uniform
    param_uniform *get_param_uniform(atom_t name) {
      param *param = get_param(name);
      return param ? param->get_param_uniform() : NULL;
    }

    /// set the diffuse color parameter (if it exists)
    void set_diffuse(const vec4 &color) {
      if (param *p = get_param_uniform(atom_diffuse)) {
        p->get_param_uniform()->set_value(buffer.data(), &color, sizeof(color));
      }
    }

    void set_uniform(param_uniform *param, const void *data, size_t size) {
      memcpy(buffer.data() + param->get_offset(), data, size);
    }

    dynarray<ref<param> > &get_params() {
      return params;
    }

    param_uniform *add_uniform(const void *data, atom_t name, uint16_t _type, uint16_t _repeat, param::stage_type _stage=param::stage_fragment) {
      param_buffer_info pbi(buffer);
      param_uniform *result = new param_uniform(pbi, data, name, _type, _repeat, _stage);
      params.push_back(result);

      param_bind_info pbind;
      pbind.program = custom_shader->get_program();
      result->bind(pbind);
      return result;
    }

    param_sampler *add_sampler(GLint texture_slot, atom_t name, image *_image, sampler *_sampler, param::stage_type _stage=param::stage_fragment) {
      param_buffer_info pbi(buffer);
      pbi.texture_slot = texture_slot;
      param_sampler *result = new param_sampler(pbi, name, _image, _sampler, _stage);
      params.push_back(result);

      param_bind_info pbind;
      pbind.program = custom_shader->get_program();
      result->bind(pbind);
      return result;
    }
  };
}}

