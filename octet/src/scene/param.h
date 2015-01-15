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
  struct param_buffer_info;

  class param : public resource {
  public:
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

    virtual void bind(param_bind_info &pbi) {
    }

    virtual void render(const uint8_t *buffer) {
    }

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

  struct param_buffer_info {
    GLint texture_slot;
    dynarray<uint8_t> &buffer;

    param_buffer_info(dynarray<uint8_t> &buffer) : buffer(buffer) {
      texture_slot = 0;
    }
  };

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

      // in uniform buffers, everything is in units of 16 bytes
      // matrices are repeats of vec4s
      unsigned size = _repeat * 16;
      switch (_type) {
        case GL_FLOAT_MAT2: size *= 2; break;
        case GL_FLOAT_MAT3: size *= 3; break;
        case GL_FLOAT_MAT4: size *= 4; break;
      }

      //pbi.size += size;
      offset = pbi.buffer.size();
      pbi.buffer.resize(offset + size);

      // if data is non-null, we add to the "buffer" part of pbi. (eg. colors)
      // otherwise we just grow the buffer size (eg. matrices, lighting)
      if (!data) return;

      int32_t *idest = (int32_t*)(pbi.buffer.data() + offset);
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

    /// connect the parameter to the shader
    void bind(param_bind_info &pbi) {
      uniform = glGetUniformLocation(pbi.program, get_atom_name());
      //log("bind %d %s\n", uniform, get_atom_name());
    }

    /// get the uniform location
    GLint get_uniform() const {
      return uniform;
    }

    unsigned get_offset() const {
      return offset;
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
      param_uniform(pbi, &pbi.texture_slot, name, _sampler->get_sampler_type(), 1, _stage), image_(_image), sampler_(_sampler)
    {
      texture_slot = pbi.texture_slot++;
    }

    /// Set the OpenGL state for this sampler.
    void render(const uint8_t *buffer) {
      param_uniform::render(buffer);
      glActiveTexture(GL_TEXTURE0 + texture_slot);
      glBindTexture(sampler_->get_gl_target(), sampler_->get_gl_texture(image_));

      //log("%s: u%d=ts%d targ=%04x tex=%d\n", get_atom_name(), get_uniform(), texture_slot, sampler_->get_gl_target(), sampler_->get_gl_texture(image_));
    }
  };

  /// Shader that uses parameters.
  class param_shader : public shader {
    std::string vertex_shader;
    std::string fragment_shader;

  public:
    RESOURCE_META(param_shader)

    param_shader() {
    }

    param_shader(const char *vs_url, const char *fs_url) {
      dynarray<uint8_t> vs;
      dynarray<uint8_t> fs;
      app_utils::get_url(vs, vs_url);
      app_utils::get_url(fs, fs_url);

      vertex_shader.assign((const char*)vs.data(), (const char*)(vs.data() + vs.size()));
      fragment_shader.assign((const char*)fs.data(), (const char*)(fs.data() + fs.size()));
    }

    void init(dynarray<ref<param> > &params) {
      shader::init(vertex_shader.data(), fragment_shader.data());

      param_bind_info pbi;
      pbi.program = get_program();

      for (unsigned i = 0; i != params.size(); ++i) {
        params[i]->bind(pbi);
      }
    }
  };
}}

