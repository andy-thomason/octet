////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// 3D mesh container
//

// mesh builder class for standard meshes.
class collada_builder {
  TiXmlDocument doc;
  dictionary<TiXmlElement *, allocator> ids;

  // find all the ids in an xml file
  void find_ids(TiXmlElement *parent) {
    for (TiXmlElement *elem = parent->FirstChildElement(); elem; elem = elem->NextSiblingElement()) {
      const char *attrib = elem->Attribute("id");
      if (attrib) {
        ids[attrib] = elem;
      }
      find_ids(elem);
    }
  }

  TiXmlElement *find_id(const char *source) {
    if (source) {
      if (source[0] == '#') source++;
      return ids[source];
    }
    return 0;
  }

  int semantic_to_attr(const char *semantic, const char *set) {
    int int_set = set ? atoi(set) : 0;
    if (!strcmp(semantic, "POSITION")) {
      return mesh_state::attribute_pos;
    } else if (!strcmp(semantic, "NORMAL")) {
      return mesh_state::attribute_normal;
    } else if (!strcmp(semantic, "TEXCOORD")) {
      return (int)mesh_state::attribute_uv + int_set;
    } else if (!strcmp(semantic, "COLOR")) {
      return mesh_state::attribute_color;
    } else
    {
      return 8;
    }
  }

  void atofv(dynarray<float> &values, const char *src) {  
    while (*src > 0 && *src <= ' ') ++src;
    while(*src != 0) {
      double whole = 0, msign = 1;
      if (*src == '-') { msign = -1; src++; }
      if( !(*src >= '0' && *src <= '9') && *src != '.' ) break;
      while (*src >= '0' && *src <= '9') whole = whole * 10 + (*src++ - '0');
      if (*src == '.') {
        src++;
        double frac = 0, v = 1;
        while (*src >= '0' && *src <= '9') { frac = frac * 10 + (*src++ - '0'); v *= 10; }
        whole += frac / v;
      }
      if (*src == 'e' || *src == 'E') {
        int esign = 1;
        src++;
        if (*src == '-') { esign = -1; src++; }
        else if (*src == '+') src++;
        int exp = 0;
        while (*src >= '0' && *src <= '9') { exp = exp * 10 + (*src++ - '0'); }
        whole = whole * pow(10.0, exp * esign);
      }
      values.push_back((float)(whole * msign));
      while (*src > 0 && *src <= ' ') ++src;
    }
  }

  // convert an ascii sequence of integers to an array of integers
  void atoiv(dynarray<int> &values, const char *src) {  
    while (*src > 0 && *src <= ' ') ++src;
    while(*src != 0) {
      int whole = 0, msign = 1;
      if (*src == '-') { msign = -1; src++; }
      while (*src >= '0' && *src <= '9') whole = whole * 10 + (*src++ - '0');
      values.push_back(whole * msign);
      while (*src > 0 && *src <= ' ') ++src;
    }
  }

  struct parse_input_state {
    mesh_state *s;
    dynarray<int> p;
    dynarray<float> vertices;
    dynarray<unsigned short> indices;
    unsigned attr_offset;
    unsigned attr_stride;
    unsigned input_offset;
    unsigned input_stride;
    unsigned slot;
    int pass;
  };

  void parse_input(parse_input_state &state, TiXmlElement *input) {
    const char *source = input->Attribute("source");
    const char *semantic = input->Attribute("semantic");
    const char *set = input->Attribute("set");

    if (!source || !semantic) {
      printf("warning: bad input\n");
      return;
    }

    TiXmlElement *source_elem = source ? find_id(source) : 0;
    if (!source_elem) {
      printf("warning: source not found\n");
      return;
    }

    if (!strcmp(source_elem->Value(), "vertices")) {
      for (
        TiXmlElement *input = source_elem->FirstChildElement("input");
        input != 0;
        input = input->NextSiblingElement("input")
      ) {
        return parse_input(state, input);
      }
    }

    if (strcmp(source_elem->Value(), "source")) {
      printf("warning: source not found\n");
      return;
    }

    TiXmlElement *tc = source_elem->FirstChildElement("technique_common");
    if (!tc) {
      printf("warning: no technique_common\n");
      return;
    }

    TiXmlElement *accessor = tc->FirstChildElement("accessor");
    if (!accessor) {
      printf("warning: no accessor\n");
      return;
    }

    const char *accessor_source = accessor->Attribute("source");
    //const char *accessor_offset = accessor->Attribute("offset");
    const char *accessor_stride = accessor->Attribute("stride");
    //int accessor_offset_int = accessor_offset ? atoi(accessor_offset) : 0;
    int accessor_stride_int = accessor_stride ? atoi(accessor_stride) : 0;
    TiXmlElement *accessor_source_elem = accessor_source ? find_id(accessor_source) : 0;

    if (!accessor_source_elem || accessor_stride_int == 0) {
      printf("warning: bad or no accessor source\n");
      return;
    }

    unsigned size = 0;
    const char *param_type = 0;
    for (
      TiXmlElement *param = accessor->FirstChildElement("param");
      param != 0;
      param = param->NextSiblingElement("param")
    ) {
      param_type = param->Attribute("type");
      if (param_type == 0 || strcmp(param_type, "float")) {
        printf("warning: params must be float\n");
        return;
      }
      size++;
    }

    if (state.pass == 1) {
      unsigned attr = semantic_to_attr(semantic, set);
      state.s->add_attribute(attr, size, GL_FLOAT, state.attr_offset * 4);
      state.attr_offset += size;
    } else {
      unsigned p_size = state.p.size();
      unsigned num_vertices = p_size / state.input_stride;

      dynarray<float> accessor_floats;
      atofv(accessor_floats, accessor_source_elem->GetText());
      for (unsigned i = 0; i != num_vertices; ++i) {
        unsigned index = state.p[i * state.input_stride + state.input_offset];
        //printf("%d ", index);
        for (unsigned j = 0; j != size; ++j) {
          //printf("i=%d j=%d idx=%d\n", i, j, index);
          unsigned dest_idx = i * state.attr_stride + state.attr_offset + j;
          unsigned src_idx = index * accessor_stride_int + j;
          if (dest_idx >= state.vertices.size()) {
            printf("dest_idx >= state.vertices.size()\n");
            return;
          }
          if (src_idx >= accessor_floats.size()) {
            printf("src_idx >= accessor_floats.size()\n");
            return;
          }
          state.vertices[dest_idx] = accessor_floats[src_idx];
        }
      }
      state.slot++;
      state.attr_offset += size;
    }
  }

public:
  collada_builder() {
  }

  void load(const char *url) {
    doc.LoadFile(app_utils::get_path(url));
    TiXmlElement *top = doc.RootElement();
    if (!top || strcmp(top->Value(), "COLLADA")) {
      printf("warning: not a collada file");
      return;
    }
    find_ids(top);
  }

  void get_mesh_state(mesh_state &s, const char *id) {
    s.init();

    TiXmlElement *geometry = ids[id];
    if (!geometry || strcmp(geometry->Value(), "geometry")) {
      printf("warning: geometry %s not found\n", id);
      return;
    }

    TiXmlElement *mesh = geometry->FirstChildElement("mesh");
    if (!mesh) {
      printf("warning: geometry %s has no mesh\n", id);
      return;
    }
    for (TiXmlElement *mesh_child = mesh->FirstChildElement();
      mesh_child != NULL;
      mesh_child = mesh_child->NextSiblingElement()
    ) {
      if (
        !strcmp(mesh_child->Value(), "triangles") ||
        !strcmp(mesh_child->Value(), "polylist")
      ) {
        TiXmlElement *pelem = mesh_child->FirstChildElement("p");
        if (!pelem) {
          printf("warning: %s has no %s\n", mesh_child->Value(), id);
          return;
        }

        parse_input_state state;
        state.s = &s;
        atoiv(state.p, pelem->GetText());
        state.input_stride = 1;
        unsigned implicit_offset = 0;
        state.slot = 0;
        state.attr_offset = 0;

        // find the maximum offset
        for (TiXmlElement *poly_child = mesh_child->FirstChildElement("input");
          poly_child != NULL;
          poly_child = poly_child->NextSiblingElement("input")
        ) {
          const char *offset = poly_child->Attribute("offset");
          unsigned int_offset = offset ? atoi(offset) : implicit_offset++;
          if (int_offset+1 > state.input_stride) {
            state.input_stride = int_offset+1;
          }
        }

        unsigned p_size = state.p.size();
        if (p_size % state.input_stride != 0) {
          printf("warning: expected multiple of %d indices\n", state.input_stride);
          return;
        }

        unsigned num_vertices = p_size / state.input_stride;

        // find the output size
        //unsigned attr_stride = 0;
        for (TiXmlElement *input = mesh_child->FirstChildElement("input");
          input != NULL;
          input = input->NextSiblingElement("input")
        ) {
          const char *offset = input->Attribute("offset");
          state.input_offset = offset ? atoi(offset) : 0;
          state.pass = 1;
          parse_input(state, input);
        }

        state.attr_stride = state.attr_offset;
        state.vertices.resize(state.attr_stride * num_vertices);
        state.input_offset = 0;
        state.attr_offset = 0;

        // build the attributes
        for (TiXmlElement *input = mesh_child->FirstChildElement("input");
          input != NULL;
          input = input->NextSiblingElement("input")
        ) {
          const char *offset = input->Attribute("offset");
          state.input_offset = offset ? atoi(offset) : 0;
          state.pass = 2;
          parse_input(state, input);
        }

        unsigned num_indices = 0;

        // if we have a vcount element (polylist), we build polygons out of triangles
        // and hope they are convex!
        TiXmlElement *vcount_elem = mesh_child->FirstChildElement("vcount");
        if (vcount_elem) {
          dynarray<int> vcount;
          atoiv(vcount, vcount_elem->GetText());
          for (unsigned i = 0; i != vcount.size(); ++i) {
            unsigned nv = vcount[i];
            num_indices += (nv - 2) * 3;
          }
          state.indices.resize(num_indices);

          unsigned j = 0;
          unsigned z = 0;
          for (unsigned i = 0; i != vcount.size(); ++i) {
            unsigned nv = vcount[i];
            for (unsigned k = 0; k != nv - 2; ++k) {
              state.indices[j++] = z;
              state.indices[j++] = z + k + 1;
              state.indices[j++] = z + k + 2;
            }
            z += nv;
          }
        } else {
          state.indices.resize(num_vertices);
          num_indices = num_vertices;
          for (unsigned i = 0; i != num_vertices; ++i) {
            state.indices[i] = i;
          }
        }

        unsigned isize = state.indices.size() * sizeof(state.indices[0]);
        unsigned vsize = state.vertices.size() * sizeof(state.vertices[0]);

        s.allocate(vsize, isize, app_common::can_use_vbos());
        s.assign(vsize, isize, (unsigned char*)&state.vertices[0], (unsigned char*)&state.indices[0]);
        s.set_params(state.attr_stride * 4, num_indices, num_vertices, GL_TRIANGLES, GL_UNSIGNED_SHORT);
        /*FILE *file = fopen("c:\\tmp\\3.txt","wb");
        s.dump(file);
        fclose(file);*/
      }
    }
  }
};

