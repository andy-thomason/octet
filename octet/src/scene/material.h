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

    // create the parameters that change frequently such as the matrices and lighting
    void create_dynamic_params() {
      static_buffer = new gl_resource(0, 0x100);
      dynamic_buffer = new gl_resource(0, 0x400);
      param_buffer_info dynamic_pbi(dynamic_buffer, 0);
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

    // create the final output of the vertex shader
    void create_transform() {
      //param *modelToProjection = get_param(atom_modelToProjection);
      params.push_back(new param_op(atom_gl_Position, 0, atom_modelToProjection, atom_pos, param::op_mul, param::stage_vertex));

      if (get_param(atom_modelToCamera) && get_param(atom_normal)) {
        params.push_back(new param_custom(atom_tnormal, GL_FLOAT_VEC3, "", "  vec3 tnormal = (modelToCamera * vec4(normal, 0.0)).xyz;\n", param::stage_vertex));
        //params.push_back(new param_op(atom_tnormal, GL_FLOAT_VEC3, atom_modelToCamera, atom_pos, param::op_mul, param::stage_vertex));
      }

      params.push_back(new param_custom(atom_tpos, GL_FLOAT_VEC3, "", "  vec3 tpos = (modelToCamera * pos).xyz;\n", param::stage_vertex));
    }

    // create the loop for generating the lighting
    void create_lighting() {
      if (!get_param(atom_lighting) || !get_param(atom_num_lights)) return;

      if (get_param(atom_tnormal) && get_param(atom_diffuse)) {
        if (!get_param(atom_nnormal)) {
          params.push_back(new param_op(atom_nnormal, GL_FLOAT_VEC3, atom_tnormal, atom_, param::op_normalize, param::stage_fragment));
        }
        params.push_back(new param_op(atom_npos, GL_FLOAT_VEC3, atom_tpos, atom_, param::op_copy, param::stage_fragment));

        params.push_back(
          new param_custom(
            atom_diffuse_light,
            GL_FLOAT_VEC3,
            "",
            "  vec3 diffuse_light = lighting[0].xyz;\n"
            "  for (int i = 0; i != num_lights; ++i) {\n"
            "    vec3 light_pos = lighting[i * 4 + 1].xyz;\n"
            "    vec3 light_direction = lighting[i * 4 + 2].xyz;\n"
            "    vec3 light_color = lighting[i * 4 + 3].xyz;\n"
            "    vec3 light_atten = lighting[i * 4 + 4].xyz;\n"
            "    float diffuse_factor = max(dot(light_direction, nnormal), 0.0);\n"
            //"    float atten = 1.0 / (0.0001 * dot(light_pos - npos, light_pos - npos));\n"
            //"    float atten = dot(light_pos - npos, light_pos - npos) > 1000.0 ? 0.0 : 1.0;\n"
            "    diffuse_light += diffuse_factor * light_color;\n"
            "  }\n",
            param::stage_fragment
          )
        );
      }

      if (get_param(atom_normal) && get_param(atom_specular) && get_param(atom_shininess)) {
        if (!get_param(atom_nnormal)) {
          params.push_back(new param_op(atom_nnormal, GL_FLOAT_VEC3, atom_normal, atom_, param::op_normalize, param::stage_fragment));
        }
        params.push_back(
          new param_custom(
            atom_specular_light,
            GL_FLOAT_VEC3,
            "",
            "  vec3 specular_light = vec3(0.0, 0.0, 0.0);\n"
            "  for (int i = 0; i != num_lights; ++i) {\n"
            "    vec3 light_direction = lighting[i * 4 + 2].xyz;\n"
            "    vec3 light_color = lighting[i * 4 + 3].xyz;\n"
            "    vec3 half_direction = normalize(light_direction + vec3(0, 0, 1));\n"
            "    float specular_factor = pow(max(dot(half_direction, nnormal), 0.0), shininess);\n"
            "    specular_light += specular_factor * light_color;\n"
            "  }\n",
            param::stage_fragment
          )
        );
      }
    }

    // create the final output of the fragment shader
    void create_combiner() {
      if (get_param(atom_diffuse_light) && get_param(atom_diffuse)) {
        params.push_back(
          new param_custom(atom_gl_FragColor, GL_FLOAT_VEC4, "", "  gl_FragColor = vec4(diffuse.xyz * diffuse_light, 1.0);\n", param::stage_fragment)
          //new param_custom(atom_gl_FragColor, GL_FLOAT_VEC4, "", "  gl_FragColor = vec4(lighting[3].xyz, 1.0);\n", param::stage_fragment)
        );
      } else {
        params.push_back(
          new param_custom(atom_gl_FragColor, GL_FLOAT_VEC4, "", "  gl_FragColor = vec4(diffuse.xyz, 1.0);\n", param::stage_fragment)
        );
      }
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
    material(const vec4 &color) {
      // materials are constructed from parameters which build the final shader.
      // this allows us to use OpenGLES2 (uniforms) and 3 (buffers) as well as new shader features.
      params.reserve(16);

      create_dynamic_params();
      create_attribute_params();
      create_transform();

      param_buffer_info static_pbi(static_buffer, 1);
      params.push_back(new param_color(static_pbi, color, atom_diffuse, param::stage_fragment));

      create_lighting();
      create_combiner();

      custom_shader = new param_shader(params);
    }

    /// create a material from an existing image
    material(image *img) {
      params.reserve(16);

      create_dynamic_params();
      create_attribute_params();
      create_transform();

      param_buffer_info static_pbi(static_buffer, 1);
      params.push_back(new param_sampler(static_pbi, atom_diffuse_sampler, img, new sampler(), param::stage_fragment));
      params.push_back(new param_op(atom_diffuse, GL_FLOAT_VEC4, atom_diffuse_sampler, atom_uv, param::op_texture2D, param::stage_fragment));

      create_lighting();
      create_combiner();

      custom_shader = new param_shader(params);
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
        gl_resource::wolock dynamic_lock(dynamic_buffer);
        param_uniform *modelToProjection_param = get_param_uniform(atom_modelToProjection);
        if (modelToProjection_param) modelToProjection_param->set_value(dynamic_lock.u8(), modelToProjection.get(), sizeof(modelToProjection));

        param_uniform *modelToCamera_param = get_param_uniform(atom_modelToCamera);
        if (modelToCamera_param) modelToCamera_param->set_value(dynamic_lock.u8(), modelToCamera.get(), sizeof(modelToCamera));

        param_uniform *lighting_param = get_param_uniform(atom_lighting);
        if (lighting_param) lighting_param->set_value(dynamic_lock.u8(), light_uniforms, sizeof(vec4) * num_light_uniforms);

        param_uniform *num_lights_param = get_param_uniform(atom_num_lights);
        if (num_lights_param) num_lights_param->set_value(dynamic_lock.u8(), &num_lights, sizeof(int32_t));
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

    param *get_param(atom_t name) {
      for (unsigned i = 0; i != params.size(); ++i) {
        if (params[i]->get_name() == name) {
          return params[i];
        }
      }
      return NULL;
    }

    param_uniform *get_param_uniform(atom_t name) {
      param *param = get_param(name);
      return param ? param->get_param_uniform() : NULL;
    }
  };
}}

