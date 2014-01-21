////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Animation resource
//

namespace octet { namespace scene {
  /// Instance of an animation; which Animation, what the target is, current time, etc.
  class animation_instance : public resource {
    ref<animation> anim;
    ref<resource> target;
    float time;
    bool is_looping;
    bool is_paused;
  public:
    RESOURCE_META(animation_instance)

    /// Create an animation instance. Adding this to the scene starts the animation playing.
    animation_instance(animation *anim=0, resource *target=0, bool is_looping=true) {
      this->target = target;
      this->anim = anim;
      this->time = 0;
      this->is_looping = is_looping;
      this->is_paused = false;
    }

    /// serialize the animation
    void visit(visitor &v) {
      v.visit(anim, atom_anim);
      v.visit(target, atom_target);
      v.visit(time, atom_time);
      v.visit(is_looping, atom_is_looping);
      v.visit(is_paused, atom_is_paused);
    }

    /// get the animation
    const animation *get_anim() const {
      return anim;
    }

    /// get the current time.
    float get_time() const {
      return time;
    }

    /// update the animation and the resources it connects to.
    void update(float delta_time) {
      if (target) {
        for (int ch = 0; ch != anim->get_num_channels(); ++ch) {
          anim->eval_chan(ch, time, target);
        }
      } else {
        for (int ch = 0; ch != anim->get_num_channels(); ++ch) {
          resource *anim_target = anim->get_target(ch);
          anim->eval_chan(ch, time, anim_target);
        }
      }

      //log("update %f\n", delta_time);
      if (!is_paused) {
        time += delta_time;
        //log("..update %f\n", time);
        if (time >= anim->get_end_time()) {
          if (is_looping) {
            time -= anim->get_end_time();
          } else {
            is_paused = true;
          }
        }
      }
    }
  };
}}
