////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// work in progress

namespace octet { namespace resources {
  class job : public resource {
    ref<job> next;

    enum state_t {
      state_waiting,
      state_running,
    };

    state_t state;

    class scheduler {
      job *waiting;
      job *running;
    public:
      scheduler() {
        waiting = 0;
        running = 0;
      }

      void add_to_waiting(job *jb) {
        jb->next = waiting;
        waiting = this;
      }
    };

    scheduler *get_scheduler() {
      static scheduler sch;
      return &sch;
    }
  public:
    job() {
      scheduler *sch = get_scheduler();
      sch->add_to_waiting(this);
    }

    virtual ~job() {
    }

    virtual void kernel() = 0;
    virtual bool is_ready() = 0;

    state_t get_state() {
      return state;
    }
  };
} }
