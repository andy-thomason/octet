////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Phong shader that uses textures for all material channels
// This shader has only one light which 

namespace octet { namespace shaders {
  class phong_shader : public shader {
    // indices to use with glUniform*()

    GLuint modelToProjection_index; // index for model space to projection space matrix
    GLuint modelToCamera_index;     // second matrix used for lighting maps model to camera space
    GLuint cameraToProjection_index;// used in skinned shader
    GLuint light_direction_index;   // direction of light in camera space
    GLuint samplers_index;          // index for texture samplers
    GLuint shininess_index;         // index for shininess factor (~30 is good)
    GLuint light_ambient_index;     // index for ambient light color
    GLuint light_diffuse_index;     // index for diffuse light color
    GLuint light_specular_index;    // index for specular light color

  public:
    void init() {
      // this is the vertex shader.
      // it is called for each corner of each triangle
      // it inputs pos and uv from each corner
      // it outputs gl_Position, normal_ and uv_ to the rasterizer
      // normal_ is the normal in camera space to make calculations easier
      const char vertex_shader[] = SHADER_STR(
        varying vec2 uv_;
        varying vec3 normal_;
      
        attribute vec4 pos;
        attribute vec3 normal;
        attribute vec2 uv;
      
        uniform mat4 modelToProjection;
        uniform mat4 modelToCamera;
      
        void main() {
          uv_ = uv;
          normal_ = (modelToCamera * vec4(normal,0)).xyz;
          gl_Position = modelToProjection * pos;
        }
      );

      // this is the fragment shader
      // after the rasterizer breaks the triangle into fragments
      // this is called for every fragment
      // it outputs gl_FragColor, the color of the pixel and inputs normal_ and uv_
      // the four samplers give emissive, diffuse, specular and ambient colors
      const char fragment_shader[] = SHADER_STR(
        varying vec2 uv_;
        varying vec3 normal_;
        uniform vec3 light_direction;
        uniform vec4 light_diffuse;
        uniform vec4 light_ambient;
        uniform vec4 light_specular;
        uniform sampler2D samplers[4];
        uniform float shininess;
      
        void main() {
          vec3 nnormal = normalize(normal_);
          vec3 half_direction = normalize(light_direction + vec3(0, 0, 1));
          float diffuse_factor = max(dot(light_direction, nnormal), 0.0);
          float specular_factor = pow(max(dot(half_direction, nnormal), 0.0), shininess);
          vec4 diffuse = texture2D(samplers[0], uv_);
          vec4 ambient = texture2D(samplers[1], uv_);
          vec4 emission = texture2D(samplers[2], uv_);
          vec4 specular = texture2D(samplers[3], uv_);
          gl_FragColor = 
            ambient * light_ambient +
            diffuse * light_diffuse * diffuse_factor +
            emission +
            specular * light_specular * specular_factor;
        }
      );
      init_uniforms(vertex_shader, fragment_shader);
    }

    void init_uniforms(const char *vertex_shader, const char *fragment_shader) {
      // use the common shader code to compile and link the shaders
      // the result is a shader program
      shader::init(vertex_shader, fragment_shader);

      // extract the indices of the uniforms to use later
      modelToProjection_index = glGetUniformLocation(program(), "modelToProjection");
      cameraToProjection_index = glGetUniformLocation(program(), "cameraToProjection");
      modelToCamera_index = glGetUniformLocation(program(), "modelToCamera");
      light_direction_index = glGetUniformLocation(program(), "light_direction");
      samplers_index = glGetUniformLocation(program(), "samplers");
      shininess_index = glGetUniformLocation(program(), "shininess");
      light_ambient_index = glGetUniformLocation(program(), "light_ambient");
      light_diffuse_index = glGetUniformLocation(program(), "light_diffuse");
      light_specular_index = glGetUniformLocation(program(), "light_specular");
    }

    void render(const mat4t &modelToProjection, const mat4t &modelToCamera, const vec4 &light_direction, float shininess, vec4 &light_ambient, vec4 &light_diffuse, vec4 &light_specular, int num_samplers=4) {
      // tell openGL to use the program
      shader::render();

      // customize the program with uniforms
      glUniform3fv(light_direction_index, 1, light_direction.get());
      glUniformMatrix4fv(modelToProjection_index, 1, GL_FALSE, modelToProjection.get());
      glUniformMatrix4fv(modelToCamera_index, 1, GL_FALSE, modelToCamera.get());
      glUniform4fv(light_ambient_index, 1, light_ambient.get());
      glUniform4fv(light_diffuse_index, 1, light_diffuse.get());
      glUniform4fv(light_specular_index, 1, light_specular.get());
      glUniform1f(shininess_index, shininess);

      // we use textures 0-3 for material properties.
      static const GLint samplers[] = { 0, 1, 2, 3, 4 };
      glUniform1iv(samplers_index, num_samplers, samplers);
    }

    void render_skinned(const mat4t &cameraToProjection, const mat4t *modelToCamera, int num_matrices, const vec4 &light_direction, float shininess, vec4 &light_ambient, vec4 &light_diffuse, vec4 &light_specular, int num_samplers=4) {
      // tell openGL to use the program
      shader::render();

      // customize the program with uniforms
      glUniform3fv(light_direction_index, 1, light_direction.get());
      glUniformMatrix4fv(cameraToProjection_index, 1, GL_FALSE, cameraToProjection.get());
      glUniformMatrix4fv(modelToCamera_index, num_matrices, GL_FALSE, (float*)modelToCamera);
      glUniform4fv(light_ambient_index, 1, light_ambient.get());
      glUniform4fv(light_diffuse_index, 1, light_diffuse.get());
      glUniform4fv(light_specular_index, 1, light_specular.get());
      glUniform1f(shininess_index, shininess);

      // we use textures 0-3 for material properties.
      static const GLint samplers[] = { 0, 1, 2, 3, 4 };
      glUniform1iv(samplers_index, num_samplers, samplers);
    }
  };
}}
