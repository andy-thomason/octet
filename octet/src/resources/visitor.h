////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//

namespace octet { namespace resources {
  class visitor;

  class visitable {
  public:
    virtual ~visitable() = 0;
    virtual atom_t get_type() = 0;
    virtual void visit(visitor &v) = 0;
  };

  /// A Generalised visitor for serialisation, scripting, web interfaces and RPCs.
  ///
  /// A visitor pattern can be used to solve a number of problems and provides
  /// "Metadata" for the classes.
  class visitor {
    enum { debug = true };
    unsigned depth;
    bool error;

    void begin_visit(atom_t type) {
      if (debug) log("%*svisit %s\n", get_depth()*2, "", app_utils::get_atom_name(type));
      depth++;
    }

    void end_visit(atom_t type) {
      depth--;
      if (debug) log("%*svisit %s\n", get_depth()*2, "", app_utils::get_atom_name(type));
    }
  public:
    /// Constructor. This will be called by derived classes.
    visitor() {
      depth = 0;
      error = false;
    }

    /// Destructor. This will be called by derived classes.
    /// Note this is a virtual destructor.
    virtual ~visitor() {
    }

    /// Depth for debugging. Used when tracing visits.
    unsigned get_depth() {
      return depth;
    }

    /// If we get an error while scanning, set this value.
    void set_error(bool value) {
      error = value;
    }

    /// returns true if an error has occured
    bool get_error() {
      return error;
    }

    /// Begin a reference that is part of a structure. begin_ref returns true if we need to recurse.
    virtual bool begin_ref(void *ref, atom_t sid, atom_t type) = 0;

    /// Begin a reference that is part of an array.
    virtual bool begin_ref(void *ref, int index, atom_t type) = 0;

    /// Begin a reference that is part of a dictionary.
    virtual bool begin_ref(void *ref, const char *sid, atom_t type) = 0;

    /// End a reference
    virtual void end_ref() = 0;

    /// Begin an array or dictionary.
    virtual bool begin_refs(atom_t sid, int &size, bool is_dict) = 0;

    /// End an array or dictionary.
    virtual void end_refs(bool is_dict) = 0;

    /// Implement this for "blobs" binary large objects
    virtual void visit_bin(void *value, size_t size, atom_t sid, atom_t type) = 0;

    /// Implement this for octet strings
    virtual void visit_string(string &value, atom_t sid) {}

    /// Implement this for readers.
    virtual bool is_reader() { return false; }

    /// Implement this to read/write references
    virtual bool begin_read_ref(void *&ref, atom_t &sid, atom_t &type) { return false; }

    /// Implement this to read/write arrays
    virtual bool begin_read_ref(void *&ref, int index, atom_t &type) { return false; }

    /// Implement this to read/write dictionaries
    virtual bool begin_read_ref(void *&ref, const char *&sid, atom_t &type) { return false; }

    /// Implement this to read/write dynarrays
    virtual unsigned begin_read_dynarray(unsigned elem_size, atom_t &sid) { return 0; }

    /// Implement this to read/write dynarrays
    virtual void end_read_dynarray(void *ptr, unsigned bytes) {}

    /// readers use this to add a new reference
    virtual void add_new_ref(void *ref) {}

    /// begin an aggregate
    virtual bool begin_agg(void *ref, atom_t sid, atom_t type) { return true; }

    /// end an aggregate
    virtual void end_agg() {}

    /// Call this in your "visit" method
    void visit(int8_t &value, atom_t sid) {
      visit_bin(&value, sizeof(value), sid, atom_int8);
    }

    /// Call this in your "visit" method
    void visit(int16_t &value, atom_t sid) {
      visit_bin(&value, sizeof(value), sid, atom_int16);
    }

    /// Call this in your "visit" method
    void visit(int32_t &value, atom_t sid) {
      visit_bin(&value, sizeof(value), sid, atom_int32);
    }

    /// Call this in your "visit" method
    void visit(uint8_t &value, atom_t sid) {
      visit_bin(&value, sizeof(value), sid, atom_uint8);
    }

    /// Call this in your "visit" method
    void visit(uint16_t &value, atom_t sid) {
      visit_bin(&value, sizeof(value), sid, atom_uint16);
    }

    /// Call this in your "visit" method
    void visit(uint32_t &value, atom_t sid) {
      visit_bin(&value, sizeof(value), sid, atom_uint32);
    }

    /// Call this in your "visit" method
    void visit(mat4t &value, atom_t sid) {
      visit_bin(&value, sizeof(value), sid, atom_mat4t);
    }

    /// Call this in your "visit" method
    void visit(vec2 &value, atom_t sid) {
      visit_bin(&value, sizeof(value), sid, atom_vec2);
    }

    /// Call this in your "visit" method
    void visit(vec3 &value, atom_t sid) {
      visit_bin(&value, sizeof(value), sid, atom_vec3);
    }

    /// Call this in your "visit" method
    void visit(vec4 &value, atom_t sid) {
      visit_bin(&value, sizeof(value), sid, atom_vec4);
    }

    /// Call this in your "visit" method
    void visit(atom_t &value, atom_t sid) {
      visit_bin(&value, sizeof(value), sid, atom_atom);
    }

    /// Call this in your "visit" method
    void visit(string &value, atom_t sid) {
      visit_string(value, sid);
    }

    /// Call this in your "visit" method for references
    template <class type> void visit(ref<type> &value, atom_t sid) {
      if (error) return;
      if (is_reader()) {
        atom_t type_name = atom_;
        void *ref = 0;
        if (begin_read_ref(ref, sid, type_name)) {
          if (!ref && type_name != atom_) {
            type *val = (type*)type::new_type(type_name);
            if (!val) {
              log("unable to make type %s\n", app_utils::get_atom_name(type_name));
              set_error(true);
              return;
            }
            value = val;
            add_new_ref((void*)val);
            begin_visit(type_name);
            val->visit(*this);
            if (error) return;
            end_visit(type_name);
            end_ref();
          } else {
            value = (type*)ref;
          }
        }
      } else {
        atom_t type_name = value ? value->get_type() : atom_;
        if (begin_ref((type*)value, sid, type_name)) {
          begin_visit(type_name);
          value->visit(*this);
          if (error) return;
          end_visit(type_name);
          end_ref();
        }
      }
    }

    /// Call this in your "visit" method for aggregates (sub-structures etc.)
    void visit_agg(visitable &value, atom_t sid) {
      if (error) return;
      if (begin_agg(&value, sid, value.get_type())) {
        value.visit(*this);
        end_agg();
      }
    }

    /// Call this in your "visit" method for dynarrays of references
    template <class type> void visit(dynarray<ref<type> > &value, atom_t sid) {
      if (error) return;
      int size = value.size();
      if (begin_refs(sid, size, false)) {
        if (is_reader()) {
          value.resize(size);
          for(int key = 0; key != size; ++key) {
            atom_t type_name = atom_;
            void *ref = 0;
            if (!begin_read_ref(ref, key, type_name)) break;

            if (!ref && type_name != atom_) {
              type *val = (type*)type::new_type(type_name);
              if (!val) {
                log("unable to make type %s\n", app_utils::get_atom_name(type_name));
                set_error(true);
                return;
              }
              value[key] = val;
              add_new_ref((void*)val);
              begin_visit(type_name);
              val->visit(*this);
              if (error) return;
              end_visit(type_name);
              end_ref();
            } else {
              value[key] = (type*)ref;
            }
          }
        } else {
          for (int i = 0, nv = value.size(); i != nv; ++i) {
            atom_t type_name = value[i] ? value[i]->get_type() : atom_;
            if (begin_ref((type*)value[i], i, type_name)) {
              begin_visit(type_name);
              value[i]->visit(*this);
              if (error) return;
              end_visit(type_name);
              end_ref();
            }
          }
        }
        end_refs(false);
      }
    }

    /// Call this in your "visit" method for dictionaries
    template <class type> void visit(dictionary<ref<type> > &value, atom_t sid) {
      if (error) return;
      int size = value.get_size();
      if (begin_refs(sid, size, true)) {
        if (is_reader()) {
          for(int idx = 0; idx != size; ++idx) {
            atom_t type_name = atom_;
            const char *key = 0;
            void *ref = 0;
            if (!begin_read_ref(ref, key, type_name)) break;
            if (!ref && type_name != atom_) {
              type *val = (type*)type::new_type(type_name);
              if (!val) {
                log("unable to make type %s\n", app_utils::get_atom_name(type_name));
                set_error(true);
                return;
              }
              value[key] = val;
              add_new_ref((void*)val);
              begin_visit(type_name);
              val->visit(*this);
              if (error) return;
              end_visit(type_name);
              end_ref();
            } else {
              value[key] = (type*)ref;
            }
          }
        } else {
          unsigned num_keys = 0;
          for (unsigned i = 0, nv = value.get_num_indices(); i != nv; ++i) {
            const char *key = value.get_key(i);
            if (key) {
              num_keys++;
              type *val = (type*)value.get_value(i);
              atom_t type_name = val ? val->get_type() : atom_;
              if (begin_ref(val, key, type_name)) {
                begin_visit(type_name);
                val->visit(*this);
                if (error) return;
                end_visit(type_name);
                end_ref();
              }
            }
          }
          assert(num_keys == value.get_size());
        }
        end_refs(true);
      }
    }

    /// Call this in your "visit" method for dynarrays of POD types (except references)
    template <class type> void visit(dynarray<type> &value, atom_t sid) {
      if (error) return;
      if (is_reader()) {
        unsigned size = begin_read_dynarray(sizeof(value[0]), sid);
        value.resize(size);
        end_read_dynarray((void*)&value[0], sizeof(type) * value.size());
      } else {
        if (value.size()) {
          visit_bin((void*)&value[0], sizeof(type) * value.size(), sid, atom_dynarray);
        } else {
          visit_bin(NULL, 0, sid, atom_dynarray);
        }
      }
    }

    /// Call this in your "visit" method for any other type.
    template <class type> void visit(type &value, atom_t sid) {
      visit_bin((void*)&value, sizeof(value), sid, atom_unknown);
    }
  };
} }

