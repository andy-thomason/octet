////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Common shader code - compiles and links shaders

// "stringifying macro" makes shaders more readable 
// (effectively puts quotes around X)
#define SHADER_STR(X) #X

namespace octet {
  class shader {
    GLuint program_;
  public:
    shader() {}

    GLuint program() { return program_; }
  
    void init(const char *vs, const char *fs) {
      printf("creating shader program\n");

      GLsizei length;
      char buf[256];
      // create our vertex shader and compile it
      GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
      glShaderSource(vertex_shader, 1, &vs, NULL);
      glCompileShader(vertex_shader);
      glGetShaderInfoLog(vertex_shader, sizeof(buf), &length, buf);
      puts(buf);
    
      // create our fragment shader and compile it
      GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(fragment_shader, 1, &fs, NULL);
      glCompileShader(fragment_shader);
      glGetShaderInfoLog(fragment_shader, sizeof(buf), &length, buf);
      puts(buf);

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
      glGetProgramInfoLog(program, sizeof(buf), &length, buf);
      puts(buf);
    }
  
    // use the program we have compiled in init()
    void render() {
      glUseProgram(program_);
    }
  };

}
