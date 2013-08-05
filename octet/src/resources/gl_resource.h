////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// a gl_resource can be stored in a gl buffer or allocated memory
//

namespace octet {
  class gl_resource : public visitable {
    dynarray<uint8_t> bytes;
    GLuint buffer;
    GLuint target;

  public:
    RESOURCE_META(gl_resource)

    gl_resource() {
      buffer = 0;
    }

    void visit(visitor &v) {
      v.visit(bytes, atom_bytes);
      v.visit(target, atom_target);
    }

    void allocate(GLuint target, unsigned size, bool use_vbo) {
      release();
      if (use_vbo) {
        glGenBuffers(1, &buffer);
        glBindBuffer(target, buffer);
        glBufferData(target, size, NULL, GL_STATIC_DRAW);
      }
      bytes.resize(size);
      this->target = target;
    }

    void release() {
      if (buffer != 0) {
        glDeleteBuffers(1, &buffer);
      }
      bytes.reset();
      buffer = 0;
    }

    ~gl_resource() {
      release();
    }

    unsigned get_size() const {
      return bytes.size();
    }

    void *get_ptr() const {
      return (void*)&bytes[0];
    }

    void *bind() const {
      if (buffer) {
        glBindBuffer(target, buffer);
        return 0;
      } else {
        return get_ptr();
      }
    }

    void assign(void *ptr, unsigned offset, unsigned size) {
      if (buffer) {
        glBindBuffer(target, buffer);
        glBufferSubData(target, offset, size, ptr);
      }
      assert(offset + size <= this->get_size());
      memcpy((void*)((char*)get_ptr() + offset), ptr, size);
    }

    void copy(const gl_resource &rhs) {
      assign(rhs.get_ptr(), 0, rhs.get_size());
    }

    bool get_use_vbo() const {
      return buffer != 0;
    }
  };
}
