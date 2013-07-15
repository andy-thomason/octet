////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Animation resource
//

class animation_instance : public resource {
  ref<animation> anim;
  ref<animation_target> target;
  float time;
  bool is_looping;
  bool is_paused;
public:
  RESOURCE_META(animation_instance)

  animation_instance(animation *anim, animation_target *target, bool is_looping) {
    this->target = target;
    this->anim = anim;
    this->time = 0;
    this->is_looping = is_looping;
    this->is_paused = false;
  }

  const animation *get_anim() const {
    return anim;
  }

  float get_time() const {
    return time;
  }

  bool update_time(float delta_time) {
    time += delta_time;
    if (time >= anim->get_end_time()) {
      if (is_looping) {
        time -= anim->get_end_time();
      } else {
        return true;
      }
    }
    return false;
  }
};
