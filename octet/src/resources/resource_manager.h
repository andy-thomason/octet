////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
//

class resource_manager {
  typedef dictionary<GLuint> textures_t;
  typedef dictionary<int> sounds_t;

  static textures_t &textures() { static textures_t instance;  return instance; }
  static sounds_t &sounds() { static sounds_t instance;  return instance; }

  static GLuint get_texture_handle_internal(unsigned gl_kind, const char *name) {
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

  static unsigned u4(unsigned char *src) {
    return src[0] + src[1] * 256 + src[2] * 65536 + src[3] * 0x1000000;
  }

  static ALuint get_sound_handle_internal(unsigned al_kind, const char *name) {
    if (name[0] == '#') {
      // todo: implement notes etc.
      return 0;
    } else {
      dynarray<unsigned char> buffer;
      app_utils::get_url(buffer, name);
      if (buffer.size() >= 6 && !memcmp(&buffer[0], "RIFF", 4)) {
        unsigned offset = 0;
        unsigned samples = 44100;
        unsigned char *src = &buffer[0];
        for (unsigned i = 12; i+8 <= buffer.size(); i += 8 + u4(src+i+4)) {
          if (src[i] == 'f' && src[i+1] == 'm' && src[i+2] == 't' && src[i+3] == ' ') {
            samples = u4(src+i+12);
          } else if (src[i] == 'd' && src[i+1] == 'a' && src[i+2] == 't' && src[i+3] == 'a') {
            offset = i + 8;
            break;
          }
        }
        return app_utils::make_sound_buffer(al_kind, 44100, buffer, offset, buffer.size() - offset);
      } else {
        printf("warning: unknown audio format\n");
      }
    }
    return 0;
  }
public:

  // factory for textures
  static GLuint get_texture_handle(unsigned gl_kind, const char *name) {
    GLuint &result = textures()[name];
    if (result == 0) {
      result = get_texture_handle_internal(gl_kind, name);
    }
    return result;
  }

  // factory for sounds
  static int get_sound_handle(unsigned al_kind, const char *name) {
    int &result = sounds()[name];
    if (result == 0) {
      result = get_sound_handle_internal(al_kind, name);
    }
    return result;
  }

};

/*
    ALCdevice *dev = alcOpenDevice(NULL);
    assert(dev);
    ALCcontext *ctx = alcCreateContext(dev, NULL);
    alcMakeContextCurrent(ctx);
    assert(ctx);
    
    alGenSources(num_sources, sources);
    alGenBuffers(256, buffers);
    
    size_t magic_offset = 44;
    FILE *file = fopen("assets/abc.wav", "rb");
    assert(file && "assets/abc.wav not found");
    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *tmp = (char*)malloc(length);
    fread(tmp, 1, length, file);
    fclose(file);
    //for( int i = 0; i != 44; ++i) printf("%02x ", tmp[i]&0xff);
    
    // use the labels file to build the buffers for the sound
    FILE *labels = fopen("assets/abc_labels.txt","r");
    assert(labels && "assets/abc_labels.txt not found");
    char line[64];
    while (fgets(line, sizeof(line), labels)) {
      float start, end;
      char name[64];
      sscanf(line, "%f %f %s", &start, &end, name);
      size_t start_offset = magic_offset + 2* (int)(44100*start);
      size_t end_offset = magic_offset + 2 * (int)(44100*end);

      //printf("%d %d %d %s [%d]\n", buffer_idx, start_offset, end_offset, name, length);
      assert(start_offset < length && end_offset > start_offset && end_offset < length);

      alBufferData(
        buffers[name[0]],
        AL_FORMAT_MONO16,
        tmp + start_offset,
        end_offset - start_offset,
        44100
      );
    }
    fclose(labels);
    free(tmp);
    
    alListener3f(AL_POSITION, 0, 0, 0);
    cur_source = 0;
    
    for (int i = 0; i != num_icons; ++i) {
      icon_textures[i] = -1;
    }
    create_icon(0);

*/