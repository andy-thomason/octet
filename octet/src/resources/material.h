////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2013
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

  void render(class bump_shader &shader, const mat4 &modelToProjection, const mat4 &modelToCamera, const vec4 &light_direction, vec4 &light_ambient, vec4 &light_diffuse, vec4 &light_specular);
};

