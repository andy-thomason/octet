////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Common compute shader code - compiles and links compute shaders

// "stringifying macro" makes shaders more readable 
// (effectively puts quotes around X)
#define SHADER_STR(X) #X

namespace octet { namespace shaders {
  class compute_shader : public resource {
    GLuint program_;

    void link(GLuint shader_object) {
      // assemble the program for use by glUseProgram
      GLuint program = glCreateProgram();
      glAttachShader(program, shader_object);
      glLinkProgram(program);

      program_ = program;
      GLsizei length;
      char buf[0x10000];
      glGetProgramInfoLog(program, sizeof(buf), &length, buf);
      if (length) {
        fputs(buf, log("program errors\n"));
      }
    }
  public:
    GLuint program() { return program_; }
  
    compute_shader(const char *url) {
      #ifndef __APPLE__
        dynarray<uint8_t> cs;
        app_utils::get_url(cs, url);
        cs.push_back(0);
        const GLchar *csp = (const GLchar *)cs.data();

        GLsizei length;
        char buf[256];
        // create our vertex shader and compile it
        GLuint shader_object = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(shader_object, 1, &csp, NULL);
        glCompileShader(shader_object);
        glGetShaderInfoLog(shader_object, sizeof(buf), &length, buf);
        if (length) {
          log("%s\nCompute shader error:\n%s\n\n\n\n", cs.data(), buf);
          printf("see log.txt for shader errors\n");
        }
      
        link(shader_object);
      #endif
    }

    // start using the program
    void use() {
      glUseProgram(program_);
    }

    // use the program we have compiled in init()
    void dispatch(size_t x, size_t y = 1, size_t z = 1) {
      #ifndef __APPLE__
        glDispatchCompute((GLuint)x, (GLuint)y, (GLuint)z);
      #endif
    }

    /// get the OpenGL program object.
    GLuint get_program() const {
      return program_;
    }
  };

}}
