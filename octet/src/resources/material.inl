////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Material: this inl file solves circular dependencies for the render function
//

inline void material::render(bump_shader &shader, const mat4 &modelToProjection, const mat4 &modelToCamera, const vec4 &light_direction, vec4 &light_ambient, vec4 &light_diffuse, vec4 &light_specular) {
  shader.render(modelToProjection, modelToCamera, light_direction, shininess, light_ambient, light_diffuse, light_specular);
    
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
