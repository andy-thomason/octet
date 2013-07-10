////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Animation resource
//

class animation : public resource {
public:
  struct animation_instance {
    int animation;
    int time;
    int rate;
    bool is_looping;
    bool is_paused;
  };

private:
  enum chan_kind {
    chan_float,
    chan_matrix,
  };

  // todo: this could be a GL/CL buffer
  dynarray<unsigned char> data;

  /// one channel of an animation
  struct channel {
    int target;   /// atom for target
    chan_kind kind;  /// float or transform
    int offset;      /// where in data
    unsigned num_times;   /// how many time values
  };

  dynarray<channel> channels;
public:
  animation() {
  }

  int get_num_channels() {
    return (int)channels.size();
  }

  // crude matrix based channel
  void add_channel_from_matrices(int num_times, float *times, float *matrices) {
    channel ch;
    ch.kind = chan_matrix;
    ch.num_times = num_times;
    int offset = ch.offset = (int)data.size();
    int bytes = num_times * sizeof(unsigned short) + num_times * sizeof(mat4t);
    data.resize(ch.offset + bytes);;
    for (int i = 0; i != num_times; ++i) {
      unsigned short it = (unsigned short)( times[i] * 1000 );
      *((unsigned short*)&data[offset]) = it;
      offset += sizeof(unsigned short);
    }
    for (int i = 0; i != num_times; ++i) {
      mat4t m;
      m.init_row_major(matrices + i*16);
      *((mat4t*)&data[offset]) = m;
      offset += sizeof(mat4t);
    }
  }

  // evaluate one channel at one time - very inefficient.
  // time is in ms.
  void eval_chan(int chan, unsigned short time, void *dest, size_t max_size) {
    channel &ch = channels[chan];
    unsigned short *p = (unsigned short *)data[ch.offset];
    unsigned a = 0;
    unsigned b = ch.num_times - 1;

    assert(time <= p[b]);

    while (b - a > 1) {
      unsigned mid = a + ((b - a) >> 1);
      if (time > p[mid]) {
        a = mid;
      } else {
        b = mid;
      }
    }

    unsigned data_offset = ch.offset + ch.num_times * sizeof(unsigned short);

    switch( ch.kind ) {
      case chan_matrix: {
        float t = float(time - p[a]) / (p[b] - p[a]);
        assert(max_size >= sizeof(mat4t));
        mat4t ma, mb;
        memcpy(&ma, &data[data_offset + a * sizeof(mat4t)], sizeof(mat4t));
        memcpy(&mb, &data[data_offset + b * sizeof(mat4t)], sizeof(mat4t));
        mat4t m = ma * t + mb * (1-t);
        memcpy(dest, &m, sizeof(mat4t));
      } break;
    }
  }
};

