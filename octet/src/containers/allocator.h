////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
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

// this is a dummy class used to customise the placement new and delete
struct dynarray_dummy_t {};

// placement new operator, allows construction in-place at "place"
void *operator new(size_t size, void *place, dynarray_dummy_t x) { return place; }

// dummy placement delete operator, allows destruction at "place"
void operator delete(void *ptr, void *place, dynarray_dummy_t x) {}

// generate hungarian forms of types (abbreviations of variants of types)
// eg. vec3_in is used for input args of type vec3
#define OCTET_HUNGARIANS(name) \
  class name; \
  typedef const name &name##_in; \
  typedef name &name##_out; \
  typedef name name##_ret; \
  typedef const name *name##_pc; \
  typedef name *name##_p; \
  typedef const name &name##_rc; \
  typedef name &name##_r;

// generate hungarian forms of types (abbreviations of variants of types)
// eg. vec3_in is used for input args of type vec3
#define OCTET_HUNGARIANS_NC(name) \
  typedef const name &name##_in; \
  typedef name &name##_out; \
  typedef name name##_ret; \
  typedef const name *name##_pc; \
  typedef name *name##_p; \
  typedef const name &name##_rc; \
  typedef name &name##_r;


namespace octet { namespace containers {
  class allocator {
    // singleton state, a bit like an old-world global variable
    struct state_t {
      size_t num_bytes;
    };

    static state_t &state() {
      static state_t instance;
      return instance;
    }

  public:
    // todo: implement this from scratch using a pool allocator
    static void *malloc(size_t size) {
      state().num_bytes += size;
      #if OCTET_MAC
        void *res = 0;
        posix_memalign(&res, 16, size);
      #elif OCTET_SSE
        void *res = ::_aligned_malloc(size, 16);
      #elif OCTET_VITA
        void *res = ::memalign(size, 16);
      #else
        void *res = ::malloc(size);
      #endif
      //printf("malloc %p[%d] -> %d\n", res, size, state().num_bytes);
      return res;
    }

    static void free(void *ptr, size_t size) {
      state().num_bytes -= size;
      //printf("free %p[%d] -> %d\n", ptr, size, state().num_bytes);
      #if OCTET_MAC
        return ::free(ptr);
      #elif OCTET_SSE
        return ::_aligned_free(ptr);
      #else
        return ::free(ptr);
      #endif
    }

    static void *realloc(void *ptr, size_t old_size, size_t size) {
      state().num_bytes += size - old_size;
      #if OCTET_MAC
        void *res = ::realloc(ptr, size);
      #elif OCTET_SSE
        void *res = ::_aligned_realloc(ptr, size, 16);
      #else
        void *res = ::realloc(ptr, size);
      #endif
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
} }

