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
  /// Color, Image or function describing color at a point in space or on a surface.
  ///
  /// A param is an abstract thing that can provide color or other attributes
  /// it could be a texture (with a uv set) or a vertex attribute or a solid color.
  /// we could also extend it to provide functions in shaders.
  class param : public resource {
    atom_t kind;
    ref<image> img;
    ref<sampler> smpl;
    ref<param> texcoords;

    vec4 color;

    // derived
    GLuint gl_texture;
    GLuint gl_target;

  public:
    RESOURCE_META(param)

    /// default constructor makes a blank material.
    param() {
      gl_texture = 0;
      gl_target = GL_TEXTURE_2D;
      kind = atom_color;
      color = vec4(0.5f, 0.5f, 0.5f, 1);
    }

    /// Make a solid color parameter.
    param(const vec4 &color) {
      gl_texture = 0;
      gl_target = GL_TEXTURE_2D;
      this->color = color;
      kind = atom_color;
    }

    /// Make an image parameter
    param(image *img, sampler *smpl = 0) {
      gl_texture = 0;
      gl_target = GL_TEXTURE_2D;
      this->img = img;
      this->smpl = smpl;
      kind = atom_image;
    }

    /// Serialize or access attributes by name
    void visit(visitor &v) {
      v.visit(kind, atom_kind);
      v.visit(img, atom_image);
      v.visit(smpl, atom_sampler);
    }

    /// generate a texture for this parameter
    GLuint get_gl_texture() {
      if (!gl_texture) {
        if (kind == atom_image) {
          gl_texture = img->get_gl_texture();
          gl_target = img->get_gl_target();
        } else {
          char name[16];
          sprintf(name, "#%02x%02x%02x%02x", (int)(color[0]*255.0f+0.5f), (int)(color[1]*255.0f+0.5f), (int)(color[2]*255.0f+0.5f), (int)(color[3]*255.0f+0.5f));
          gl_texture = resource_dict::get_texture_handle(GL_RGBA, name);
        }
      }
      return gl_texture;
    }

    /// Get the color for this parameter.
    vec4 get_color() const {
      return color;
    }

    /// Set the color for this parameter.
    void set_color(const vec4 &value) {
      color = value;
    }

    /// Get the image for this parameter.
    image *get_image() const {
      return img;
    }

    /// Get the sampler for this parameter.
    sampler *get_sampler() const {
      return smpl;
    }

    /// Render this parameter into a texture slot.
    void render(unsigned slot) {
      glActiveTexture(GL_TEXTURE0 + slot);
      glBindTexture(gl_target, get_gl_texture());
      if (smpl) {
        //smpl->render(gl_target);
      }
    }
  };

}}

