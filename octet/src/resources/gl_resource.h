////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// a gl_resource can be stored in a gl buffer or allocated memory
//

namespace octet { namespace resources {
  /// Wrapper for an OpenGL resource.
  class gl_resource : public resource {
    #ifdef OCTET_GLES2
      // in GLES2, we need to have a second buffer containing the data
      dynarray<uint8_t> bytes;
    #else
      size_t size;
    #endif

    // This buffer object contains the bytes in GPU memory
    GLuint buffer;

    // GL_ARRAY_BUFFER etc.
    GLuint target;

  public:
    /// Helper class to make a write-only lock
    class wolock {
      gl_resource *res;
      void *ptr;
    public:
      wolock(gl_resource *res) { this->res = res; ptr = res->lock_write_only(); }
      ~wolock() { res->unlock_write_only(); }
      uint8_t *u8() const { return (uint8_t*)ptr; }
      uint16_t *u16() const { return (uint16_t*)ptr; }
      uint32_t *u32() const { return (uint32_t*)ptr; }
      float *f32() const { return (float*)ptr; }
    };

    /// Helper class to make a read-write lock
    class rwlock {
      gl_resource *res;
      void *ptr;
    public:
      rwlock(gl_resource *res) { this->res = res; ptr = res->lock(); }
      ~rwlock() { res->unlock(); }
      uint8_t *u8() const { return (uint8_t*)ptr; }
      uint16_t *u16() const { return (uint16_t*)ptr; }
      uint32_t *u32() const { return (uint32_t*)ptr; }
      float *f32() const { return (float*)ptr; }
      vec4 *v4() const { return (vec4*)ptr; }
    };

    /// Helper class to make a read-only lock
    class rolock {
      gl_resource *res;
      const void *ptr;
    public:
      rolock(gl_resource *res) { this->res = res; ptr = res->lock_read_only(); }
      ~rolock() { res->unlock_read_only(); }
      const uint8_t *u8() const { return (const uint8_t*)ptr; }
      const uint16_t *u16() const { return (const uint16_t*)ptr; }
      const uint32_t *u32() const { return (const uint32_t*)ptr; }
      const float *f32() const { return (const float*)ptr; }
      const vec4 *v4() const { return (const vec4*)ptr; }
    };
  public:
    RESOURCE_META(gl_resource)

    /// Make a new OpenGL Resource
    gl_resource(unsigned target=0, unsigned size=0) {
      buffer = 0;
      this->target = target;
      if (size) {
        allocate(target, size);
      }
    }

    /// serialize this object.
    void visit(visitor &v) {
      #ifdef OCTET_GLES2
        v.visit(bytes, atom_bytes);
      #endif
      v.visit(target, atom_target);
    }

    /// Allocate a new OpenGL object.
    void allocate(GLuint target, size_t size, GLuint kind = GL_STATIC_DRAW) {
      reset();
      glGenBuffers(1, &buffer);
      glBindBuffer(target, buffer);
      glBufferData(target, size, NULL, kind);
      #ifdef OCTET_GLES2
        bytes.resize(size);
      #else
        this->size = size;
      #endif
      this->target = target;
      glBindBuffer(target, 0);
    }

    /// Clear the OpenGL object
    void reset() {
      if (buffer != 0) {
        glDeleteBuffers(1, &buffer);
      }
      #ifdef OCTET_GLES2
        bytes.reset();
      #endif
      buffer = 0;
    }

    /// Destructor
    ~gl_resource() {
      reset();
    }

    /// get the target this resource is bound to
    unsigned get_target() const {
      return target;
    }

    /// get the buffer size
    size_t get_size() const {
      #ifdef OCTET_GLES2
        return bytes.size();
      #else
        return size;
      #endif
    }

    /// get the GL buffer object we are wrapping.
    GLuint get_buffer() const {
      return buffer;
    }

    /// get a read-only lock on this buffer
    /// deprecated
    const void *lock_read_only() const {
      #ifdef OCTET_GLES2
        return (const void*)&bytes[0];
      #else
        glBindBuffer(target, buffer);
        #ifdef __APPLE__
          // OSX does not support glMapBufferRange 
          return glMapBuffer(target, GL_READ_ONLY);
        #else
          return glMapBufferRange(target, 0, size, GL_MAP_READ_BIT);
        #endif
      #endif
    }

    /// release read-only lock on this buffer
    /// deprecated
    void unlock_read_only() const {
      #ifndef OCTET_GLES2
        glBindBuffer(target, buffer);
        glUnmapBuffer(target);
      #endif
    }

    /// get a read-write lock on this buffer. Do not use this by preference.
    /// deprecated
    void *lock() const {
      #ifdef OCTET_GLES2
        return (void*)&bytes[0];
      #else
        glBindBuffer(target, buffer);
        #ifdef __APPLE__
          // OSX does not support glMapBufferRange 
          void *res = glMapBuffer(target, GL_READ_WRITE);
          return res;
        #else
          return glMapBufferRange(target, 0, size, GL_MAP_WRITE_BIT|GL_MAP_WRITE_BIT);
        #endif
      #endif
    }

    /// release a read-write lock
    /// deprecated
    void unlock() const {
      #ifdef OCTET_GLES2
        glBindBuffer(target, buffer);
        glBufferSubData(target, 0, bytes.size(), &bytes[0]);
      #else
        glUnmapBuffer(target);
      #endif
    }

    /// get a read-write lock on this buffer
    /// deprecated
    void *lock_write_only() const {
      #ifdef OCTET_GLES2
        return (void*)&bytes[0];
      #else
        glBindBuffer(target, buffer);
        #ifdef __APPLE__
          // OSX does not support glMapBufferRange 
          return glMapBuffer(target, GL_WRITE_ONLY);
        #else
          return glMapBufferRange(target, 0, size, GL_MAP_WRITE_BIT);
        #endif
      #endif
    }

    /// release a read-write lock
    /// deprecated
    void unlock_write_only() const {
      #ifdef OCTET_GLES2
        glBindBuffer(target, buffer);
        glBufferSubData(target, 0, bytes.size(), &bytes[0]);
      #else
        glUnmapBuffer(target);
      #endif
    }

    /// bind the resource to the target
    void bind() const {
      glBindBuffer(target, buffer);
    }

    /// copy data into the resource
    void assign(const void *ptr, size_t offset, size_t size) {
      assert(offset + size <= this->get_size());

      memcpy((void*)((char*)lock_write_only() + offset), ptr, size);
      unlock_write_only();
    }

    /// copy data from another gl resource.
    void copy(const gl_resource *rhs) {
      allocate(rhs->get_target(), rhs->get_size());
      assign(rhs->lock_read_only(), 0, rhs->get_size());
      rhs->unlock_read_only();
    }
  };
} }
