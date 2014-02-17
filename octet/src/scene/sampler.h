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
  // Textures need to have control over sampling.
  // for example, we can sample to the "nearest" texel or use
  // "linear" blending between adjacent texels and mip levels
  // (trilinear filtering)
  class sampler : public resource {
    uint16_t texture_mag_filter; // GL_LINEAR_MIPMAP_LINEAR etc.
    uint16_t texture_min_filter; // GL_LINEAR / GL_NEAREST
    uint16_t texture_wrap_s;     // GL_CLAMP / GL_REPEAT
    uint16_t texture_wrap_t;     // GL_CLAMP / GL_REPEAT
    uint16_t texture_wrap_r;     // GL_CLAMP / GL_REPEAT
    uint16_t gl_target;          // GL_TEXTURE_2D, 3D, CUBE_MAP
    string name;                 // name used internally in shader (ie. diffuse_sampler)
    GLuint gl_texture;           // allocted texture

  public:
    RESOURCE_META(sampler)

    // default constructor makes a repeating trilinear filter
    sampler(const char *name_=0) {
      texture_mag_filter = GL_LINEAR_MIPMAP_LINEAR;
      texture_min_filter = GL_LINEAR;
      texture_wrap_s = GL_REPEAT;
      texture_wrap_t = GL_REPEAT;
      texture_wrap_r = GL_REPEAT;
      gl_target = GL_TEXTURE_2D;
      gl_texture = 0;
      name = name_;
    }

    void render(unsigned target) {
      glTexParameteri(target, GL_TEXTURE_MAG_FILTER, texture_mag_filter);
      glTexParameteri(target, GL_TEXTURE_MIN_FILTER, texture_min_filter);
      glTexParameteri(target, GL_TEXTURE_WRAP_S, texture_wrap_s);
      glTexParameteri(target, GL_TEXTURE_WRAP_T, texture_wrap_t);
      glTexParameteri(target, GL_TEXTURE_WRAP_R, texture_wrap_r);
    }

    unsigned get_gl_target() const {
      return gl_target;
    }

    unsigned get_gl_texture(image *img) {
      if (!gl_texture) {
        gl_texture = img->get_gl_texture();

        glTexParameteri(gl_target, GL_TEXTURE_MAG_FILTER, texture_mag_filter);
        glTexParameteri(gl_target, GL_TEXTURE_MIN_FILTER, texture_min_filter);
        glTexParameteri(gl_target, GL_TEXTURE_WRAP_S, texture_wrap_s);
        glTexParameteri(gl_target, GL_TEXTURE_WRAP_T, texture_wrap_t);
        glTexParameteri(gl_target, GL_TEXTURE_WRAP_R, texture_wrap_r);
      }
      return gl_texture;
    }

    const char *get_glsl_type() {
      switch (gl_target) {
        case GL_TEXTURE_3D: return "sampler3D";
        case GL_TEXTURE_CUBE_MAP: return "samplerCube";
        default: return "sampler2D";
      }
    }

    const char *get_glsl_texture_fetch() {
      switch (gl_target) {
        case GL_TEXTURE_3D: return "texture3D";
        case GL_TEXTURE_CUBE_MAP: return "textureCube";
        default: return "texture2D";
      }
    }

    const char *get_name() {
      return name;
    }

    void visit(visitor &v) {
      v.visit(texture_mag_filter, atom_texture_mag_filter);
      v.visit(texture_min_filter, atom_texture_min_filter);
      v.visit(texture_wrap_s, atom_texture_wrap_s);
      v.visit(texture_wrap_t, atom_texture_wrap_t);
    }
  };
}}

