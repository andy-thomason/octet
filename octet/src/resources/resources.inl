////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// a container for named resources
//

// todo: kill this
GLuint octet::resources::resource_dict::get_texture_handle_internal(unsigned gl_kind, const char *url) {
  if (url[0] == '!') {
    return app_utils::get_stock_texture(gl_kind, url+1);
  } else if (url[0] == '#') {
    return app_utils::get_solid_texture(gl_kind, url+1);
  } else {
    dynarray<uint8_t> buffer;
    dynarray<uint8_t> image;
    app_utils::get_url(buffer, url);
    uint16_t format = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    const unsigned char *src = &buffer[0];
    const unsigned char *src_max = src + buffer.size();
    if (buffer.size() >= 6 && !memcmp(&buffer[0], "GIF89a", 6)) {
      gif_decoder dec;
      dec.get_image(image, format, width, height, src, src_max);
    } else if (buffer.size() >= 6 && buffer[0] == 0xff && buffer[1] == 0xd8) {
      jpeg_decoder dec;
      dec.get_image(image, format, width, height, src, src_max);
    } else if (buffer.size() >= 6 && buffer[0] == 0 && buffer[1] == 0 && buffer[2] == 2) {
      tga_decoder dec;
      dec.get_image(image, format, width, height, src, src_max);
    } else {
      printf("warning: unknown texture format\n");
      return 0;
    }

    if (width > 0 && height > 0 && format) {
      return app_utils::make_texture(format, &image[0], image.size(), format, width, height);
    } else
    {
      return 0;
    }
  }
}

inline octet::resources::resource *octet::resources::resource::new_type(atom_t type) {
  switch ((int)type) {
    #define OCTET_CLASS(N, X) case atom_##X: return new X();
    //#pragma message("resources.inl")
    #include "../resources/classes.h"
    #undef OCTET_CLASS
  }
  return NULL;
}

