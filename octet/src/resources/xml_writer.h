////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// visitor for writing xml files.
//

namespace octet { namespace resources {
  /// Work in progress: serialise an octet tree to write as an XML file.
  /// This is useful for debugging, for example.
  class xml_writer : public visitor {
    dynarray<TiXmlElement *> stack;
    TiXmlElement *root;
    hash_map<void *, int> refs;
    int next_id;

    char hex_digit(unsigned i) {
      return i < 10 ? i + '0' : i + 'a' - 10;
    }

    const char *to_hex(void *data, size_t size) {
      static char tmp[128 + 1];
      assert(size*2 < sizeof(tmp));
      for (size_t i = 0; i != size; ++i) {
        tmp[i*2+0 ] = hex_digit(((uint8_t*)data)[i] >> 4);
        tmp[i*2+1 ] = hex_digit(((uint8_t*)data)[i] & 0x0f);
      }
      tmp[size*2] = 0;
      return tmp;
    }
  public:
    /// create a new XML file in the tiny xml document, root.
    xml_writer(TiXmlElement *root) {
      next_id = 1;
      stack.push_back(root);
      this->root = root;
    }

    bool begin_ref(void *ref, const char *sid, atom_t type) {
      TiXmlElement *parent = stack.back();

      int &id = refs[ref];
      bool is_new = id == 0;

      if (is_new) {
        id = next_id++;
        TiXmlElement *child = new TiXmlElement(app_utils::get_atom_name(type));
        child->SetAttribute("id", id);
        root->LinkEndChild(child);
        stack.push_back(child);
      }

      parent->SetAttribute(sid, id);
      return is_new;
    }

    bool begin_ref(void *ref, atom_t sid, atom_t type) {
      return begin_ref(ref, app_utils::get_atom_name(sid), type);
    }

    bool begin_ref(void *ref, int index, atom_t type) {
      char tmp[32];
      sprintf(tmp, "i%d", index);
      return begin_ref(ref, tmp, type);
    }

    void end_ref() {
      stack.pop_back();
    }

    bool begin_refs(atom_t sid, int &size, bool is_dict) {
      TiXmlElement *child = new TiXmlElement("refs");
      stack.back()->LinkEndChild(child);
      stack.push_back(child);
      //child->SetAttribute("count", number);
      child->SetAttribute("sid", app_utils::get_atom_name(sid));
      return true;
    }

    void end_refs(bool is_dict) {
      stack.pop_back();
    }

    void visit_bin(void *value, unsigned size, atom_t sid, atom_t type) {
      if (size <= 16) {
        stack.back()->SetAttribute(app_utils::get_atom_name(sid), to_hex(value, size));
      } else {
        TiXmlElement *child = new TiXmlElement("data");
        stack.back()->LinkEndChild(child);
        dynarray<char> data;
        data.resize(size*2+1);
        for (unsigned i = 0; i != size; ++i) {
          data[i*2+0 ] = hex_digit(((uint8_t*)value)[i] >> 4);
          data[i*2+1 ] = hex_digit(((uint8_t*)value)[i] & 0x0f);
        }
        data[size*2] = 0;
        TiXmlText *text = new TiXmlText(&data[0]);
        child->LinkEndChild(text);
        child->SetAttribute("sid", app_utils::get_atom_name(sid));
      }
    }

    void visit(int &value, atom_t sid) {
      stack.back()->SetAttribute(app_utils::get_atom_name(sid), value);
    }

    void visit(unsigned &value, atom_t sid) {
      stack.back()->SetAttribute(app_utils::get_atom_name(sid), value);
    }

    void visit(atom_t &value, atom_t sid) {
      stack.back()->SetAttribute(app_utils::get_atom_name(sid), app_utils::get_atom_name(value));
    }

    void visit(vec4 &value, atom_t sid) {
      char tmp[64];
      stack.back()->SetAttribute(app_utils::get_atom_name(sid), value.toString(tmp, sizeof(tmp)));
    }

    void visit(mat4t &value, atom_t sid) {
      char tmp[256];
      stack.back()->SetAttribute(app_utils::get_atom_name(sid), value.toString(tmp, sizeof(tmp)));
    }
  };
} }

