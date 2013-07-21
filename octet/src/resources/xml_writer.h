////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// visitor for writing xml files.
//

namespace octet {
  class xml_writer : public visitor {
    int depth;
    FILE *file;
    hash_map<void *, int> refs;
    int id;
  public:
    xml_writer(FILE *file) {
      depth = 0;
      this->file = file;
      id = 1;
    }

    bool begin_ref(void *ref, const char *sid, const char *type) {
      int &prev = refs[ref];
      if (!prev) {
        fprintf(file, "%*s<%s sid=%s id=%d>\n", depth*2, "", type, sid, id);
        depth++;
        prev = id++;
        return true;
      } else {
        fprintf(file, "%*s<%s sid=%s id=%d/>\n", depth*2, "", type, sid, prev);
        return false;
      }
    }

    void end_ref(const char *sid, const char *type) {
      depth--;
      fprintf(file, "%*s</%s>\n", depth*2, "", type);
    }

    bool begin_ref(void *ref, int index, const char *type) {
      int &prev = refs[ref];
      const char *sid = "";
      if (!prev) {
        fprintf(file, "%*s<%s sid=%s id=%d>\n", depth*2, "", type, sid, id);
        depth++;
        prev = id++;
        return true;
      } else {
        fprintf(file, "%*s<%s sid=%s id=%d/>\n", depth*2, "", type, sid, prev);
        return false;
      }
    }

    void end_ref(int index, const char *type) {
      depth--;
      fprintf(file, "%*s</%s>\n", depth*2, "", type);
    }


    void begin_refs(const char *sid, const char *type, int number) {
      fprintf(file, "%*s<%s>\n", depth*2, "", type);
      depth++;
    }

    void end_refs(const char *sid, const char *type, int number) {
      depth--;
      fprintf(file, "%*s</%s>\n", depth*2, "", type);
    }


    void visit(mat4t &value, const char *sid) {
      fprintf(file, "%*s<mat4t sid=%s value=\"%s\"/>\n", depth*2, "", sid, value.toString());
    }

    void visit(float &value, const char *sid) {
      fprintf(file, "%*s<float sid=%s value=\"%f\"/>\n", depth*2, "", sid, value);
    }

    void visit(int &value, const char *sid) {
      fprintf(file, "%*s<int sid=%s value=\"%d\"/>\n", depth*2, "", sid, value);
    }

    void visit(atom_t &value, const char *sid) {
      fprintf(file, "%*s<int sid=%s value=\"%d\"/>\n", depth*2, "", sid, (int)value);
    }
  };
}

