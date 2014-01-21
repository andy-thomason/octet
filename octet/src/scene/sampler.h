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
  public:
    RESOURCE_META(sampler)

    // default constructor makes a repeating trilinear filter
    sampler() {
      texture_mag_filter = GL_LINEAR_MIPMAP_LINEAR;
      texture_min_filter = GL_LINEAR;
      texture_wrap_s = GL_REPEAT;
      texture_wrap_t = GL_REPEAT;
    }

    void render(unsigned target) {
      /*glTexParameteri(target, GL_TEXTURE_MAG_FILTER, texture_mag_filter);
      glTexParameteri(target, GL_TEXTURE_MIN_FILTER, texture_min_filter);
      glTexParameteri(target, GL_TEXTURE_WRAP_S, texture_wrap_s);
      glTexParameteri(target, GL_TEXTURE_WRAP_T, texture_wrap_t);*/
    }

    void visit(visitor &v) {
      v.visit(texture_mag_filter, atom_texture_mag_filter);
      v.visit(texture_min_filter, atom_texture_min_filter);
      v.visit(texture_wrap_s, atom_texture_wrap_s);
      v.visit(texture_wrap_t, atom_texture_wrap_t);
    }
  };
}}

