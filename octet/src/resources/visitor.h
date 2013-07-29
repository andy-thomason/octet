////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// generalised visitor for serialisation, web interfaces and RPCs.
//

namespace octet {
  class visitor;

  class visitable {
  public:
    virtual atom_t get_type() = 0;
    virtual void visit(visitor &v) = 0;
  };

  class visitor {
  public:
    // implementation
    virtual bool begin_ref(void *ref, atom_t sid, atom_t type) = 0;
    virtual bool begin_ref(void *ref, int index, atom_t type) = 0;
    virtual bool begin_ref(void *ref, const char *sid, atom_t type) = 0;
    virtual void end_ref() = 0;

    virtual void begin_refs(atom_t sid, int number) = 0;
    virtual void end_refs() = 0;

    virtual void visit_bin(void *value, size_t size, atom_t sid) = 0;

    virtual void visit(int &value, atom_t sid) = 0;
    virtual void visit(atom_t &value, atom_t sid) = 0;

    // visit references
    template <class type> void visit(ref<type> &value, atom_t sid) {
      if (value) {
        if (begin_ref((type*)value, sid, value->get_type())) {
          value->visit(*this);
          end_ref();
        }
      }
    }

    // visit aggregates (like gl_resource)
    void visit_agg(visitable &value, atom_t sid) {
      if (begin_ref(&value, sid, value.get_type())) {
        value.visit(*this);
        end_ref();
      }
    }

    // arrays of references
    template <class type> void visit(dynarray<ref<type> > &value, atom_t sid) {
      begin_refs(sid, value.size());
      for (int i = 0, nv = value.size(); i != nv; ++i) {
        if (value[i]) {
          if (begin_ref((type*)value[i], i, value[i]->get_type())) {
            value[i]->visit(*this);
            end_ref();
          }
        }
      }
      end_refs();
    }

    // dictionaries of references
    template <class type> void visit(dictionary<ref<type> > &value, atom_t sid) {
      begin_refs(sid, value.get_num_indices());
      for (unsigned i = 0, nv = value.get_num_indices(); i != nv; ++i) {
        const char *key = value.get_key(i);
        if (key) {
          type *val = (type*)value.get_value(i);
          if (begin_ref(val, key, val->get_type())) {
            val->visit(*this);
            end_ref();
          }
        }
      }
      end_refs();
    }

    // Plain old data dynarrays
    template <class type> void visit(dynarray<type> &value, atom_t sid) {
      if (value.size()) {
        visit_bin((void*)&value[0], sizeof(type) * value.size(), sid);
      }
    }

    // any other type
    template <class type> void visit(type &value, atom_t sid) {
      visit_bin((void*)&value, sizeof(value), sid);
    }
  };
}

