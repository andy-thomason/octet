////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// a gl_resource can be stored in a gl buffer or allocated memory
//

namespace octet {
  class gl_resource : public resource {
    // in GLES2, we need to have a second buffer containing the data
    dynarray<uint8_t> bytes;

    // This buffer object contains the bytes in GPU memory
    GLuint buffer;

    // GL_ARRAY_BUFFER etc.
    GLuint target;

  public:
    RESOURCE_META(gl_resource)

    gl_resource(unsigned target=0, unsigned size=0) {
      buffer = 0;
      this->target = target;
      if (size) {
        allocate(target, size);
      }
    }

    // serialize this object.
    void visit(visitor &v) {
      v.visit(bytes, atom_bytes);
      v.visit(target, atom_target);
    }

    // 
    void allocate(GLuint target, unsigned size) {
      reset();
      glGenBuffers(1, &buffer);
      glBindBuffer(target, buffer);
      glBufferData(target, size, NULL, GL_STATIC_DRAW);
      bytes.resize(size);
      this->target = target;
    }

    void reset() {
      if (buffer != 0) {
        glDeleteBuffers(1, &buffer);
      }
      bytes.reset();
      buffer = 0;
    }

    ~gl_resource() {
      reset();
    }

    unsigned get_target() const {
      return target;
    }

    unsigned get_size() const {
      return bytes.size();
    }

    const void *lock_read_only() const {
      return (void*)&bytes[0];
    }

    void unlock_read_only() const {
    }

    void *lock() const {
      return (void*)&bytes[0];
    }

    void unlock() const {
      glBindBuffer(target, buffer);
      glBufferSubData(target, 0, bytes.size(), &bytes[0]);
    }

    void bind() const {
      glBindBuffer(target, buffer);
    }

    void assign(void *ptr, unsigned offset, unsigned size) {
      assert(offset + size <= this->get_size());

      memcpy((void*)((char*)lock() + offset), ptr, size);
      unlock();
    }

    void copy(const gl_resource *rhs) {
      allocate(rhs->get_target(), rhs->get_size());
      assign(rhs->lock(), 0, rhs->get_size());
      rhs->unlock();
    }
  };
}
