////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Material 
//
//
// Materials are represented as GL textures with solid colours as single pixel textures.
// This simplifies shader design.
//

namespace octet {
  // a param is an abstract thing that can provide color or other attributes
  // it could be a texture (with a uv set) or a vertex attribute or a solid color.
  // we could also extend it to provide functions in shaders.
  class param : public resource {
    atom_t kind;
    ref<image> img;

    vec4 color;

    // derived
    GLuint gl_texture;

  public:
    RESOURCE_META(param)

    // default constructor makes a blank material.
    param() {
      gl_texture = 0;
      kind = atom_color;
      color = vec4(0.5f, 0.5f, 0.5f, 1);
    }

    param(const vec4 &color) {
      gl_texture = 0;
      this->color = color;
      kind = atom_color;
    }

    param(image *img) {
      gl_texture = 0;
      this->img = img;
      kind = atom_image;
    }

    // access attributes by name
    void visit(visitor &v) {
      v.visit(kind, atom_kind);
      v.visit(img, atom_image);
    }

    // generate a texture for this parameter
    GLuint get_gl_texture() {
      if (!gl_texture) {
        if (kind == atom_image) {
          gl_texture = img->get_gl_texture();
        } else {
          char name[16];
          sprintf(name, "#%02x%02x%02x%02x", (int)(color[0]*255.0f+0.5f), (int)(color[1]*255.0f+0.5f), (int)(color[2]*255.0f+0.5f), (int)(color[3]*255.0f+0.5f));
          gl_texture = resources::get_texture_handle(GL_RGBA, name);
        }
      }
      return gl_texture;
    }

    // what is the color of this parameter
    vec4 get_color() {
      return color;
    }

    void set_color(const vec4 &value) {
      color = value;
    }

    image *get_image() {
      return img;
    }
  };
}

