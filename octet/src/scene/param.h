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
  struct param_bind_info;
  struct param_compose_info;
  struct param_buffer_info;

  class param : public resource {
  public:
    enum op_type {
      op_texture2D,

      op_normalize,

      op_mul,
      op_cast,
      op_copy,
    };
  
    enum stage_type {
      stage_vertex,
      stage_fragment,
      stage_max
    };

  private:
    atom_t name;
    stage_type stage_;
    uint16_t type;
  public:

    param(atom_t _name=atom_, uint16_t _type=0, stage_type _stage=stage_fragment) : name(_name), type(_type), stage_(_stage) {
    }

    virtual void compose(param_compose_info &pci) {
    }

    virtual void bind(param_bind_info &pbi) {
    }

    virtual void render(const uint8_t *buffer) {
    }

    const char *compose_glue(param_compose_info &pci, stage_type dest_stage, char *tmp, size_t size);

    const char *get_atom_name() const {
      return app_utils::get_atom_name(name);
    }

    atom_t get_name() const {
      return name;
    }

    uint16_t get_gl_type() {
      return type;
    }

    const char *get_param_type() const {
      switch (type) {
        case GL_BYTE: return "char";
        case GL_UNSIGNED_BYTE: return "unsigned char";
        case GL_SHORT: return "short";
        case GL_UNSIGNED_SHORT: return "unsigned short";
        case GL_INT: return "int";
        case GL_UNSIGNED_INT: return "unsigned int";
        case GL_FLOAT: return "float";
        case GL_FLOAT_VEC2: return "vec2";
        case GL_FLOAT_VEC3: return "vec3";
        case GL_FLOAT_VEC4: return "vec4";
        case GL_INT_VEC2: return "ivec2";
        case GL_INT_VEC3: return "ivec3";
        case GL_INT_VEC4: return "ivec4";
        case GL_BOOL: return "bool";
        case GL_BOOL_VEC2: return "bvec2";
        case GL_BOOL_VEC3: return "bvec3";
        case GL_BOOL_VEC4: return "bvec4";
        case GL_FLOAT_MAT2: return "mat2";
        case GL_FLOAT_MAT3: return "mat3";
        case GL_FLOAT_MAT4: return "mat4";
        case GL_SAMPLER_2D: return "sampler2D";
        case GL_SAMPLER_CUBE: return "samplerCube";
        case GL_SAMPLER_3D: return "sampler3D";
        case GL_SAMPLER_2D_SHADOW: return "sampler2Dshadow";
        default: return "";
      }
    }

    stage_type get_stage() const {
      return stage_;
    }
  };

  struct param_bind_info {
    GLint program;
  };

  struct param_compose_info {
    dynarray <char> decls[param::stage_max];
    dynarray <char> code[param::stage_max];
    dynarray<ref<param> > &params;

    param_compose_info(dynarray<ref<param> > &_params) : params(_params) {}

    param *get_param(atom_t name);
  };

  struct param_buffer_info {
    gl_resource *buffer;
    GLint texture_slot;
    unsigned size;
    gl_resource::rwlock lock;
    uint8_t uniform_buffer;

    param_buffer_info(gl_resource *_buffer, uint8_t _uniform_buffer) : lock(_buffer) {
      buffer =_buffer;
      texture_slot = 0;
      size = 0;
      uniform_buffer = _uniform_buffer;
    }
  };

  /// generate glue between stages
  inline const char *param::compose_glue(param_compose_info &pci, stage_type dest_stage, char *tmp, size_t size) {
    if (get_stage() == dest_stage) return get_atom_name();

    // todo: handle more than one stage!
    format(pci.decls[get_stage()], "varying %s %s_;\n", get_param_type(), get_atom_name());
    format(pci.code[get_stage()], "  %s_ = %s;\n", get_atom_name(), get_atom_name());
    format(pci.decls[dest_stage], "varying %s %s_;\n", get_param_type(), get_atom_name());

    snprintf(tmp, size, "%s_", get_atom_name());
    return tmp;
  }

  /// Uniform parameter class.
  ///
  /// Uniforms are used to send parameters to OpenGL that do not change between triangles or fragments.
  ///
  /// For OpenGL ES2 we keep uniforms in a dynarray and use glUniform* to copy them to OpenGL.
  /// For OpenGL ES3 we keep uniforms in a uniform buffer and use the buffer.
  /// The parameter uniform records the location, name and type of the uniform as well as the repeat count for arrays.
  class param_uniform : public param {
    GLint uniform;           // uniform index
    uint16_t offset;         // offset in uniform buffer
    uint16_t repeat;         // how many in array?
    uint8_t uniform_buffer;  // Which uniform buffer? 0 = dynamic, 1 = static.
  public:
    RESOURCE_META(param_uniform)

    param_uniform() {
    }

    /// create a new uniform parameter with a prototype in "buffer"
    /// "data" may be NULL and you must use app_utils::get_atom() for the name or one of the atom_ enums (see atom.h)
    param_uniform(param_buffer_info &pbi, const void *data, atom_t name, uint16_t _type, uint16_t _repeat, stage_type _stage=stage_fragment) :
      param(name, _type, _stage)
    {
      repeat = _repeat;
      offset = (uint32_t)pbi.size;
      uniform_buffer = pbi.uniform_buffer;

      // in uniform buffers, everything is in units of 16 bytes
      // matrices are repeats of vec4s
      unsigned size = _repeat * 16;
      switch (_type) {
        case GL_FLOAT_MAT2: size *= 2; break;
        case GL_FLOAT_MAT3: size *= 3; break;
        case GL_FLOAT_MAT4: size *= 4; break;
      }

      pbi.size += size;

      assert(pbi.size <= pbi.buffer->get_size() && "Increase the uniform buffer size for this material");

      // if data is non-null, we add to the "buffer" part of pbi. (eg. colors)
      // otherwise we just grow the buffer size (eg. matrices, lighting)
      if (!data) return;

      int32_t *idest = (int32_t*)(pbi.lock.u8() + offset);
      const int32_t *isrc = (const int32_t*)data;

      switch (get_gl_type()) {
        case GL_FLOAT: idest[0] = isrc[0]; break;
        case GL_FLOAT_VEC2: idest[0] = isrc[0]; idest[1] = isrc[1]; break;
        case GL_FLOAT_VEC3: idest[0] = isrc[0]; idest[1] = isrc[1]; idest[2] = isrc[2]; break;
        case GL_FLOAT_VEC4: idest[0] = isrc[0]; idest[1] = isrc[1]; idest[2] = isrc[2]; idest[3] = isrc[3]; break;

        case GL_SAMPLER_2D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_2D_SHADOW:
        case GL_INT:
        case GL_BOOL: 
        case GL_UNSIGNED_INT: idest[0] = isrc[0]; break;
        case GL_BOOL_VEC2: case GL_INT_VEC2: idest[0] = isrc[0]; idest[1] = isrc[1]; break;
        case GL_BOOL_VEC3: case GL_INT_VEC3: idest[0] = isrc[0]; idest[1] = isrc[1]; idest[2] = isrc[2]; break;
        case GL_BOOL_VEC4: case GL_INT_VEC4: idest[0] = isrc[0]; idest[1] = isrc[1]; idest[2] = isrc[2]; idest[3] = isrc[3]; break;

        case GL_FLOAT_MAT2: idest[0] = isrc[0]; idest[1] = isrc[1]; idest[2] = isrc[2]; idest[3] = isrc[3]; break;
        case GL_FLOAT_MAT3: memcpy(idest, isrc, 9*sizeof(float)); break;
        case GL_FLOAT_MAT4: memcpy(idest, isrc, 16*sizeof(float)); break;

        default: abort();
      }
    }

    /// generate source code for the shader
    void compose(param_compose_info &pci) {
      if (repeat > 1) {
        format(pci.decls[get_stage()], "uniform %s %s[%d];\n", get_param_type(), get_atom_name(), repeat);
      } else {
        format(pci.decls[get_stage()], "uniform %s %s;\n", get_param_type(), get_atom_name());
      }
    }

    /// connect the parameter to the shader
    void bind(param_bind_info &pbi) {
      uniform = glGetUniformLocation(pbi.program, get_atom_name());
      //log("bind %d %s\n", uniform, get_atom_name());
    }

    /// get the uniform location
    GLint get_uniform() const {
      return uniform;
    }

    /// if buffer is a pointer to a uniform buffer, set the value in the correct place.
    void set_value(uint8_t *buffer, const void *value, unsigned size) {
      memcpy(buffer + offset, value, size);
    }

    /// if buffer is a pointer to a uniform buffer, fetch the value from that location
    const void *get_value(const uint8_t *buffer) const {
      return buffer + offset;
    }

    /// which uniform buffer does this parameter belong to?
    uint8_t get_uniform_buffer_index() {
      return uniform_buffer;
    }

    /// for OpenGL ES2, call glUniform* to copy the uniform to the GPU command buffer.
    /// for OpenGL ES3, we can use the uniform buffer directly and so don't need this.
    void render(const uint8_t *buffer) {
      GLint uni = get_uniform();

      if (uni == -1) return;

      switch (get_gl_type()) {
        case GL_FLOAT: glUniform1fv(uni, repeat, (float*)(buffer + offset)); break;
        case GL_FLOAT_VEC2: glUniform2fv(uni, repeat, (float*)(buffer + offset)); break;
        case GL_FLOAT_VEC3: glUniform3fv(uni, repeat, (float*)(buffer + offset)); break;
        case GL_FLOAT_VEC4: glUniform4fv(uni, repeat, (float*)(buffer + offset)); break;

        case GL_SAMPLER_2D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_2D_SHADOW:
        case GL_INT:
        case GL_BOOL: 
        case GL_UNSIGNED_INT: glUniform1iv(uni, repeat, (GLint*)(buffer + offset)); break;
        case GL_BOOL_VEC2: case GL_INT_VEC2: glUniform2iv(uni, repeat, (GLint*)(buffer + offset)); break;
        case GL_BOOL_VEC3: case GL_INT_VEC3: glUniform3iv(uni, repeat, (GLint*)(buffer + offset)); break;
        case GL_BOOL_VEC4: case GL_INT_VEC4: glUniform4iv(uni, repeat, (GLint*)(buffer + offset)); break;

        case GL_FLOAT_MAT2: glUniformMatrix2fv(uni, repeat, GL_FALSE, (float*)(buffer + offset)); break;
        case GL_FLOAT_MAT3: glUniformMatrix3fv(uni, repeat, GL_FALSE, (float*)(buffer + offset)); break;
        case GL_FLOAT_MAT4: glUniformMatrix4fv(uni, repeat, GL_FALSE, (float*)(buffer + offset)); break;

        default: abort();
      }
    }
  };

  /// Attribute parameter - eg. pos, uv etc.
  class param_attribute : public param {
  public:
    RESOURCE_META(param_attribute)

    param_attribute() {
    }

    /// Construct an attribute parameter to represent data from the geometry.
    param_attribute(atom_t name, uint16_t _type) :
      param(name, _type, stage_vertex) {
    }

    /// Build the shader given this attribute parameter.
    void compose(param_compose_info &pci) {
      format(pci.decls[get_stage()], "attribute %s %s;\n", get_param_type(), get_atom_name());
    }
  };

  /// Color parameter: convenience operator for a vec4 in a buffer
  class param_color : public param_uniform {
  public:
    RESOURCE_META(param_color)

    param_color() {
    }

    /// construct a color parameter, building a uniform buffer as we go.
    param_color(param_buffer_info &pbi, vec4_in _value, atom_t name, stage_type _stage) :
      param_uniform(pbi, &_value, name, GL_FLOAT_VEC4, 1, _stage)
    {
    }

    /// get the color
    vec4 get_value(const uint8_t *buffer) const {
      return *(const vec4*)param_uniform::get_value(buffer);
    }

    /// set the color
    void set_value(uint8_t *buffer, vec4_in _value) {
      param_uniform::set_value(buffer, &_value, 16);
    }
  };

  /// Sampler parameter which includes image and sampler setup data.
  class param_sampler : public param_uniform {
    ref<image> image_;
    ref<sampler> sampler_;
    GLuint texture_slot;
  public:
    RESOURCE_META(param_sampler)

    param_sampler() {
    }

    /// constuct a sampler parameter, allocating a texture slot and adding it to a prototype uniform buffer.
    param_sampler(param_buffer_info &pbi, atom_t name, image *_image, sampler *_sampler, stage_type _stage) :
      param_uniform(pbi, &pbi.texture_slot, name, GL_SAMPLER_2D, 1, _stage), image_(_image), sampler_(_sampler)
    {
      texture_slot = pbi.texture_slot++;
    }

    /// Set the OpenGL state for this sampler.
    void render(const uint8_t *buffer) {
      param_uniform::render(buffer);
      glActiveTexture(GL_TEXTURE0 + texture_slot);
      glBindTexture(sampler_->get_gl_target(), sampler_->get_gl_texture(image_));

      //log("%s: %d=%d %04x %d\n", get_atom_name(), get_uniform(), texture_slot, sampler_->get_gl_target(), sampler_->get_gl_texture(image_));
    }
  };

  /// General purpose operation "parameter"
  class param_op : public param {
    atom_t lhs;
    atom_t rhs;
    param::op_type op;
  public:
    RESOURCE_META(param_op)

    param_op() {
      op = param::op_copy;
    }

    param_op(atom_t name, uint16_t type, atom_t _lhs, atom_t _rhs, param::op_type _op, stage_type _stage) :
      param(name, type, _stage), lhs(_lhs), rhs(_rhs), op(_op)
    {
    }

    /// Generate shader code for this operation.
    void compose(param_compose_info &pci) {
      const char *func = "";
      char tmp[2][256];
      param *lhs_param = pci.get_param(lhs);
      param *rhs_param = pci.get_param(rhs);
      assert(lhs_param && "missing lhs parameter");
      const char *lhs_name = lhs_param->compose_glue(pci, get_stage(), tmp[0], sizeof(tmp[0]));
      const char *rhs_name = rhs_param ? rhs_param->compose_glue(pci, get_stage(), tmp[1], sizeof(tmp[1])) : "";
      switch (op) {
        case op_normalize: func = "normalize"; goto func1;
        func1: {
          format(pci.code[get_stage()], "  %s %s = %s(%s);\n", get_param_type(), get_atom_name(), func, lhs_name);
        } break;

        case op_texture2D: func = "texture2D"; goto func2;
        func2: {
          format(pci.code[get_stage()], "  %s %s = %s(%s, %s);\n", get_param_type(), get_atom_name(), func, lhs_name, rhs_name);
        } break;

        case op_mul: func = "*"; goto op2;
        op2: {
          format(pci.code[get_stage()], "  %s %s = %s %s %s;\n", get_param_type(), get_atom_name(), lhs_name, func, rhs_name);
        } break;

        case op_cast: {
          if (rhs) {
            format(pci.code[get_stage()], "  %s %s = %s(%s, %s);\n", get_param_type(), get_atom_name(), get_param_type(), lhs_name, rhs_name);
          } else {
            format(pci.code[get_stage()], "  %s %s = %s(%s);\n", get_param_type(), get_atom_name(), get_param_type(), lhs_name);
          }
        } break;

        case op_copy: {
          format(pci.code[get_stage()], "  %s %s = %s;\n", get_param_type(), get_atom_name(), lhs_name);
        } break;

        default: {
          abort();
        } break;
      }
    }
  };

  /// custom parameter: insert your own shader code here
  class param_custom : public param {
    string decls;
    string code;
  public:
    RESOURCE_META(param_custom)

    param_custom() {
    }

    param_custom(atom_t _name, uint16_t _type, const char *_decls, const char *_code, param::stage_type _stage) :
      param(_name, _type, _stage), decls(_decls), code(_code)
    {
    }

    void compose(param_compose_info &pci) {
      format(pci.decls[get_stage()], "%s", decls.c_str());
      format(pci.code[get_stage()], "%s", code.c_str());
    }
  };

  /// Shader that is constructed from parameters.
  class param_shader : public shader {
  public:
    RESOURCE_META(param_shader)

    param_shader() {
    }

    param_shader(
      dynarray<ref<param> > &params,
      const char *vertex = "%s\nvoid main() {\n%s\n}\n",
      const char *fragment = "%s\nvoid main() {\n%s\n}\n"
    ) {
      param_compose_info pci(params);

      for (unsigned i = 0; i != param::stage_max; ++i) {
        pci.decls[i].reserve(0x1000);
        pci.code[i].reserve(0x1000);
      }

      for (unsigned i = 0; i != params.size(); ++i) {
        params[i]->compose(pci);
      }

      dynarray<char> vertex_shader;
      format(vertex_shader, vertex, pci.decls[0].data(), pci.code[0].data());

      dynarray<char> fragment_shader;
      format(fragment_shader, fragment, pci.decls[1].data(), pci.code[1].data());
      log("vs\n%s\n\n", vertex_shader.data());
      log("fs\n%s\n\n", fragment_shader.data());
      shader::init(vertex_shader.data(), fragment_shader.data());

      param_bind_info pbi;
      pbi.program = get_program();

      for (unsigned i = 0; i != params.size(); ++i) {
        params[i]->bind(pbi);
      }
    }
  };


  inline param *param_compose_info::get_param(atom_t name) {
    for (unsigned i = 0; i != params.size(); ++i) {
      if (params[i]->get_name() == name) {
        return params[i];
      }
    }
    return NULL;
  }
}}

