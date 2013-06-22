////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// game-style memory allocator
//
// this is a placeholder for a game-style memory allocator
// using malloc and free is frowned upon in grown-up circles.

class allocator {
  // singleton state, a bit like an old-world global variable
  struct state_t {
    int num_bytes;
  };

  static state_t &state() {
    static state_t instance;
    return instance;
  }

public:
  // todo: implement this from scratch
  static void *malloc(size_t size) {
    //printf("malloc -> %d\n", state().num_bytes);
    state().num_bytes += size;
    return ::malloc(size);
  }

  static void free(void *ptr, size_t size) {
    state().num_bytes -= size;
    //printf("free -> %d\n", state().num_bytes);
    return ::free(ptr);
  }
};

