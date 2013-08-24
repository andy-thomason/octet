////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
//

namespace octet {
  class job : public resource {
    ref<job> next;

    enum state_t {
      state_ready,
      state_running,
      state_finished,
    };

    class scheduler {
    public:
      scheduler() {
      }
    };

    scheduler *get_scheduler() {
      static scheduler;
      return &scheduler;
    }
  public:
    job() {
    }

    virtual ~job() {
    }

    virtual void run() = 0;
  };
}
