////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// generalised visitor for serialisation, web interfaces and RPCs.
//

namespace octet {
  class visitor {
  public:
    virtual bool begin_ref(void *ref, const char *sid, atom_t type) = 0;
    virtual void end_ref(const char *sid, atom_t type) = 0;
    virtual bool begin_ref(void *ref, int index, atom_t type) = 0;
    virtual void end_ref(int index, atom_t type) = 0;

    virtual void begin_refs(const char *sid, atom_t type, int number) = 0;
    virtual void end_refs(const char *sid, atom_t type, int number) = 0;

    virtual void visit(mat4t &value, const char *name) = 0;
    virtual void visit(float &value, const char *name) = 0;
    virtual void visit(int &value, const char *name) = 0;
    virtual void visit(atom_t &value, const char *name) = 0;

    template <class type> void visit(ref<type> &value, const char *sid) {
      if (value) {
        if (begin_ref((type*)value, sid, value->get_type())) {
          value->visit(*this);
          end_ref(sid, value->get_type());
        }
      }
    }

    template <class type> void visit(dynarray<ref<type>> &value, const char *sid) {
      begin_refs(sid, type::get_type_static(), value.size());
      for (int i = 0, nv = value.size(); i != nv; ++i) {
        if (value[i]) {
          if (begin_ref((type*)value[i], i, value[i]->get_type())) {
            value[i]->visit(*this);
            end_ref(i, value[i]->get_type());
          }
        }
      }
      end_refs(sid, type::get_type_static(), value.size());
    }
  };
}

