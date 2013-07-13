////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// a container for named resources
//

GLuint resources::get_texture_handle_internal(unsigned gl_kind, const char *name) {
  if (name[0] == '!') {
    return app_utils::get_stock_texture(gl_kind, name+1);
  } else if (name[0] == '#') {
    return app_utils::get_solid_texture(gl_kind, name+1);
  } else {
    dynarray<unsigned char> buffer;
    app_utils::get_url(buffer, name);
    if (buffer.size() >= 6 && !memcmp(&buffer[0], "GIF89a", 6)) {
      gif_decoder gif;
      const unsigned char *src = &buffer[0];
      const unsigned char *src_max = src + buffer.size();
      return gif.get_texture(gl_kind, src, src_max);
    } else {
      printf("warning: unknown texture format\n");
    }
  }
  return 0;
}
