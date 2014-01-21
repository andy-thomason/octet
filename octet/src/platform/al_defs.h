////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// fake OpenAL for windows.
//

namespace octet {
  typedef char ALCboolean;
  typedef char ALCchar;
  typedef char ALCbyte;
  typedef unsigned char ALCubyte;
  typedef short ALCshort;
  typedef unsigned short ALCushort;
  typedef int ALCint;
  typedef unsigned int ALCuint;
  typedef int ALCsizei;
  typedef int ALCenum;
  typedef float ALCfloat;
  typedef double ALCdouble;
  typedef void ALCvoid;

  enum {
    ALC_FALSE = 0,
    ALC_TRUE = 1,
    ALC_FREQUENCY = 0x1007,
    ALC_REFRESH = 0x1008,
    ALC_SYNC = 0x1009,
    ALC_MONO_SOURCES = 0x1010,
    ALC_STEREO_SOURCES = 0x1011,
    ALC_NO_ERROR = ALC_FALSE,
    ALC_INVALID_DEVICE = 0xA001,
    ALC_INVALID_CONTEXT = 0xA002,
    ALC_INVALID_ENUM = 0xA003,
    ALC_INVALID_VALUE = 0xA004,
    ALC_OUT_OF_MEMORY = 0xA005,
    ALC_DEFAULT_DEVICE_SPECIFIER = 0x1004,
    ALC_DEVICE_SPECIFIER = 0x1005,
    ALC_EXTENSIONS = 0x1006,
    ALC_MAJOR_VERSION = 0x1000,
    ALC_MINOR_VERSION = 0x1001,
    ALC_ATTRIBUTES_SIZE = 0x1002,
    ALC_ALL_ATTRIBUTES = 0x1003,
    ALC_DEFAULT_ALL_DEVICES_SPECIFIER = 0x1012,
    ALC_ALL_DEVICES_SPECIFIER = 0x1013,
    ALC_CAPTURE_DEVICE_SPECIFIER = 0x310,
    ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER = 0x311,
    ALC_CAPTURE_SAMPLES = 0x312,
  };

  typedef char ALboolean;
  typedef char ALchar;
  typedef char ALbyte;
  typedef unsigned char ALubyte;
  typedef short ALshort;
  typedef unsigned short ALushort;
  typedef int ALint;
  typedef unsigned int ALuint;
  typedef int ALsizei;
  typedef int ALenum;
  typedef float ALfloat;
  typedef double ALdouble;
  typedef void ALvoid;

  enum {
    AL_INVALID = -1,
    AL_NONE = 0,
    AL_FALSE = 0,
    AL_TRUE = 1,
    AL_SOURCE_RELATIVE = 0x202,
    AL_CONE_INNER_ANGLE = 0x1001,
    AL_CONE_OUTER_ANGLE = 0x1002,
    AL_PITCH = 0x1003,
    AL_POSITION = 0x1004,
    AL_DIRECTION = 0x1005,
    AL_VELOCITY = 0x1006,
    AL_LOOPING = 0x1007,
    AL_BUFFER = 0x1009,
    AL_GAIN = 0x100A,
    AL_MIN_GAIN = 0x100D,
    AL_MAX_GAIN = 0x100E,
    AL_ORIENTATION = 0x100F,
    AL_CHANNEL_MASK = 0x3000,
    AL_SOURCE_STATE = 0x1010,
    AL_INITIAL = 0x1011,
    AL_PLAYING = 0x1012,
    AL_PAUSED = 0x1013,
    AL_STOPPED = 0x1014,
    AL_BUFFERS_QUEUED = 0x1015,
    AL_BUFFERS_PROCESSED = 0x1016,
    AL_SEC_OFFSET = 0x1024,
    AL_SAMPLE_OFFSET = 0x1025,
    AL_BYTE_OFFSET = 0x1026,
    AL_SOURCE_TYPE = 0x1027,
    AL_STATIC = 0x1028,
    AL_STREAMING = 0x1029,
    AL_UNDETERMINED = 0x1030,
    AL_FORMAT_MONO8 = 0x1100,
    AL_FORMAT_MONO16 = 0x1101,
    AL_FORMAT_STEREO8 = 0x1102,
    AL_FORMAT_STEREO16 = 0x1103,
    AL_REFERENCE_DISTANCE = 0x1020,
    AL_ROLLOFF_FACTOR = 0x1021,
    AL_CONE_OUTER_GAIN = 0x1022,
    AL_MAX_DISTANCE = 0x1023,
    AL_FREQUENCY = 0x2001,
    AL_BITS = 0x2002,
    AL_CHANNELS = 0x2003,
    AL_SIZE = 0x2004,
    AL_UNUSED = 0x2010,
    AL_PENDING = 0x2011,
    AL_PROCESSED = 0x2012,
    AL_NO_ERROR = AL_FALSE,
    AL_INVALID_NAME = 0xA001,
    AL_ILLEGAL_ENUM = 0xA002,
    AL_INVALID_ENUM = 0xA002,
    AL_INVALID_VALUE = 0xA003,
    AL_ILLEGAL_COMMAND = 0xA004,
    AL_INVALID_OPERATION = 0xA004,
    AL_OUT_OF_MEMORY = 0xA005,
    AL_VENDOR = 0xB001,
    AL_VERSION = 0xB002,
    AL_RENDERER = 0xB003,
    AL_EXTENSIONS = 0xB004,
    AL_DOPPLER_FACTOR = 0xC000,
    AL_DOPPLER_VELOCITY = 0xC001,
    AL_SPEED_OF_SOUND = 0xC003,
    AL_DISTANCE_MODEL = 0xD000,
    AL_INVERSE_DISTANCE = 0xD001,
    AL_INVERSE_DISTANCE_CLAMPED = 0xD002,
    AL_LINEAR_DISTANCE = 0xD003,
    AL_LINEAR_DISTANCE_CLAMPED = 0xD004,
    AL_EXPONENT_DISTANCE = 0xD005,
    AL_EXPONENT_DISTANCE_CLAMPED = 0xD006,
  };

  class ALbuffer {
    unsigned ref_cnt;

    dynarray<uint8_t> data;
    unsigned format;
    unsigned freq;
  public:

    ALbuffer() {
      ref_cnt = 0;
    }

    void add_ref() {
      ref_cnt++;
    }

    void release() {
      if (!--ref_cnt) {
        delete this;
      }
    }

    void set_data(const void *new_data, unsigned size, unsigned format, unsigned freq) {
      data.resize(size);
      memcpy(&data[0], new_data, size);
      this->format = format;
      this->freq = freq;
    }

    unsigned get_max_samples() {
      if (format == AL_FORMAT_MONO16) {
        return data.size() / 2;
      }
      return 0;
    }

    void render(float *mixer, unsigned start, unsigned num_samples, unsigned num_channels, float gain) {
      unsigned max_samples = get_max_samples();
      if (format == AL_FORMAT_MONO16) {
        int16_t *sp = (int16_t *)&data[0] + start;
        float scale = gain * (1.0f/16384);
        for (unsigned i = 0; i != num_samples; ++i) {
          float delta = *sp++ * scale;
          mixer[0] += delta;
          mixer[1] += delta;
          mixer += 2;
        }
      }
    }
  };

  class ALsource {
    unsigned ref_cnt;
    unsigned buffer;
    unsigned sample;
    unsigned state;
    unsigned looping;
    float gain;

    void *access(unsigned param) {
      if (param == AL_SOURCE_STATE) {
        return &state;
      } else if (param == AL_SAMPLE_OFFSET) {
        return &sample;
      } else if (param == AL_BUFFER) {
        return &buffer;
      } else if (param == AL_LOOPING) {
        return &looping;
      } else if (param == AL_GAIN) {
        return &gain;
      } else {
        return 0;
      }
    }

  public:

    ALsource() {
      ref_cnt = 0;
      state = AL_STOPPED;
      sample = 0;
      looping = 0;
      buffer = 0;
      gain = 1;
    }

    void add_ref() {
      ref_cnt++;
    }

    void release() {
      if (!--ref_cnt) {
        delete this;
      }
    }

    void seti(unsigned param, unsigned value) {
      unsigned *ptr = (unsigned*)access(param);
      *ptr = value;
    }

    void setf(unsigned param, float value) {
      float *ptr = (float*)access(param);
      *ptr = value;
    }

    unsigned geti(unsigned param) {
      unsigned *ptr = (unsigned*)access(param);
      return *ptr;
    }

    float getf(unsigned param) {
      float *ptr = (float*)access(param);
      return *ptr;
    }

    void render(ALbuffer *buffer, float *mixer, unsigned num_samples, unsigned num_channels) {
      if (state == AL_STOPPED || state == AL_PAUSED) return;

      while (num_samples) {
        unsigned max_samples = buffer->get_max_samples();
        if (sample >= max_samples) {
          if (looping && max_samples != 0) {
            sample = 0;
          } else {
            state = AL_STOPPED;
            break;
          }
        }
        unsigned segment = num_samples < max_samples ? num_samples : max_samples;
        buffer->render(mixer, sample, segment, num_channels, gain);
        sample += segment;
        num_samples -= segment;
      }
    }
  };

  struct ALCdevice {
  };

  class ALCcontext {
    enum { ring_size = 8 };
    dynarray<ref<ALbuffer> > buffers;
    dynarray<ref<ALsource> > sources;
    dynarray<float> mixer;
    dynarray<int16_t> mixer_bin;
    unsigned rate;
    unsigned num_channels;
    unsigned samples;
    FILE *log;

    void fill_buffer(int16_t *dest) {
      memset(&mixer[0], 0, mixer.size() * sizeof(mixer[0]));

      for (unsigned i = 0; i != sources.size(); ++i) {
        ALsource *src = sources[i];
        if (src) {
          // fill mixer
          ALbuffer *buffer = get_buffer(src->geti(AL_BUFFER));
          if (buffer) {
            //fprintf(log, "i=%d b=%p b=%d ST=%04x s=%d\n", i, buffer, src->geti(AL_BUFFER), src->geti(AL_SOURCE_STATE), src->geti(AL_SAMPLE_OFFSET));
            src->render(buffer, &mixer[0], mixer.size() / num_channels, num_channels);
          }
        }
      }

      const float *ps = &mixer[0];
      for (unsigned i = 0; i != mixer.size(); ++i) {
        int value = (int)(*ps++ * 32767 + 32768.5f);
        if (value < 0) value = 0; else if (value > 65535) value = 65535;
        *dest++ = (int16_t)(value) - 32768;
      }
    }

    #ifdef WIN32
      HWAVEOUT waveout;
      WAVEHDR headers[ring_size];
      unsigned ring_ptr;
    #endif
  public:
    ALCcontext() {
      //log = fopen("c:/tmp/al.txt", "wb");
      rate = 44100;
      num_channels = 2;
      samples = 0;

      buffers.resize(1);
      sources.resize(1);

      mixer.resize(1024);
      mixer_bin.resize(mixer.size() * ring_size);

      #ifdef WIN32
        WAVEFORMATEX fmt;
        fmt.nSamplesPerSec = 44100;
        fmt.wBitsPerSample = 16;
        fmt.nChannels = num_channels;
        fmt.cbSize = 0;
        fmt.wFormatTag = WAVE_FORMAT_PCM;
        fmt.nBlockAlign = fmt.wBitsPerSample/8 * num_channels;
        fmt.nAvgBytesPerSec = fmt.nBlockAlign * fmt.nSamplesPerSec;
        waveout = 0;

        if (waveOutOpen(&waveout, WAVE_MAPPER, &fmt, (DWORD_PTR)0, (DWORD_PTR)0, CALLBACK_NULL)) {
          printf("warning: failed to open windows wave out\n");
        }

        memset(&headers, 0, sizeof(headers));
        memset(&mixer_bin[0], 0, mixer_bin.size() * sizeof(mixer_bin[0]));
        for (unsigned i = 0; i != ring_size; ++i) {
          WAVEHDR *hdr = headers + i;
          hdr->lpData = (LPSTR)&mixer_bin[mixer.size() * i];
          hdr->dwBufferLength = mixer.size() * sizeof(int16_t);
          waveOutPrepareHeader(waveout, hdr, sizeof(*hdr));
        }
      #endif
    }

    ~ALCcontext() {
      #ifdef WIN32
        for (unsigned i = 0; i != ring_size; ++i) {
          WAVEHDR *hdr = headers + i;
          waveOutUnprepareHeader(waveout, hdr, sizeof(*hdr));
        }
        waveOutClose(waveout);
        waveout = 0;
      #endif
    }

    unsigned gen_buffer() {
      unsigned res = buffers.size();
      buffers.resize(res + 1);
      return res;
    }

    unsigned gen_source() {
      unsigned res = sources.size();
      sources.resize(res + 1);
      return res;
    }

    ALbuffer *get_buffer(unsigned i ) {
      if (i >= buffers.size()) return 0;
      if (!buffers[i]) {
        buffers[i] = new ALbuffer();
      }
      return buffers[i];
    }

    ALsource *get_source(unsigned i ) {
      if (i >= sources.size()) return 0;
      if (!sources[i]) {
        sources[i] = new ALsource();
      }
      return sources[i];
    }

    // render the sound
    void update() {
      #ifdef WIN32
        //fprintf(log, "update\n"); 
        for(;;) {
          unsigned i = ring_ptr++ % ring_size;
          WAVEHDR *hdr = headers + i;
          //fprintf(log, "%08x\n", hdr->dwFlags);
          if (hdr->dwFlags & WHDR_INQUEUE) {
            break;
          }

          fill_buffer((int16_t*)hdr->lpData);
          unsigned z = 0;
          for (unsigned i = 0; i != hdr->dwBufferLength; ++i) {
            if (hdr->lpData[i]) z++;
          }

          MMRESULT res = waveOutWrite(waveout, hdr, sizeof(*hdr));
          //fprintf(log, "  wr %d %d\n", res, z);
        }
      #endif
    }
  };

  static inline ALCcontext *Fake_AL_context(ALCcontext *value=0) {
    static ALCcontext *cur;
    if (value) cur = value;
    return cur;
  }

  inline void alEnable( ALenum capability ) {
  }


  inline void alDisable( ALenum capability ) {
  }
 

  inline ALboolean alIsEnabled( ALenum capability ) {
    return 0;
  }
 

  inline const ALchar* alGetString( ALenum param ) {
    return 0;
  }


  inline void alGetBooleanv( ALenum param, ALboolean* data ) {
  }


  inline void alGetIntegerv( ALenum param, ALint* data ) {
  }


  inline void alGetFloatv( ALenum param, ALfloat* data ) {
  }


  inline void alGetDoublev( ALenum param, ALdouble* data ) {
  }


  inline ALboolean alGetBoolean( ALenum param ) {
    return 0;
  }


  inline ALint alGetInteger( ALenum param ) {
    return 0;
  }


  inline ALfloat alGetFloat( ALenum param ) {
    return 0;
  }


  inline ALdouble alGetDouble( ALenum param ) {
    return 0;
  }


  inline ALenum alGetError( void ) {
    return 0;
  }


  inline ALboolean alIsExtensionPresent( const ALchar* extname ) {
    return 0;
  }


  inline void* alGetProcAddress( const ALchar* fname ) {
    return 0;
  }


  inline ALenum alGetEnumValue( const ALchar* ename ) {
    return 0;
  }


  inline void alListenerf( ALenum param, ALfloat value ) {
  }


  inline void alListener3f( ALenum param, ALfloat value1, ALfloat value2, ALfloat value3 ) {
  }


  inline void alListenerfv( ALenum param, const ALfloat* values ) {
  }
 

  inline void alListeneri( ALenum param, ALint value ) {
  }


  inline void alListener3i( ALenum param, ALint value1, ALint value2, ALint value3 ) {
  }


  inline void alListeneriv( ALenum param, const ALint* values ) {
  }


  inline void alGetListenerf( ALenum param, ALfloat* value ) {
  }


  inline void alGetListener3f( ALenum param, ALfloat *value1, ALfloat *value2, ALfloat *value3 ) {
  }


  inline void alGetListenerfv( ALenum param, ALfloat* values ) {
  }


  inline void alGetListeneri( ALenum param, ALint* value ) {
  }


  inline void alGetListener3i( ALenum param, ALint *value1, ALint *value2, ALint *value3 ) {
  }


  inline void alGetListeneriv( ALenum param, ALint* values ) {
  }


  inline void alGenSources( ALsizei n, ALuint* sources ) {
    ALCcontext *ctxt = Fake_AL_context();
    for (ALsizei i = 0; i != n; ++i) {
      sources[i] = ctxt->gen_source();
    }
  }
 

  inline void alDeleteSources( ALsizei n, const ALuint* sources ) {
  }


  inline ALboolean alIsSource( ALuint sid ) {
    return 0;
  }
 

  inline void alSourcef( ALuint sid, ALenum param, ALfloat value ) {
    ALCcontext *ctxt = Fake_AL_context();
    ALsource *src = ctxt->get_source(sid);
    src->setf(param, value);
  }
 

  inline void alSource3f( ALuint sid, ALenum param, ALfloat value1, ALfloat value2, ALfloat value3 ) {
  }


  inline void alSourcefv( ALuint sid, ALenum param, const ALfloat* values ) {
  }
 

  inline void alSourcei( ALuint sid, ALenum param, ALint value ) {
    ALCcontext *ctxt = Fake_AL_context();
    ALsource *src = ctxt->get_source(sid);
    src->seti(param, value);
  }
 

  inline void alSource3i( ALuint sid, ALenum param, ALint value1, ALint value2, ALint value3 ) {
  }


  inline void alSourceiv( ALuint sid, ALenum param, const ALint* values ) {
  }


  inline void alGetSourcef( ALuint sid, ALenum param, ALfloat* value ) {
  }


  inline void alGetSource3f( ALuint sid, ALenum param, ALfloat* value1, ALfloat* value2, ALfloat* value3) {
  }


  inline void alGetSourcefv( ALuint sid, ALenum param, ALfloat* values ) {
  }


  inline void alGetSourcei( ALuint sid,  ALenum param, ALint* value ) {
  }


  inline void alGetSource3i( ALuint sid, ALenum param, ALint* value1, ALint* value2, ALint* value3) {
  }


  inline void alGetSourceiv( ALuint sid,  ALenum param, ALint* values ) {
  }


  inline void alSourcePlayv( ALsizei ns, const ALuint *sids ) {
  }


  inline void alSourceStopv( ALsizei ns, const ALuint *sids ) {
  }


  inline void alSourceRewindv( ALsizei ns, const ALuint *sids ) {
  }


  inline void alSourcePausev( ALsizei ns, const ALuint *sids ) {
  }

  inline void alSourcePlay( ALuint sid ) {
    ALCcontext *ctxt = Fake_AL_context();
    ALsource *src = ctxt->get_source(sid);
    if (src) {
      src->seti(AL_SAMPLE_OFFSET, 0);
      src->seti(AL_SOURCE_STATE, AL_PLAYING);
    }
  }

  inline void alSourceStop( ALuint sid ) {
    ALCcontext *ctxt = Fake_AL_context();
    ALsource *src = ctxt->get_source(sid);
    if (src) {
      src->seti(AL_SOURCE_STATE, AL_STOPPED);
    }
  }


  inline void alSourceRewind( ALuint sid ) {
    ALCcontext *ctxt = Fake_AL_context();
    ALsource *src = ctxt->get_source(sid);
    if (src) {
      src->seti(AL_SAMPLE_OFFSET, 0);
    }
  }


  inline void alSourcePause( ALuint sid ) {
    ALCcontext *ctxt = Fake_AL_context();
    ALsource *src = ctxt->get_source(sid);
    if (src) {
      src->seti(AL_SOURCE_STATE, AL_PAUSED);
    }
  }


  inline void alSourceQueueBuffers( ALuint sid, ALsizei numEntries, const ALuint *bids ) {
  }


  inline void alSourceUnqueueBuffers( ALuint sid, ALsizei numEntries, ALuint *bids ) {
  }


  inline void alGenBuffers( ALsizei n, ALuint* buffers ) {
    ALCcontext *ctxt = Fake_AL_context();
    for (ALsizei i = 0; i != n; ++i) {
      buffers[i] = ctxt->gen_buffer();
    }
  }


  inline void alDeleteBuffers( ALsizei n, const ALuint* buffers ) {
  }


  inline ALboolean alIsBuffer( ALuint bid ) {
    return AL_TRUE;
  }


  inline void alBufferData( ALuint bid, ALenum format, const ALvoid* data, ALsizei size, ALsizei freq ) {
    ALCcontext *ctxt = Fake_AL_context();
    ALbuffer *buf = ctxt->get_buffer(bid);
    if (buf) {
      buf->set_data(data, size, format, freq);
    }
  }


  inline void alBufferf( ALuint bid, ALenum param, ALfloat value ) {
  }


  inline void alBuffer3f( ALuint bid, ALenum param, ALfloat value1, ALfloat value2, ALfloat value3 ) {
  }


  inline void alBufferfv( ALuint bid, ALenum param, const ALfloat* values ) {
  }


  inline void alBufferi( ALuint bid, ALenum param, ALint value ) {
  }


  inline void alBuffer3i( ALuint bid, ALenum param, ALint value1, ALint value2, ALint value3 ) {
  }


  inline void alBufferiv( ALuint bid, ALenum param, const ALint* values ) {
  }


  inline void alGetBufferf( ALuint bid, ALenum param, ALfloat* value ) {
  }


  inline void alGetBuffer3f( ALuint bid, ALenum param, ALfloat* value1, ALfloat* value2, ALfloat* value3) {
  }


  inline void alGetBufferfv( ALuint bid, ALenum param, ALfloat* values ) {
  }


  inline void alGetBufferi( ALuint bid, ALenum param, ALint* value ) {
  }


  inline void alGetBuffer3i( ALuint bid, ALenum param, ALint* value1, ALint* value2, ALint* value3) {
  }


  inline void alGetBufferiv( ALuint bid, ALenum param, ALint* values ) {
  }


  inline void alDopplerFactor( ALfloat value ) {
  }


  inline void alDopplerVelocity( ALfloat value ) {
  }


  inline void alSpeedOfSound( ALfloat value ) {
  }


  inline void alDistanceModel( ALenum distanceModel ) {
  }


  inline ALCcontext * alcCreateContext( ALCdevice *device, const ALCint* attrlist ) {
    return new ALCcontext();
  }


  inline ALCboolean alcMakeContextCurrent( ALCcontext *context ) {
    Fake_AL_context( context );
    return 0;
  }

  inline void alcProcessContext( ALCcontext *context ) {
  }


  inline void alcSuspendContext( ALCcontext *context ) {
  }


  inline void alcDestroyContext( ALCcontext *context ) {
    delete context;
  }


  inline ALCcontext * alcGetCurrentContext( void ) {
    return Fake_AL_context();
  }


  inline ALCdevice* alcGetContextsDevice( ALCcontext *context ) {
    return 0;
  }

  inline ALCdevice * alcOpenDevice( const ALCchar *devicename ) {
    return new ALCdevice();
  }


  inline ALCboolean alcCloseDevice( ALCdevice *device ) {
    delete device;
    return 0;
  }

  inline ALCenum alcGetError( ALCdevice *device ) {
    return 0;
  }

  inline ALCboolean alcIsExtensionPresent( ALCdevice *device, const ALCchar *extname ) {
    return 0;
  }


  inline void  * alcGetProcAddress( ALCdevice *device, const ALCchar *funcname ) {
    return 0;
  }

  inline ALCenum alcGetEnumValue( ALCdevice *device, const ALCchar *enumname ) {
    return 0;
  }

  inline const ALCchar * alcGetString( ALCdevice *device, ALCenum param ) {
    return 0;
  }


  inline void alcGetIntegerv( ALCdevice *device, ALCenum param, ALCsizei size, ALCint *data ) {
  }

  inline ALCdevice* alcCaptureOpenDevice( const ALCchar *devicename, ALCuint frequency, ALCenum format, ALCsizei buffersize ) {
    return 0;
  }


  inline ALCboolean alcCaptureCloseDevice( ALCdevice *device ) {
    return 0;
  }


  inline void alcCaptureStart( ALCdevice *device ) {
  }


  inline void alcCaptureStop( ALCdevice *device ) {
  }


  inline void alcCaptureSamples( ALCdevice *device, ALCvoid *buffer, ALCsizei samples ) {
  }
}
