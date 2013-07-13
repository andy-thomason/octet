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
  enum chan_kind {
    chan_float,
    chan_matrix,
  };
private:

  // todo: this could be a GL/CL buffer
  dynarray<unsigned char> data;

  /// one channel of an animation
  struct channel {
    atom_t target;   /// atom for target
    chan_kind kind;  /// float or transform
    int offset;      /// where in data
    unsigned num_times;   /// how many time values
  };

  dynarray<channel> channels;

  float end_time;
public:
  RESOURCE_META(animation)
  
  animation() {
    end_time = 0;
  }

  int get_num_channels() const {
    return (int)channels.size();
  }

  chan_kind get_chan_kind(int ch) const {
    return channels[ch].kind;
  }

  atom_t get_target(int ch) const {
    return channels[ch].target;
  }

  float get_end_time() const {
    return end_time;
  }

  // crude matrix based channel
  void add_channel_from_matrices(int num_times, float *times, float *matrices) {
    channel ch;
    ch.kind = chan_matrix;
    ch.num_times = num_times;
    int offset = ch.offset = (int)data.size();
    int bytes = num_times * sizeof(unsigned short) + num_times * sizeof(mat4t);
    data.resize(ch.offset + bytes);
    end_time = times[num_times-1] > end_time ? times[num_times-1] : end_time;
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
  void eval_chan(int chan, float time, void *dest, size_t max_size) const {
    int time_ms = int(time * 0.001f);
    const channel &ch = channels[chan];
    unsigned short *p = (unsigned short *)data[ch.offset];
    unsigned a = 0;
    unsigned b = ch.num_times - 1;

    if (time_ms < p[0]) {
      time_ms = p[0];
    } else if (time_ms >= p[b]) {
      time_ms = p[b];
      a = b - 1;
    } else {
      while (b - a > 1) {
        unsigned mid = a + ((b - a) >> 1);
        if (time_ms > p[mid]) {
          a = mid;
        } else {
          b = mid;
        }
      }
    }

    unsigned data_offset = ch.offset + ch.num_times * sizeof(unsigned short);

    switch( ch.kind ) {
      case chan_matrix: {
        float t = float(time_ms - p[a]) / (p[b] - p[a]);
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

