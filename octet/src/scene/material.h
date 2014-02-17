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

    // storage for static uniforms such as colours and samplers
    ref<gl_resource> static_buffer;

    // storage for dynamic uniforms such as matrices and lighting
    ref<gl_resource> dynamic_buffer;

  public:
    RESOURCE_META(material)

    /// Default constructor makes a blank material.
    material() {
    }

    /// Alternative constructor.
    material(const vec4 &color) {
      // materials are constructed from parameters which build the final shader.
      // this allows us to use OpenGLES2 (uniforms) and 3 (buffers) as well as new shader features.
      static_buffer = new gl_resource(GL_UNIFORM_BUFFER, 256);
      dynamic_buffer = new gl_resource(GL_UNIFORM_BUFFER, 256);
      param_buffer_info dynamic_pbi(dynamic_buffer, 0);
      param_buffer_info static_pbi(static_buffer, 1);
      param_uniform *modelToProjection = new param_uniform(dynamic_pbi, NULL, atom_modelToProjection, GL_FLOAT_MAT4, 1, param::stage_vertex);
      param_uniform *modelToCamera = new param_uniform(dynamic_pbi, NULL, atom_modelToCamera, GL_FLOAT_MAT4, 1, param::stage_vertex);
      param_color *diffuse = new param_color(static_pbi, color, atom_diffuse, param::stage_fragment);
      param_attribute *pos = new param_attribute(atom_pos, GL_FLOAT_VEC4);
      param_attribute *uv = new param_attribute(atom_uv, GL_FLOAT_VEC2);
      param_op *gl_Position = new param_op(atom_gl_Position, 0, modelToProjection, pos, param::op_mul, param::stage_vertex);
      param_op *gl_FragColor = new param_op(atom_gl_FragColor, 0, diffuse, NULL, param::op_copy, param::stage_fragment);

      params.reserve(8);
      params.push_back(modelToProjection);
      params.push_back(modelToCamera);
      params.push_back(pos);
      params.push_back(uv);
      params.push_back(gl_Position);
      params.push_back(diffuse);
      params.push_back(gl_FragColor);

      custom_shader = new param_shader(params);
    }

    /// create a material from an existing image
    material(image *img) {
      static_buffer = new gl_resource(GL_UNIFORM_BUFFER, 256);
      dynamic_buffer = new gl_resource(GL_UNIFORM_BUFFER, 256);
      param_buffer_info dynamic_pbi(dynamic_buffer, 0);
      param_buffer_info static_pbi(static_buffer, 1);
      param_uniform *modelToProjection = new param_uniform(dynamic_pbi, NULL, atom_modelToProjection, GL_FLOAT_MAT4, 1, param::stage_vertex);
      param_uniform *modelToCamera = new param_uniform(dynamic_pbi, NULL, atom_modelToCamera, GL_FLOAT_MAT4, 1, param::stage_vertex);
      param_attribute *pos = new param_attribute(atom_pos, GL_FLOAT_VEC4);
      param_attribute *uv = new param_attribute(atom_uv, GL_FLOAT_VEC2);
      param_attribute *noral = new param_attribute(atom_normal, GL_FLOAT_VEC2);

      param_sampler *diffuse_sampler = new param_sampler(static_pbi, atom_diffuse_sampler, img, new sampler(), param::stage_fragment);
      param_op *diffuse = new param_op(atom_diffuse, GL_FLOAT_VEC4, diffuse_sampler, uv, param::op_texture2D, param::stage_fragment);

      param_op *gl_Position = new param_op(atom_gl_Position, 0, modelToProjection, pos, param::op_mul, param::stage_vertex);
      param_op *gl_FragColor = new param_op(atom_gl_FragColor, 0, diffuse, NULL, param::op_copy, param::stage_fragment);

      params.reserve(8);
      params.push_back(modelToProjection);
      params.push_back(modelToCamera);
      params.push_back(pos);
      params.push_back(uv);
      params.push_back(gl_Position);
      params.push_back(diffuse_sampler);
      params.push_back(diffuse);
      params.push_back(gl_FragColor);

      custom_shader = new param_shader(params);
    }

    material(param *diffuse, param *ambient, param *emission, param *specular, param *bump, param *shininess) {
    }

    /// Serialize.
    void visit(visitor &v) {
    }

    /// Set the uniforms for this material.
    void render(const mat4t &modelToProjection, const mat4t &modelToCamera, vec4 *light_uniforms, int num_light_uniforms, int num_lights) {
      {
        // matrices and lighting go in the dynamic uniform buffer
        gl_resource::wolock dynamic_lock(dynamic_buffer);
        param_uniform *modelToProjection_param = params[0]->get_param_uniform();
        if (modelToProjection_param) modelToProjection_param->set_value(dynamic_lock.u8(), modelToProjection.get(), sizeof(modelToProjection));

        param_uniform *modelToCamera_param = params[1]->get_param_uniform();
        if (modelToCamera_param) modelToCamera_param->set_value(dynamic_lock.u8(), modelToCamera.get(), sizeof(modelToCamera));
      }

      custom_shader->render();

      {
        // colours and textures go in the static uniform buffer
        gl_resource::rolock static_lock(static_buffer);
        gl_resource::rolock dynamic_lock(dynamic_buffer);

        for (unsigned i = 0; i != params.size(); ++i) {
          param_uniform *pu = params[i]->get_param_uniform();
          if (pu) {
            //printf("%s: %d\n", pu->get_name(), pu->get_uniform_buffer_index());
            if (pu->get_uniform_buffer_index()) {
              pu->render(static_lock.u8());
            } else {
              pu->render(dynamic_lock.u8());
            }
          }
        }
      }
    }

    /// Set the uniforms for this material on skinned meshes.
    void render_skinned(const mat4t &cameraToProjection, const mat4t *modelToCamera, int num_nodes, vec4 *light_uniforms, int num_light_uniforms, int num_lights) const {
      //shader.render_skinned(cameraToProjection, modelToCamera, num_nodes, light_uniforms, num_light_uniforms, num_lights);
      //bind_textures();
    }

  };
}}

