////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// game-style memory allocator
//

class allocator {
  struct state_t {
    int num_bytes;
  };

  static state_t &state() {
    static state_t instance;
    return instance;
  }
/*
  class state {
    enum {
      pool_granularity = 16,
      num_pools = 16,
      page_size = 65563,
    };

    struct pool {
      
    };

    pool *pools[num_pools];

    // pool size is a compromise between 
    static unsigned which_pool(unsigned bytes) {
      if (bytes <= 16) {
        return 0;
      } else if (bytes <= 32) {
        return 1;
      } else if (bytes <= 48) {
        return 2;
      } else if (bytes <= 64) {
        return 3;
      } else {
      }
    }

    // one page of a virtual memory system
    struct pool {
      unsigned char bytes[page_size - sizeof(pool*)];
      pool *next;
    };

  public:

    state() {
      memset(pools, 0, sizeof(pools[0]) * num_pools);
      assert(sizeof(pool) == page_size);
    }

    void *malloc(size_t size) {
      if (size == 0) return NULL;

      unsigned pool = (size - 1) / pool_granularity;

      if (pool >= num_pools) {
        // todo: use VirtualAlloc for windows etc.
        return ::malloc(size);
      } else {
        if (!pools[pool]) {
          
        }
      }
    }
  };

  // singleton state
  static state &s() {
    state s;
    return s;
  }
  */

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

