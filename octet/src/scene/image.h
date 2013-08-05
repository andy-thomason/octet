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
  class image : public resource {
    // primary attributes (to save)

    // source of image
    string url;

    // how to store it in GL
    uint32_t num_components;

    // derived attributes (not for saving)
    GLuint gl_texture;

    // dimensions
    uint32_t width;
    uint32_t height;

    // full image in RGBA
    dynarray<uint8_t> full_image;

    // compressed texture
    dynarray<uint8_t> compressed_image;

    void init(const char *name) {
      this->url = name;
      num_components = 0;
      width = height = 0;
      gl_texture = 0;
    }

  public:
    RESOURCE_META(image)

    // default constructor makes a blank material.
    image() {
      init("");
    }

    image(const char *name) {
      init(name);
    }

    // best to use a param for this
    /*image(const vec4 &color) {
      char name[16];
      sprintf(name, "#%02x%02x%02x%02x", (int)(color[0]*255.0f+0.5f), (int)(color[1]*255.0f+0.5f), (int)(color[2]*255.0f+0.5f), (int)(color[3]*255.0f+0.5f));
      init(name);
    }*/

    // access attributes by name
    void visit(visitor &v) {
      v.visit(url, atom_url);
      //v.visit(gl_format, atom_gl_format);
    }

    // load the image from a file
    void load() {
      dynarray<uint8_t> buffer;
      num_components = 0;
      app_utils::get_url(buffer, url);
      const unsigned char *src = &buffer[0];
      const unsigned char *src_max = src + buffer.size();
      if (buffer.size() >= 6 && !memcmp(&buffer[0], "GIF89a", 6)) {
        gif_decoder dec;
        dec.get_image(full_image, num_components, width, height, src, src_max);
      } else if (buffer.size() >= 6 && buffer[0] == 0xff && buffer[1] == 0xd8) {
        jpeg_decoder dec;
        dec.get_image(full_image, num_components, width, height, src, src_max);
      } else if (buffer.size() >= 6 && buffer[0] == 0 && buffer[1] == 0 && buffer[2] == 2) {
        tga_decoder dec;
        dec.get_image(full_image, num_components, width, height, src, src_max);
      } else if (buffer.size() >= 4 && buffer[0] == 'D' && buffer[1] == 'D' && buffer[2] == 'S' && buffer[3] == ' ') {
        dds_decoder dec;
        dec.get_image(full_image, num_components, width, height, src, src_max);
      } else {
        printf("warning: unknown texture format\n");
        return;
      }
    }

    GLuint get_gl_texture() {
      if (!gl_texture) {
        if (full_image.size() == 0) {
          load();
        }
        assert(full_image.size() == width * height * 4);

        // make a new texture handle
        glGenTextures(1, &gl_texture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gl_texture);

        // todo: handle compressed textures
        glTexImage2D(GL_TEXTURE_2D, 0, num_components == 3 ? GL_RGB : GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)full_image.data());

        // this may not work on very old systems, comment it out.
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      }
      return gl_texture;
    }
  };
}

