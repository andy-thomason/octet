////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
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
    const unsigned char *src = &buffer[0];
    const unsigned char *src_max = src + buffer.size();
    if (buffer.size() >= 6 && !memcmp(&buffer[0], "GIF89a", 6)) {
      gif_decoder dec;
      return dec.get_texture(src, src_max);
    } else if (buffer.size() >= 6 && buffer[0] == 0 && buffer[1] == 0 && buffer[2] == 2) {
      tga_decoder dec;
      return dec.get_texture(src, src_max);
    } else {
      printf("warning: unknown texture format\n");
    }
  }
  return 0;
}
