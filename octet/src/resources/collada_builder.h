////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// load a COLLADA file.
//
// This class uses "tiny xml" a handy little XML reader
//
// Do not read this until you have a good understanding of C++ coding, it will melt your mind.
// It is, however, one of the smallest COLLADA readers in the Universe of its kind.
//
// COLLADA is notoriously complex, with large amounts of redundancy that needs to be skipped.
// On the surface it looks like a GL model, but adds many unresolvable abstractions.
// A new standard glTF looks more tractable with a more GL-like model. Unfortunately,
// there are very few direct exporters as yet.

// mesh builder class for standard meshes.
class collada_builder {
public:

private:
  TiXmlDocument doc;
  string doc_path;
  dictionary<TiXmlElement *, allocator> ids;
  dynarray<float> temp_floats;

  // find all the ids in an xml file
  void find_ids(TiXmlElement *parent) {
    for (TiXmlElement *elem = parent->FirstChildElement(); elem; elem = elem->NextSiblingElement()) {
      const char *attrib = elem->Attribute("id");
      if (attrib) {
        //printf("%s %s\n", elem->Value(), attrib);
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

  TiXmlElement *child(TiXmlElement *parent, const char *value) {
    return parent ? parent->FirstChildElement(value) : NULL;
  }

  TiXmlElement *sibling(TiXmlElement *element, const char *value) {
    return element ? element->NextSiblingElement(value) : NULL;
  }

  const char *attr(TiXmlElement *parent, const char *value) {
    return parent ? parent->Attribute(value) : NULL;
  }

  const char *text(TiXmlElement *parent) {
    return parent ? parent->GetText() : NULL;
  }

  const char *value(TiXmlElement *parent) {
    return parent ? parent->Value() : NULL;
  }

  int semantic_to_attr(const char *semantic, const char *set) {
    struct nameToValue { const char *name; int value; };
    static const nameToValue n2v[] = {
      { "POSITION", 0},
      { "WEIGHT", 1},
      { "BLENDWEIGHT", 1},
      { "NORMAL", 2},
      { "DIFFUSE", 3},
      { "COLOR", 3},
      { "SPECULAR", 4},
      { "TESSFACTOR", 5},
      { "FOGCOORD", 5},
      { "PSIZE", 6},
      { "JOINT", 7},
      { "BLENDINDICES", 7},
      { "TEXCOORD", 8},
      { "TANGENT", 14},
      { "BINORMAL", 15},
    };
    int int_set = set ? atoi(set) : 0;
    for (int i = 0; i != sizeof(n2v)/sizeof(n2v[0]); ++i) {
      if (!strcmp(semantic, n2v[i].name)) {
        return n2v[i].value + int_set;
      }
    }
    return 8;
  }

  // convert a string like "1.2 3.4 43.12" into an array of float values
  void atofv(dynarray<float> &values, const char *src) {  
    values.resize(0);
    if (!src) return;

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

  // convert an ascii sequence of integers like "1 3 9 12 34" to an array of integers
  void atoiv(dynarray<int> &values, const char *src) {  
    values.resize(0);
    while (*src > 0 && *src <= ' ') ++src;
    while(*src != 0) {
      int whole = 0, msign = 1;
      if (*src == '-') { msign = -1; src++; }
      while (*src >= '0' && *src <= '9') whole = whole * 10 + (*src++ - '0');
      values.push_back(whole * msign);
      while (*src > 0 && *src <= ' ') ++src;
    }
  }

  // convert an ascii sequence of integers like "fred bert harry" into an array of strings
  void atonv(dynarray<string> &values, const char *src) {
    values.resize(0);
    while (*src != 0 && *src <= ' ') ++src;
    while(*src != 0) {
      int index = (int)values.size();
      values.resize(index + 1);
      char tmp[128];
      int i = 0;
      while (*src != 0 && *src != ' ') {
        if (i < sizeof(tmp)-1) tmp[i++] = *src;
        src++;
      }
      tmp[i] = 0;
      values[index] = tmp;
      while (*src != 0 && *src <= ' ') ++src;
    }
  }

  // structure used when building a skin
  struct skin_state {
    // collada-style skin state
    dynarray<int> vcount;              // from skin vcount
    dynarray<float> raw_weights;       // from WEIGHT semantic - one per vertex
    dynarray<int> raw_indices;         // from JOINT semantic - one per vertex - must match INV_BIND_MATRIX
    dynarray<float> inv_bind_matrices; // from INV_BIND_MATRIX semantic
    dynarray<float> bind_shape_matrix; // from BIND_SHAPE_MATRIX element
    string joints;                     // from JOINT semantic - sids of affected nodes

    // OpenGL-style skin state
    enum { max_indices = 4 };
    dynarray<float> gl_weights;
    dynarray<int> gl_indices;
  };

  // a structure to keep track of the complex COLLADA <input> tags
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
    unsigned vertex_input_offset;
    int pass;
    skin_state *skinst;
  };

  // parse and <input> tag
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

    TiXmlElement *input2 = source_elem->FirstChildElement("input");
    if (input2) {
      // recursive <input> tag:; includes other inputs
      for (;input2 != 0; input2 = input2->NextSiblingElement("input")) {
        parse_input(state, input2);
      }
      return;
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
    const char *accessor_offset = accessor->Attribute("offset");
    const char *accessor_stride = accessor->Attribute("stride");
    int accessor_offset_int = accessor_offset ? atoi(accessor_offset) : 0;
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
      const char *param_name = param->Attribute("name");

      if (param_name) {
        param_type = param->Attribute("type");
        size++;
      } else {
        accessor_offset_int++;
      }
    }

    if (!param_type) {
      printf("warning: no param type\n");
      return;
    }

    int type = 0;
    if (!strcmp(param_type, "float")) {
      type = 1;
    } else if (!strcmp(param_type, "float4x4")) {
      type = 2;
    } else if (!strcmp(param_type, "name")) {
      type = 3;
    } else {
      printf("warning: unsupported type\n");
      return;
    }

    unsigned p_size = state.p.size();
    unsigned num_vertices = p_size / state.input_stride;

    if (state.pass == 1) {
      // attribute metrics pass
      unsigned attr = semantic_to_attr(semantic, set);
      state.s->add_attribute(attr, size, GL_FLOAT, state.attr_offset * 4);
      state.attr_offset += size;
    } else if (state.pass == 2) {
      dynarray<float> accessor_floats;
      if (!strcmp(accessor_source_elem->Value(), "float_array")) {
        atofv(accessor_floats, accessor_source_elem->GetText());
      }

      // attribute building pass
      for (unsigned i = 0; i != num_vertices; ++i) {
        unsigned index = state.p[i * state.input_stride + state.input_offset];
        //printf("%d ", index);
        for (unsigned j = 0; j != size; ++j) {
          //printf("i=%d j=%d idx=%d\n", i, j, index);
          unsigned dest_idx = i * state.attr_stride + state.attr_offset + j;
          unsigned src_idx = accessor_offset_int + index * accessor_stride_int + j;
          if (dest_idx >= state.vertices.size()) {
            printf("dest_idx >= state.vertices.size()\n");
            return;
          }

          if (type == 1) {
            if (src_idx >= accessor_floats.size()) {
              printf("src_idx >= accessor_floats.size()\n");
              return;
            }
            state.vertices[dest_idx] = accessor_floats[src_idx];
          } else {
            state.vertices[dest_idx] = (float)src_idx;
          }
        }
      }
      state.slot++;
      state.attr_offset += size;
    } else if (state.pass == 3) {
      // skin pass
      if (!strcmp(semantic, "JOINT")) {
        for (unsigned i = 0; i != num_vertices; ++i) {
          unsigned index = state.p[i * state.input_stride + state.input_offset];
          unsigned src_idx = accessor_offset_int + index * accessor_stride_int;
          state.skinst->raw_indices[i] = src_idx;
        }
      } else if (!strcmp(semantic, "WEIGHT")) {
        dynarray<float> accessor_floats;
        atofv(accessor_floats, accessor_source_elem->GetText());
        assert(state.skinst->raw_weights.size() >= num_vertices);
        for (unsigned i = 0; i != num_vertices; ++i) {
          unsigned index = state.p[i * state.input_stride + state.input_offset];
          unsigned src_idx = accessor_offset_int + index * accessor_stride_int;
          state.skinst->raw_weights[i] = accessor_floats[src_idx];
        }
      }
    }
  }

  TiXmlElement *find_param(TiXmlElement *profile_COMMON, const char *sid, const char *child_name) {
    if (!sid) return NULL;

    for (
      TiXmlElement *new_param = profile_COMMON->FirstChildElement("newparam");
      new_param; new_param = new_param->NextSiblingElement("newparam")
    ) {
      const char *sid_param = new_param->Attribute("sid");
      if (sid_param && !strcmp(sid_param, sid)) {
        return new_param->FirstChildElement(child_name);
      }
    }
    return NULL;
  }

  GLuint get_texture(TiXmlElement *shader, TiXmlElement *profile_COMMON, const char *value, const char *deflt) {
    TiXmlElement *section = child(shader, value);
    TiXmlElement *color = child(section, "color");
    TiXmlElement *texture = child(section, "texture");
    if (color) {
      atofv(temp_floats, color->GetText());
      if (temp_floats.size() == 3) {
        temp_floats.push_back(1);
      }
      if (temp_floats.size() >= 4) {
        char name[16];
        sprintf(
          name, "#%02x%02x%02x%02x",
          (int)(temp_floats[0]*255.0f+0.5f), (int)(temp_floats[1]*255.0f+0.5f),
          (int)(temp_floats[2]*255.0f+0.5f), (int)(temp_floats[3]*255.0f+0.5f)
        );
        return resources::get_texture_handle(GL_RGBA, name);
      }
    } else if (texture) {
      // todo: handle multiple texcoords
      const char *texture_name = attr(texture, "texture");
      TiXmlElement *sampler2D = find_param(profile_COMMON, texture_name, "sampler2D");
      TiXmlElement *source = child(sampler2D, "source");
      const char *surface_name = text(source);
      TiXmlElement *surface = find_param(profile_COMMON, surface_name, "surface");
      TiXmlElement *init_from = child(surface, "init_from");
      const char *image_name = text(init_from);
      TiXmlElement *image = find_id(image_name);
      const char *url_attr = text(child(image, "init_from"));
      if (url_attr) {
        string new_path = doc_path;
        string url = url_attr;
        int extension_pos = url.extension_pos();
        if (extension_pos != -1) {
          // at present we only accept gifs. Use image alchemy to convert files.
          url.truncate(extension_pos);
          url += ".gif";
        }
        new_path += url;
        return resources::get_texture_handle(GL_RGBA, new_path);
      }
    }
    return resources::get_texture_handle(GL_RGBA, deflt);
  }

  float get_float(TiXmlElement *shader, const char *value) {
    TiXmlElement *section = shader->FirstChildElement(value);
    TiXmlElement *child = section ? section->FirstChildElement("float") : 0;
    if (child) {
      atofv(temp_floats, child->GetText());
      if (temp_floats.size() >= 1) {
        return temp_floats[0];
      }
    }
    return 0;
  }

  void add_materials(resources &dict) {
    TiXmlElement *lib_mat = doc.RootElement()->FirstChildElement("library_materials");

    if (!dict.has_resource("default_material")) {
      bump_material *defmat = new bump_material();
      defmat->make_color(vec4(0.5, 0.5, 0.5, 1), false, false);
      dict.set_resource("default_material", defmat);
    }

    if (!lib_mat) return;

    for (TiXmlElement *material = lib_mat->FirstChildElement(); material != NULL; material = material->NextSiblingElement()) {
      TiXmlElement *ieffect = material->FirstChildElement("instance_effect");
      const char *url = ieffect ? ieffect->Attribute("url") : 0;
      TiXmlElement *effect = find_id(url);
      TiXmlElement *profile_COMMON = effect ? effect->FirstChildElement("profile_COMMON") : 0;
      TiXmlElement *technique = profile_COMMON ? profile_COMMON->FirstChildElement("technique") : 0;
      TiXmlElement *phong = technique ? technique->FirstChildElement("phong") : 0;
      TiXmlElement *shader = phong ? phong : technique ? technique->FirstChildElement("lambert") : 0;
      if (shader) {
        url += url[0] == '#';
        GLuint emission = get_texture(shader, profile_COMMON, "emission", "#808080ff");
        GLuint ambient = get_texture(shader, profile_COMMON, "ambient", "#808080ff");
        GLuint diffuse = get_texture(shader, profile_COMMON, "diffuse", "#808080ff");
        GLuint specular = get_texture(shader, profile_COMMON, "specular", "#808080ff");
        GLuint bump = get_texture(shader, profile_COMMON, "bump", "#808080ff");
        float shininess = get_float(shader, "shininess");
        bump_material *mat = new bump_material();
        mat->init(diffuse, ambient, emission, specular, bump, shininess);
        dict.set_resource(attr(material, "id"), mat);
      } else {
        bump_material *mat = new bump_material();
        mat->make_color(vec4(0.5, 0.5, 0.5, 0), false, false);
        dict.set_resource(attr(material, "id"), mat);
      }
    }
  }

  void add_mesh_instances(TiXmlElement *technique_common, const char *url, scene_node *node, skeleton *skel, skin *skn, resources &dict, scene &s) {
    if (!url) return;

    url += url[0] == '#';

    if (technique_common) {
      for (
        TiXmlElement *instance = technique_common->FirstChildElement("instance_material");
        instance != NULL;
        instance = instance->NextSiblingElement("instance_material")
      ) {
        const char *symbol = instance->Attribute("symbol");
        const char *target = instance->Attribute("target");
        if (target && target[0] == '#') target++;
        // add a scene_node/mesh/material combination to the mesh_instance
        if (symbol && url) {
          string new_url;
          new_url.format("%s+%s", new_url, symbol);
          mesh_state *mesh = dict.get_mesh_state(new_url);
          bump_material *mat = dict.get_bump_material(target);
          if (!mat) mat = dict.get_bump_material("default_material");
          if (mesh) {
            mesh_instance *mi = new mesh_instance(node, mesh, mat, skn, skel);
            s.add_mesh_instance(mi);
          }
        }
      }
    } else {
      mesh_state *mesh = dict.get_mesh_state(url);
      bump_material *mat = dict.get_bump_material("default_material");
      if (mesh) {
        mesh_instance *mi = new mesh_instance(node, mesh, mat, skn, skel);
        s.add_mesh_instance(mi);
      }
    }
  }

  // add an <instance_geometry> mesh instance
  void add_instance_geometry(TiXmlElement *element, scene_node *node, resources &dict, scene &s) {
    const char *url = element->Attribute("url");
    url += url[0] == '#';
    TiXmlElement *bind_material = child(element, "bind_material");
    TiXmlElement *technique_common = child(bind_material, "technique_common");

    add_mesh_instances(technique_common, url, node, 0, 0, dict, s);
  }

  // add an <instance_controller> skin instance
  void add_instance_controller(TiXmlElement *element, scene_node *node, resources &dict, scene &s) {
    const char *controller_url = attr(element, "url");
    TiXmlElement *bind_material = child(element, "bind_material");
    TiXmlElement *technique_common = child(bind_material, "technique_common");

    int num_bones = 0;
    for (TiXmlElement *skel_elem = child(element, "skeleton"); skel_elem; skel_elem = sibling(skel_elem, "skeleton")) {
      num_bones++;
    }

    mesh_state *mesh = dict.get_mesh_state(controller_url);
    if (!mesh || !mesh->get_skin()) return;
    skin *skn = mesh->get_skin();

    skeleton *skel = new skeleton();
    TiXmlElement *skel_elem = child(element, "skeleton");
    //int num_nodes = s.get_num_nodes();
    dictionary<int> skin_joints;
    while (skel_elem) {
      const char *skeleton_id = text(skel_elem);
      TiXmlElement *node_elem = find_id(skeleton_id);
      scene_node *node = (scene_node*)node_elem->GetUserData();
      if (node) {
        dynarray<scene_node*> nodes;
        dynarray<int> parents;
        node->get_all_child_nodes(nodes, parents);
        for (int i = 0; i != nodes.size(); ++i) {
          scene_node *node = nodes[i];
          skel->add_bone(node->get_nodeToParent(), node->get_sid(), parents[i]);
        }
      }
      skel_elem = sibling(skel_elem, "skeleton");
    }

    //const char *url = skin->Attribute("source");
    add_mesh_instances(technique_common, controller_url, node, skel, skn, dict, s);
  }

  float quick_float(TiXmlElement *parent, const char *name) {
    TiXmlElement *child = parent->FirstChildElement(name);
    return child ? (float)atof(child->GetText()) : 0;
  }

  // add a camera to the scene
  void add_instance_camera(TiXmlElement *elem, scene_node *node, resources &dict, scene &s) {
    const char *url = elem->Attribute("url");
    TiXmlElement *cam = find_id(url);
    if (!cam) return;

    TiXmlElement *optics = child(cam, "optics");
    TiXmlElement *technique_common = child(optics, "technique_common");
    TiXmlElement *perspective = child(technique_common, "perspective");
    TiXmlElement *ortho = child(technique_common, "ortho");
    TiXmlElement *params = perspective ? perspective : ortho;
    if (params) {
      float n = quick_float(params, "znear");
      float f = quick_float(params, "zfar");
      float aspect_ratio = quick_float(params, "aspect_ratio");
      camera_instance *c = new camera_instance();
      s.add_camera_instance(c);
      if (perspective) {
        float xfov = quick_float(params, "xfov");
        float yfov = quick_float(params, "yfov");
        c->set_perspective(node, xfov, yfov, aspect_ratio, n, f);
      } else {
        float xmag = quick_float(params, "xmag");
        float ymag = quick_float(params, "ymag");
        c->set_ortho(node, xmag, ymag, aspect_ratio, n, f);
      }
    }
  }

  // add a light to the scene
  void add_instance_light(TiXmlElement *elem, scene_node *node, resources &dict, scene &s) {
    const char *url = elem->Attribute("url");
    TiXmlElement *light = find_id(url);
    if (!light) return;
  }

  // add a geometry element to the list of mesh states
  void add_geometry(resources &dict) {
    TiXmlElement *lib_geom = doc.RootElement()->FirstChildElement("library_geometries");
    for (TiXmlElement *geometry = lib_geom->FirstChildElement(); geometry != NULL; geometry = geometry->NextSiblingElement()) {
      TiXmlElement *mesh = geometry->FirstChildElement("mesh");
      const char *id = geometry->Attribute("id");

      for (TiXmlElement *mesh_child = mesh ? mesh->FirstChildElement() : 0;
        mesh_child != NULL;
        mesh_child = mesh_child->NextSiblingElement()
      ) {
        if (is_mesh_component(mesh_child->Value())) {
          mesh_state *mesh = new mesh_state();
          get_mesh_component(mesh, id, mesh_child, NULL, dict);
        }
      }
    }
  }

  // add a geometry element to the list of mesh states
  void add_controllers(resources &dict) {
    TiXmlElement *lib_ctrl = doc.RootElement()->FirstChildElement("library_controllers");
    for (TiXmlElement *controller = lib_ctrl->FirstChildElement(); controller != NULL; controller = controller->NextSiblingElement()) {
      TiXmlElement *skin_elem = controller->FirstChildElement("skin");
      const char *controller_id = controller->Attribute("id");
      TiXmlElement *geometry = find_id(attr(skin_elem, "source"));
      TiXmlElement *bind_shape_matrix = child(skin_elem, "bind_shape_matrix");
      TiXmlElement *joints_elem = child(skin_elem, "joints");
      skin_state skinst;

      if (bind_shape_matrix) {
        atofv(skinst.bind_shape_matrix, text(bind_shape_matrix));
      }

      if (joints_elem) {
        TiXmlElement *input = child(joints_elem, "input");
        while (input) {
          const char *semantic = attr(input, "semantic");
          const char *source_id = attr(input, "source");
          if (!strcmp(semantic, "JOINT")) {
            TiXmlElement *name_array = child(find_id(source_id), "Name_array");
            if (name_array) {
              skinst.joints = text(name_array);
            }
          } else if (!strcmp(semantic, "INV_BIND_MATRIX")) {
            TiXmlElement *float_array = child(find_id(source_id), "float_array");
            atofv(skinst.inv_bind_matrices, text(float_array));
          }
          input = sibling(input, "input");
        }
      }

      TiXmlElement *vertex_weights = skin_elem ? skin_elem->FirstChildElement("vertex_weights") : 0;
      if (vertex_weights && geometry) {
        get_skin(controller, vertex_weights, &skinst);
        TiXmlElement *mesh = geometry->FirstChildElement("mesh");
        const char *id = geometry->Attribute("id");

        for (TiXmlElement *mesh_child = mesh ? mesh->FirstChildElement() : 0;
          mesh_child != NULL;
          mesh_child = mesh_child->NextSiblingElement()
        ) {
          if (is_mesh_component(mesh_child->Value())) {
            mat4t modelToBind;

            if (skinst.bind_shape_matrix.size() >= 16) {
              modelToBind.init_row_major(&skinst.bind_shape_matrix[0]);
            } else {
              modelToBind.loadIdentity();
            }

            skin *mesh_skin = new skin(modelToBind);
            dynarray<string> joints;
            atonv(joints, skinst.joints);

            //mesh_skin->bindToModel.resize(skinst.inv_bind_matrices.size()/16);

            for (unsigned i = 0; i != joints.size(); ++i) {
              mat4t bindToModel;
              bindToModel.init_row_major(&skinst.inv_bind_matrices[i*16]);
              mesh_skin->add_joint(bindToModel, resources::get_atom(joints[i]));
            }

            mesh_state *mesh = new mesh_state(mesh_skin);
            get_mesh_component(mesh, controller_id, mesh_child, &skinst, dict);
          }
        }
      }
    }
  }

  // add <library_animations> to the scene
  void add_animations(resources &dict) {
    TiXmlElement *lib_anim = doc.RootElement()->FirstChildElement("library_animations");
    for (TiXmlElement *anim_elem = child(lib_anim, "animation"); anim_elem != NULL; anim_elem = sibling(anim_elem, "animation")) {
      animation *anim = new animation();
      dict.set_resource(attr(anim_elem, "id"), anim);
      for (TiXmlElement *channel_elem = child(anim_elem, "channel"); channel_elem != NULL; channel_elem = sibling(channel_elem, "channel")) {
        string target = attr(channel_elem, "target");
        TiXmlElement *sampler_elem = find_id(attr(channel_elem, "source"));
        if (sampler_elem) {
          dynarray<float> times;
          dynarray<float> transforms;
          dynarray<string> interpolation;

          TiXmlElement *input = child(sampler_elem, "input");
          while (input) {
            const char *semantic = attr(input, "semantic");
            const char *source_id = attr(input, "source");
            if (!strcmp(semantic, "INPUT")) {
              TiXmlElement *float_array = child(find_id(source_id), "float_array");
              atofv(times, text(float_array));
            } else if (!strcmp(semantic, "OUTPUT")) {
              TiXmlElement *float_array = child(find_id(source_id), "float_array");
              atofv(transforms, text(float_array));
            } else if (!strcmp(semantic, "INTERPOLATION")) {
              TiXmlElement *name_array = child(find_id(source_id), "Name_array");
              if (name_array) {
                atonv(interpolation, text(name_array));
              }
            }
            input = sibling(input, "input");
          }

          if (times.size() && transforms.size() == times.size()*16 && interpolation.size() == times.size()) {
            int num_times = (int)times.size();
            anim->add_channel_from_matrices(num_times, &times[0], &transforms[0]);
          }
        }
      }
    }
  }

  // build the scene_node heirachy
  void build_heirachy(dynarray<TiXmlElement *> &node_elems, dynarray<scene_node *> &nodes, TiXmlElement *scene_element, resources &dict, scene &s) {
    // create a stack to avoid recursion (a bad thing in games)
    dynarray<TiXmlElement *> stack;
    dynarray<scene_node *> node_stack;
    stack.reserve(64);
    node_stack.reserve(64);

    node_stack.push_back(s.get_root_node());
    stack.push_back(scene_element);
    while (!stack.is_empty()) {
      TiXmlElement *parent_elem = stack.back();
      scene_node *parent = node_stack.back();
      stack.pop_back();
      node_stack.pop_back();
      TiXmlElement *node_elem = child(parent_elem, "node");
      while (node_elem) {
        mat4t nodeToParent;
        nodeToParent.loadIdentity();
        scene_node *new_node = new scene_node(nodeToParent, resources::get_atom(attr(node_elem, "sid")));
        dict.set_resource(attr(node_elem, "id"), new_node);
        parent->add_child(new_node);
        stack.push_back(node_elem);
        node_stack.push_back(new_node);
        nodes.push_back(new_node);
        node_elems.push_back(node_elem);
        node_elem->SetUserData(new_node);
        node_elem = sibling(node_elem, "node");
      }
    }
  }

  // add matrices and instances
  void build_matrices(dynarray<TiXmlElement *> &node_elems, dynarray<scene_node *> &nodes, resources &dict, scene &s) {
    for (int ni = 0; ni != node_elems.size(); ++ni) {
      TiXmlElement *node_elem = node_elems[ni];
      scene_node *node = nodes[ni];
      mat4t &matrix = node->get_nodeToParent();
      matrix.loadIdentity();

      for (TiXmlElement *child = node_elem->FirstChildElement(); child != NULL; child = child->NextSiblingElement()) {
        const char *value = child->Value();
        if (!strcmp(value, "matrix")) {
          atofv(temp_floats, child->GetText());
          if (temp_floats.size() >= 16) {
            mat4t tmp(
              vec4(temp_floats[0], temp_floats[4], temp_floats[8], temp_floats[12]),
              vec4(temp_floats[1], temp_floats[5], temp_floats[9], temp_floats[13]),
              vec4(temp_floats[2], temp_floats[6], temp_floats[10], temp_floats[14]),
              vec4(temp_floats[3], temp_floats[7], temp_floats[11], temp_floats[15])
            );
            matrix.multMatrix(tmp);
          }
        } else if (!strcmp(value, "rotate")) {
          atofv(temp_floats, child->GetText());
          if (temp_floats.size() >= 4) {
            // not a precise definition of collada rotate, but works in most cases
            if (temp_floats[0] == 1) matrix.rotateX(temp_floats[3]);
            else if (temp_floats[1] == 1) matrix.rotateY(temp_floats[3]);
            else if (temp_floats[2] == 1) matrix.rotateZ(temp_floats[3]);
            else printf("strange file\n");
          }
        } else if (!strcmp(value, "scale")) {
          atofv(temp_floats, child->GetText());
          if (temp_floats.size() >= 3) {
            matrix.scale(temp_floats[0], temp_floats[1], temp_floats[2]);
          }
        } else if (!strcmp(value, "translate")) {
          atofv(temp_floats, child->GetText());
          if (temp_floats.size() >= 3) {
            matrix.translate(temp_floats[0], temp_floats[1], temp_floats[2]);
          }
        }
      }
    }
  }

  // add instances
  void build_instances(dynarray<TiXmlElement *> &node_elems, dynarray<scene_node *> &nodes, resources &dict, scene &s) {
    for (int ni = 0; ni != node_elems.size(); ++ni) {
      TiXmlElement *node_elem = node_elems[ni];
      scene_node *node = nodes[ni];

      for (TiXmlElement *child = node_elem->FirstChildElement(); child != NULL; child = child->NextSiblingElement()) {
        const char *value = child->Value();
        if (!strcmp(value, "instance_geometry")) {
          add_instance_geometry(child, node, dict, s);
        } else if (!strcmp(value, "instance_controller")) {
          add_instance_controller(child, node, dict, s);
        } else if (!strcmp(value, "instance_camera")) {
          add_instance_camera(child, node, dict, s);
        } else if (!strcmp(value, "instance_light")) {
          add_instance_light(child, node, dict, s);
        }
      }
    }
  }

  // if we have a vcount element (polylist), we build polygons out of triangles
  // and hope they are convex!
  unsigned convert_polygons_to_triangles(parse_input_state &state, dynarray<int> &vcount) {
    unsigned num_indices = 0;
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
    return num_indices;
  }

  // find the maximum input offset and infer the input stride (this is not explicit in the spec)
  int get_input_stride(TiXmlElement *mesh_child) {
    int input_stride = 1;
    int implicit_offset = 0;
    for (TiXmlElement *input_elem = mesh_child->FirstChildElement("input");
      input_elem != NULL;
      input_elem = input_elem->NextSiblingElement("input")
    ) {
      const char *offset = input_elem->Attribute("offset");
      int int_offset = offset ? atoi(offset) : implicit_offset++;
      if (int_offset+1 > input_stride) {
        input_stride = int_offset+1;
      }
    }
    return input_stride;
  }

  // get triangles from a trilist or polylist
  void get_mesh_component(mesh_state *mesh, const char *id, TiXmlElement *mesh_child, skin_state *skinst, resources &dict) {
    TiXmlElement *pelem = child(mesh_child, "p");

    if (!pelem) {
      printf("warning: no <p>\n");
      return;
    }

    const char *symbol = attr(mesh_child, "symbol");
    if (symbol) {
      string new_url;
      new_url.format("%s+%s", id, symbol);
      dict.set_resource(new_url, mesh);
    } else {
      printf("add mesh resource %s\n", id);
      dict.set_resource(id, mesh);
    }

    parse_input_state state;
    state.s = mesh;
    atoiv(state.p, pelem->GetText());
    state.input_stride = get_input_stride(mesh_child);
    unsigned implicit_offset = 0;
    state.slot = 0;
    state.attr_offset = 0;
    state.skinst = skinst;

    unsigned p_size = state.p.size();
    if (p_size % state.input_stride != 0) {
      printf("warning: expected multiple of %d indices\n", state.input_stride);
      return;
    }

    unsigned num_vertices = p_size / state.input_stride;

    // find the output size
    for (TiXmlElement *input = mesh_child->FirstChildElement("input");
      input != NULL;
      input = input->NextSiblingElement("input")
    ) {
      const char *offset = input->Attribute("offset");
      state.input_offset = offset ? atoi(offset) : 0;
      state.pass = 1;
      parse_input(state, input);
    }

    int blendweight_offset = 0;
    int blendindices_offset = 0;
    int blendweight_stride = 3;
    int blendindices_stride = 4;

    if (skinst) {
      // skins need extra parameters for indices and weights
      // add extra attributes for blending
      // todo: use only max(vcount) indices
      state.s->add_attribute(attribute_blendweight, blendweight_stride, GL_FLOAT, state.attr_offset * 4);
      blendweight_offset = state.attr_offset;
      state.attr_offset += blendweight_stride;
      state.s->add_attribute(attribute_blendindices, blendindices_stride, GL_FLOAT, state.attr_offset * 4);
      blendindices_offset = state.attr_offset;
      state.attr_offset += blendindices_stride;
    }

    state.attr_stride = state.attr_offset;
    state.vertices.resize(state.attr_stride * num_vertices);
    state.input_offset = 0;
    state.attr_offset = 0;
    state.vertex_input_offset = 0;

    // build the attributes
    for (TiXmlElement *input = mesh_child->FirstChildElement("input");
      input != NULL;
      input = input->NextSiblingElement("input")
    ) {
      const char *offset = input->Attribute("offset");
      state.input_offset = offset ? atoi(offset) : 0;
      state.pass = 2;
      parse_input(state, input);
      if (!strcmp(attr(input, "semantic"), "VERTEX")) {
        state.vertex_input_offset = state.input_offset;
      }
    }

    if (skinst) {
      // skins need extra parameters for indices and weights
      // copy the processed blend vertices to the gl attributes using indices from the <p> array
      unsigned p_size = state.p.size();
      unsigned num_vertices = p_size / state.input_stride;
      for (unsigned i = 0; i != num_vertices; ++i) {
        unsigned index = state.p[i * state.input_stride + state.vertex_input_offset];
        if (0) {
          app_utils::log("i%d\n", index);
        }
        for (int j = 0; j != blendindices_stride; ++j) {
          state.vertices[state.attr_stride * i + blendindices_offset + j] = (float)state.skinst->gl_indices[index * blendindices_stride + j];
        }
        for (int j = 0; j != blendweight_stride; ++j) {
          state.vertices[state.attr_stride * i + blendweight_offset + j] = state.skinst->gl_weights[index * blendweight_stride + j];
        }
      }
    }

    TiXmlElement *vcount_elem = mesh_child->FirstChildElement("vcount");

    // build an initial index based on the mesh_child value
    // todo: optimise the mesh.
    unsigned num_indices = 0;
    if (vcount_elem) {
      // polygons
      dynarray<int> vcount;
      atoiv(vcount, vcount_elem->GetText());
      num_indices = convert_polygons_to_triangles(state, vcount);
    } else {
      // just plain triangles
      state.indices.resize(num_vertices);
      num_indices = num_vertices;
      for (unsigned i = 0; i != num_vertices; ++i) {
        state.indices[i] = i;
      }
    }

    unsigned isize = state.indices.size() * sizeof(state.indices[0]);
    unsigned vsize = state.vertices.size() * sizeof(state.vertices[0]);

    mesh->allocate(vsize, isize, app_common::can_use_vbos());
    mesh->assign(vsize, isize, (unsigned char*)&state.vertices[0], (unsigned char*)&state.indices[0]);
    mesh->set_params(state.attr_stride * 4, num_indices, num_vertices, GL_TRIANGLES, GL_UNSIGNED_SHORT);
    if (0) {
      FILE *file = app_utils::log("mesh skinst=%p\n", skinst);
      mesh->dump(file);
      fflush(file);
    }
  }

  // get blend weights and matrices from a skin
  // after this we are still not home yet as the weights need to be indexed by the POSITION of the skinned mesh.
  void get_skin(TiXmlElement *geometry, TiXmlElement *mesh_child, skin_state *skin) {
    TiXmlElement *pelem = child(mesh_child, "v");

    if (!pelem) {
      printf("warning: no <v>\n");
      return;
    }

    TiXmlElement *vcount_elem = mesh_child->FirstChildElement("vcount");
    if (!vcount_elem) {
      printf("warning: no vcount element in skin\n");
    }

    atoiv(skin->vcount, vcount_elem->GetText());

    int num_vertices = 0;
    int num_vcs = skin->vcount.size();
    for (int i = 0; i != num_vcs; ++i) {
      num_vertices += skin->vcount[i];
    }

    skin->raw_indices.resize(num_vertices);
    skin->raw_weights.resize(num_vertices);

    parse_input_state state;
    state.s = NULL;
    atoiv(state.p, pelem->GetText());
    state.input_stride = get_input_stride(mesh_child);
    state.slot = 0;
    state.attr_offset = 0;
    state.skinst = skin;
    state.input_offset = 0;

    // build the raw skin paramerters
    for (TiXmlElement *input = mesh_child->FirstChildElement("input");
      input != NULL;
      input = input->NextSiblingElement("input")
    ) {
      const char *offset = input->Attribute("offset");
      state.input_offset = offset ? atoi(offset) : 0;
      state.pass = 3;
      parse_input(state, input);
    }

    // convert raw params into gl params (max 4 weights)
    int start = 0;
    int indices_stride = skin_state::max_indices;
    int weights_stride = indices_stride - 1;
    skin->gl_indices.resize(num_vcs * indices_stride);
    skin->gl_weights.resize(num_vcs * weights_stride);
    for (int i = 0; i != num_vcs; ++i) {
      int vc = skin->vcount[i];
      // make exactly max_indices weights and indices for every vertex
      // todo: get the four largest in cases where vc > max_indices
      // note that the first weight is expected to be 1 - (other weights)
      for (int j = 0; j != indices_stride; ++j) {
        int raw_index = j < vc ? skin->raw_indices[start + j] : 0;
        skin->gl_indices[i * indices_stride + j] = raw_index;
      }
      for (int j = 0; j != weights_stride; ++j) {
        float raw_weight = j + 1 < vc ? skin->raw_weights[start + j + 1] : 0;
        skin->gl_weights[i * weights_stride + j] = raw_weight;
      }
      start += vc;
    }
    if (0) {
      FILE *f = app_utils::log("raw weights & indices\n");
      for (int i = 0; i != skin->raw_indices.size(); ++i) {
        fprintf(f, "ri %d %d\n", i, skin->raw_indices[i]);
      }
      for (int i = 0; i != skin->raw_weights.size(); ++i) {
        fprintf(f, "rw %d %f\n", i, skin->raw_weights[i]);
      }
      for (int i = 0; i != skin->gl_indices.size(); ++i) {
        fprintf(f, "i %d %d\n", i, skin->gl_indices[i]);
      }
      for (int i = 0; i != skin->gl_weights.size(); ++i) {
        fprintf(f, "w %d %f\n", i, skin->gl_weights[i]);
      }
    }
  }

  // does this thing have triangles in it?
  bool is_mesh_component(const char *value) {
    return (
      !strcmp(value, "triangles") ||
      !strcmp(value, "polylist")
    );
  }

  void add_scenes(resources  &dict) {
    TiXmlElement *lib = doc.RootElement()->FirstChildElement("library_visual_scenes");

    if (!lib) return;

    for (TiXmlElement *elem = lib->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement()) {
      dynarray<TiXmlElement *> node_elems;
      dynarray<scene_node *> nodes;
      scene *scn = new scene();
      dict.set_resource(attr(elem, "id"), scn);
      build_heirachy(node_elems, nodes, elem, dict, *scn);
      build_matrices(node_elems, nodes, dict, *scn);
      build_instances(node_elems, nodes, dict, *scn);
    }

  }

public:
  collada_builder() {
  }

  // public function to load a collada file
  void load(const char *url) {
    doc_path = url;
    doc_path.truncate(doc_path.filename_pos());
    doc.LoadFile(app_utils::get_path(url));
    TiXmlElement *top = doc.RootElement();
    if (!top || strcmp(top->Value(), "COLLADA")) {
      printf("warning: not a collada file");
      return;
    }
    find_ids(top);
  }

  // once loaded, use this to access the first component in the mesh
  void get_mesh_state(mesh_state &s, const char *id, resources &dict) {
    TiXmlElement *geometry = find_id(id);
    s.init();

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
      if (is_mesh_component(mesh_child->Value())) {
        get_mesh_component(&s, id, mesh_child, NULL, dict);
        return;
      }
    }
  }

  // get the url from the default visual scene
  const char *get_default_scene() {
    TiXmlElement *scene = doc.RootElement()->FirstChildElement("scene");
    TiXmlElement *ivs = scene ? scene->FirstChildElement("instance_visual_scene") : 0;
    return ivs ? ivs->Attribute("url") : 0;
  }

  // extract resources from the collada file into a collection.
  void get_resources(resources &dict) {
    add_materials(dict);

    add_geometry(dict);

    add_controllers(dict);

    add_animations(dict);

    add_scenes(dict);
  }
};

