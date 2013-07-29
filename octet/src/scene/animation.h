////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Animation resource
//

namespace octet {
  class animation : public resource {
    // todo: this could be a GL/CL buffer
    dynarray<unsigned char> data;

    /// one channel of an animation
    struct channel {
      atom_t sid;          /// atom for sid on target (eg. node22)
      atom_t sub_target;   /// sub target (eg. rotateX)
      atom_t component;    /// component (eg. ANGLE)
      int offset;          /// where in data
      unsigned num_times;  /// how many time values
      unsigned component_size; /// number of bytes per component
    };

    // format and component of channels
    dynarray<channel> channels;

    // if we are targeting a specific node or component
    // note that we keep this separate because arrays of refs have special significance
    // and we want to keep channel as a "POD" type (plain old data).
    dynarray<ref<animation_target>> targets;

    float end_time;
  public:
    RESOURCE_META(animation)
  
    animation() {
      end_time = 0;
    }

    int get_num_channels() const {
      return (int)channels.size();
    }

    atom_t get_sid(int ch) const {
      return channels[ch].sid;
    }

    atom_t get_sub_target(int ch) const {
      return channels[ch].sub_target;
    }

    atom_t get_component(int ch) const {
      return channels[ch].component;
    }

    animation_target *get_target(int ch) const {
      return targets[ch];
    }

    float get_end_time() const {
      return end_time;
    }

    // just store the floats in the channel for now.
    // todo: optimise animation data
    void add_channel(animation_target *target, atom_t sid, atom_t sub_target, atom_t component, dynarray<float> &times, dynarray<float> &values) {
      int num_times = (int)times.size();
      int num_values = (int)values.size();
      int component_size = (num_values / num_times) * sizeof(float);

      channel ch;
      ch.num_times = num_times;
      ch.sid = sid;
      ch.sub_target = sub_target;
      ch.component = component;
      ch.component_size = component_size;

      int offset = ch.offset = (int)data.size();
      int bytes = num_times * sizeof(unsigned short) + component_size * num_times;
      data.resize(ch.offset + bytes);
      end_time = times[num_times-1] > end_time ? times[num_times-1] : end_time;
      for (int i = 0; i != num_times; ++i) {
        unsigned short it = (unsigned short)( times[i] * 1000 );
        *((unsigned short*)&data[offset]) = it;
        offset += sizeof(unsigned short);
      }
      
      memcpy(&data[offset], &values[0], component_size * num_times);
      channels.push_back(ch);
      targets.push_back(target);
    }

    // evaluate one channel at one time - very inefficient.
    // time is in ms.
    void eval_chan(int chan, float time, animation_target *target) const {
      int time_ms = int(time * 1000);
      const channel &ch = channels[chan];
      unsigned short *p = (unsigned short *)&data[ch.offset];
      unsigned a = 0;
      unsigned b = ch.num_times - 1;
      unsigned component_size = ch.component_size;
      //app_utils::log("ec %f %d\n", time, time_ms);

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

      //app_utils::log("t=%d a=%d b=%d p[a]=%d p[b]=%d\n", time_ms, a, b, p[a], p[b]);

      unsigned data_offset = ch.offset + ch.num_times * sizeof(unsigned short);

      float t = float(time_ms - p[a]) / (p[b] - p[a]);
      float tmp1[16];
      float tmp2[16];
      if (component_size <= sizeof(tmp1)) {
        memcpy(tmp1, &data[data_offset + a * component_size], component_size);
        memcpy(tmp2, &data[data_offset + b * component_size], component_size);
        for (int i = 0; i != component_size/4; ++i) {
          tmp1[i] = tmp1[i] * (1-t) + tmp2[i] * t;
        }
        //app_utils::log("  t=%f %f %f %f\n", t, tmp1[0], tmp1[1], tmp1[2]);
        target->set_value(ch.sid, ch.sub_target, ch.component, tmp1);
      }
    }
  };
}
