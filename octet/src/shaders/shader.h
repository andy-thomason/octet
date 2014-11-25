////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Common shader code - compiles and links shaders

// "stringifying macro" makes shaders more readable 
// (effectively puts quotes around X)
#define SHADER_STR(X) #X

namespace octet { namespace shaders {
  class shader : public resource {
    GLuint program_;

    void link(GLuint vertex_shader, GLuint fragment_shader) {
          // assemble the program for use by glUseProgram
      GLuint program = glCreateProgram();
      glAttachShader(program, vertex_shader);
      glAttachShader(program, fragment_shader);

      // standardize the attribute slots (in NVidia's CG you can do this in the shader)
      glBindAttribLocation(program, attribute_pos, "pos");
      glBindAttribLocation(program, attribute_normal, "normal");
      glBindAttribLocation(program, attribute_tangent, "tangent");
      glBindAttribLocation(program, attribute_bitangent, "bitangent");
      glBindAttribLocation(program, attribute_blendweight, "blendweight");
      glBindAttribLocation(program, attribute_blendindices, "blendindices");
      glBindAttribLocation(program, attribute_color, "color");
      glBindAttribLocation(program, attribute_uv, "uv");
      glLinkProgram(program);

      program_ = program;
      GLsizei length;
      char buf[0x10000];
      glGetProgramInfoLog(program, sizeof(buf), &length, buf);
      if (length) {
        fputs(buf, log("program errors during linking\n"));
        printf("program errors during linking: check log\n");
      } else {
        printf("linked ok\n");
      }
    }
  public:
    shader() {
    }

    GLuint program() { return program_; }
  
    void init(const char *vs, const char *fs) {
      //printf("creating shader program\n");

      GLsizei length;
      char buf[0x10000];
      // create our vertex shader and compile it
      GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
      glShaderSource(vertex_shader, 1, &vs, NULL);
      glCompileShader(vertex_shader);
      glGetShaderInfoLog(vertex_shader, sizeof(buf), &length, buf);
      if (length) {
        log("Vertex shader error:\n%s\n%s\n\n\n\n", buf, vs);
      }
    
      // create our fragment shader and compile it
      GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(fragment_shader, 1, &fs, NULL);
      glCompileShader(fragment_shader);
      glGetShaderInfoLog(fragment_shader, sizeof(buf), &length, buf);
      if (length) {
        log("Fragment shader error:\n%s\n%s\n\n\n\n", buf, fs);
      }

      link(vertex_shader, fragment_shader);
    }

    /// create a program from pre-compiled binary code. (ie. PS Vita)  
    void init_bin(const uint8_t *vs, const uint8_t *fs) {
      #if OCTET_VITA
        printf("creating binary shader program\n");

        GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderBinary(1, &vertex_shader, 0, vs, 0);
    
        GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderBinary(1, &fragment_shader, 0, fs, 0);

        link(vertex_shader, fragment_shader);
      #endif
    }

    // use the program we have compiled in init()
    void render() {
      glUseProgram(program_);
    }

    /// get the OpenGL program object.
    GLuint get_program() const {
      return program_;
    }
  };

}}
