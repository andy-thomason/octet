////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Material 
//

class material {
  // material
  GLuint diffuse;
  GLuint ambient;
  GLuint emission;
  GLuint specular;
  GLuint bump;

  float shininess;

  struct {
  };

public:
  // default constructor makes a blank material.
  material() {
    diffuse = 0;
    ambient = 0;
    emission = 0;
    specular = 0;
    bump = 0;
    shininess = 0;
  }

  void init(
    GLuint diffuse,
    GLuint ambient,
    GLuint emission,
    GLuint specular,
    GLuint bump,
    float shininess
  ) {
    this->diffuse = diffuse;
    this->ambient = ambient;
    this->emission = emission;
    this->specular = specular;
    this->bump = bump;
    this->shininess = shininess;
  }

  void render(bump_shader &shader, const mat4 &modelToProjection, const mat4 &modelToCamera, const vec4 &light_direction, vec4 &light_ambient, vec4 &light_diffuse, vec4 &light_specular) {
    shader.render(modelToProjection, modelToCamera, light_dir, shininess, light_ambient, light_diffuse, light_specular);
    
    // set textures 0, 1, 2, 3 to their respective values
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, ambient);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, emission);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, specular);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, bump);
    glActiveTexture(GL_TEXTURE0);
  }
};

