////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// game-style memory allocator
//
// this is a placeholder for a game-style memory allocator
// using malloc and free is frowned upon in grown-up circles.
//
// these functions are poor for the following reasons:
//
// 1) free() has to compute the size of the block to free
// 2) these functions use heavy weight locks to guard the heap.
// 3) implementations are quite variable

namespace octet {
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
    // todo: implement this from scratch using a pool allocator
    static void *malloc(size_t size) {
      state().num_bytes += size;
      void *res = ::malloc(size);
      //printf("malloc %p[%d] -> %d\n", res, size, state().num_bytes);
      return res;
    }

    static void free(void *ptr, size_t size) {
      state().num_bytes -= size;
      //printf("free %p[%d] -> %d\n", ptr, size, state().num_bytes);
      return ::free(ptr);
    }

    static void *realloc(void *ptr, size_t old_size, size_t size) {
      state().num_bytes += size - old_size;
      void *res = ::realloc(ptr, size);
      //printf("realloc %p[%d] -> %p[%d] %d\n", ptr, old_size, res, size, state().num_bytes);
      return res;
    }

    // crude check of stack integrity
    static void test(const char *label) {
      printf("test %s\n", label);
      ::free(::malloc(8192));
      ::free(::malloc(32));
    }
  };
}

