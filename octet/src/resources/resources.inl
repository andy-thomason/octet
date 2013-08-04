////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// a container for named resources
//

namespace octet {
  // todo: kill this
  GLuint resources::get_texture_handle_internal(unsigned gl_kind, const char *url) {
    if (url[0] == '!') {
      return app_utils::get_stock_texture(gl_kind, url+1);
    } else if (url[0] == '#') {
      return app_utils::get_solid_texture(gl_kind, url+1);
    } else {
      dynarray<uint8_t> buffer;
      dynarray<uint8_t> full_image;
      app_utils::get_url(buffer, url);
      uint32_t num_components = 3;
      uint32_t width = 0;
      uint32_t height = 0;
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
      } else {
        printf("warning: unknown texture format\n");
        return 0;
      }

      if (width > 0 && height > 0 && num_components) {
        return app_utils::make_texture(num_components == 4 ? GL_RGBA : GL_RGB, full_image, width, height);
      } else
      {
        return 0;
      }
    }
  };
}
