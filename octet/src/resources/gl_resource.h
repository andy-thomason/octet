////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// a gl_resource can be stored in a gl buffer or allocated memory
//

namespace octet {
  class gl_resource {
    void *ptr;
    unsigned size;
    GLuint buffer;
    GLuint target;

  public:
    gl_resource() {
      ptr = 0;
      size = 0;
      buffer = 0;
    }

    void allocate(GLuint target, unsigned size, bool use_vbo) {
      release();
      if (use_vbo) {
        glGenBuffers(1, &buffer);
        glBindBuffer(target, buffer);
        glBufferData(target, size, NULL, GL_STATIC_DRAW);
      } else {
        ptr = allocator::malloc(size);
        this->size = size;
      }
      this->target = target;
    }

    void release() {
      if (buffer != 0) {
        glDeleteBuffers(1, &buffer);
      } else {
        if (ptr) {
          allocator::free(ptr, size);
        }
      }
      ptr = 0;
      size = 0;
      buffer = 0;
    }

    ~gl_resource() {
      release();
    }

    unsigned get_size() const {
      return size;
    }

    void *get_ptr() const {
      return buffer != 0 ? 0 : ptr;
    }

    void *bind() const {
      if (buffer) {
        glBindBuffer(target, buffer);
        return 0;
      } else {
        return ptr;
      }
    }

    void assign(void *ptr, unsigned offset, unsigned size) {
      if (buffer) {
        glBindBuffer(target, buffer);
        glBufferSubData(target, offset, size, ptr);
      } else {
        assert(offset + size <= this->size);
        memcpy((void*)((char*)this->ptr + offset), ptr, size);
      }
    }

    void copy(const gl_resource &rhs) {
      allocate(rhs.target, rhs.size, rhs.buffer != 0);
      release();
      if (rhs.buffer) {
        glBindBuffer(target, buffer);
        glBindBuffer(GL_COPY_READ_BUFFER, rhs.buffer);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, target, 0, 0, size);
      } else {
        assign(rhs.ptr, 0, rhs.size);
      }
    }

    bool get_use_vbo() const {
      return buffer != 0;
    }
  };
}
