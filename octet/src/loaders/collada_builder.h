////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
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
// A new standard, glTF looks more tractable with a more GL-like model. Unfortunately,
// there are very few direct exporters as yet.

// mesh builder class for standard meshes.
namespace octet { namespace loaders {
  /// Class for loading COLADA files.
  class collada_builder {
  public:

  private:
    // turn this on to debug the file as it loads
    // 0 = none, 1 = summary, 2 = details
    enum { debug = 0 };

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
      //values.resize(0);
      if (!src) return;

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
      if (!src) return;

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
      mesh *s;
      dynarray<int> p;
      dynarray<float> vertices;
      dynarray<unsigned> indices;
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

      TiXmlElement *input2 = child(source_elem, "input");
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

      TiXmlElement *tc = child(source_elem, "technique_common");
      if (!tc) {
        printf("warning: no technique_common\n");
        return;
      }

      TiXmlElement *accessor = child(tc, "accessor");
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
        TiXmlElement *param = child(accessor, "param");
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

    // effects use "newparam" tags to store samplers and textures
    TiXmlElement *find_param(TiXmlElement *profile_COMMON, const char *sid, const char *child_name) {
      if (!sid) return NULL;

      for (
        TiXmlElement *new_param = child(profile_COMMON, "newparam");
        new_param; new_param = new_param->NextSiblingElement("newparam")
      ) {
        const char *sid_param = new_param->Attribute("sid");
        if (sid_param && !strcmp(sid_param, sid)) {
          return new_param->FirstChildElement(child_name);
        }
      }
      return NULL;
    }

    // get a texture or a solid colour
    param *get_param(param_buffer_info &pbi, GLint &texture_slot, resource_dict &dict, TiXmlElement *shader, TiXmlElement *profile_COMMON, const char *value, const vec4 &deflt) {
      TiXmlElement *section = child(shader, value);
      TiXmlElement *color = child(section, "color");
      TiXmlElement *texture = child(section, "texture");
      if (color) {
        atofv(temp_floats, color->GetText());
        if (temp_floats.size() == 3) {
          temp_floats.push_back(1);
        }
        return new param_color(pbi, vec4(temp_floats[0], temp_floats[1], temp_floats[2], temp_floats[3]), app_utils::get_atom(value), param::stage_fragment);

        /*if (temp_floats.size() >= 4) {
          char name[16];
          sprintf(
            name, "#%02x%02x%02x%02x",
            (int)(temp_floats[0]*255.0f+0.5f), (int)(temp_floats[1]*255.0f+0.5f),
            (int)(temp_floats[2]*255.0f+0.5f), (int)(temp_floats[3]*255.0f+0.5f)
          );
          return resource_dict::get_texture_handle(GL_RGBA, name);
        }*/
      } else if (texture) {
        // todo: handle multiple texcoords
        const char *texture_name = attr(texture, "texture");
        TiXmlElement *sampler2D = find_param(profile_COMMON, texture_name, "sampler2D");
        TiXmlElement *source = child(sampler2D, "source");
        const char *surface_name = text(source);
        TiXmlElement *surface = find_param(profile_COMMON, surface_name, "surface");
        TiXmlElement *init_from = child(surface, "init_from");
        const char *image_name = text(init_from);
        image *img = dict.get_image(image_name);
        if (img) return new param_sampler(pbi, app_utils::get_atom(value), img, new sampler(), param::stage_fragment);
        /*TiXmlElement *image = find_id(image_name);
        const char *url_attr = text(child(image, "init_from"));
        if (url_attr) {
          string new_path;
          new_path.format("%s%s", doc_path.c_str(), url_attr);
          return resource_dict::get_texture_handle(GL_RGBA, new_path);
        }*/
      }
      //return resource_dict::get_texture_handle(GL_RGBA, deflt);
      return new param_color(pbi, deflt, app_utils::get_atom(value), param::stage_fragment);
    }

    // get a floating point number (or the default)
    param_color *get_float(param_buffer_info &pbi, TiXmlElement *shader, const char *value, float deflt) {
      TiXmlElement *section = child(shader, value);
      TiXmlElement *float_ = child(section, "float");
      if (float_) {
        atofv(temp_floats, float_->GetText());
        if (temp_floats.size() >= 1) {
          return new param_color(pbi, vec4(temp_floats[0], 0, 0, 0), app_utils::get_atom(value), param::stage_fragment);
        }
      }
      return new param_color(pbi, vec4(deflt, 0, 0, 0), app_utils::get_atom(value), param::stage_fragment);
    }

    // add all the materials from the collada file to the resources collection
    void add_materials(resource_dict &dict) {
      TiXmlElement *lib_mat = child(doc.RootElement(), "library_materials");

      if (!dict.has_resource("default_material")) {
        material *defmat = new material(vec4(0.5, 0.5, 0.5, 1));
        dict.set_resource("default_material", defmat);
      }

      if (!lib_mat) return;

      for (TiXmlElement *mat_elem = lib_mat->FirstChildElement(); mat_elem != NULL; mat_elem = mat_elem->NextSiblingElement()) {
        TiXmlElement *ieffect = child(mat_elem, "instance_effect");
        const char *url = attr(ieffect, "url");
        TiXmlElement *effect = find_id(url);
        TiXmlElement *profile_COMMON = child(effect, "profile_COMMON");
        TiXmlElement *technique = child(profile_COMMON, "technique");
        TiXmlElement *phong = child(technique, "phong");
        TiXmlElement *blinn = child(technique, "blinn");
        TiXmlElement *lambert = child(technique, "lambert");
        TiXmlElement *shader = phong ? phong : blinn ? blinn : lambert;
        dynarray<uint8_t> static_buffer(256);
        param_buffer_info pbi(static_buffer);
        GLint texture_slot = 0;
        if (shader) {
          url += url[0] == '#';
          param *emission = get_param(pbi, texture_slot, dict, shader, profile_COMMON, "emission", vec4(0, 0, 0, 0));
          param *ambient = get_param(pbi, texture_slot, dict, shader, profile_COMMON, "ambient", vec4(0, 0, 0, 1));
          param *diffuse = get_param(pbi, texture_slot, dict, shader, profile_COMMON, "diffuse", vec4(0.5f, 0.5f, 0.5f, 0));
          param *specular = get_param(pbi, texture_slot, dict, shader, profile_COMMON, "specular", vec4(0, 0, 0, 0));
          param *bump = get_param(pbi, texture_slot, dict, shader, profile_COMMON, "bump", vec4(0.5f, 0.5f, 1.0f, 0));
          param_color *shininess = get_float(pbi, shader, "shininess", 0);
          // this is not strictly correct, but fixes some issues
          //if (shininess->get_value(buffer.data()).x() >= 1) shininess->set_value(buffer.data(), vec4(shininess->get_value(buffer.data()) * 0.01f));
          material *mat = new material(diffuse, ambient, emission, specular, bump, shininess);
          //mat->init(diffuse, ambient, emission, specular, bump, shininess);
          dict.set_resource(attr(mat_elem, "id"), mat);
        } else {
          material *mat = new material(vec4(0.5, 0.5, 0.5, 0));
          dict.set_resource(attr(mat_elem, "id"), mat);
        }
      }
    }

    // add geometry and skins from the collada file to the resources collection
    void add_mesh_instances(TiXmlElement *technique_common, const char *url, scene_node *node, skeleton *skel, resource_dict &dict, visual_scene &s) {
      if (!url) return;

      TiXmlElement *instance = child(technique_common, "instance_material");
      if (instance) {
        for (; instance != NULL; instance = instance->NextSiblingElement("instance_material")) {
          const char *symbol = instance->Attribute("symbol");
          const char *target = instance->Attribute("target");
          material *mat = dict.get_material(target);
          if (!mat) mat = dict.get_material("default_material");
          const char *mesh_url = url;
          string new_url;

          // add a scene_node/mesh/material combination to the mesh_instance
          if (symbol) {
            new_url.format("%s+%s", url, symbol);
            mesh_url = new_url.c_str();
          }

          if (debug > 0 ) {
            log("add mesh instance %s\n", mesh_url);
          }

          mesh *msh = dict.get_mesh(mesh_url);
          if (msh) {
            mesh_instance *mi = new mesh_instance(node, msh, mat, skel);
            s.add_mesh_instance(mi);
          } else {
            log("warning: missing mesh %s\n", mesh_url);
          }
        }
      } else {
        mesh *mesh = dict.get_mesh(url);
        material *mat = dict.get_material("default_material");
        if (mesh) {
          mesh_instance *mi = new mesh_instance(node, mesh, mat, skel);
          s.add_mesh_instance(mi);
        }
      }
    }

    // add an <instance_geometry> mesh instance
    void add_instance_geometry(TiXmlElement *element, scene_node *node, resource_dict &dict, visual_scene &s) {
      const char *url = element->Attribute("url");
      url += url[0] == '#';
      TiXmlElement *bind_material = child(element, "bind_material");
      TiXmlElement *technique_common = child(bind_material, "technique_common");

      add_mesh_instances(technique_common, url, node, 0, dict, s);
    }

    // add an <instance_controller> skin instance
    void add_instance_controller(TiXmlElement *element, scene_node *node, resource_dict &dict, visual_scene &s) {
      const char *controller_url = attr(element, "url");
      TiXmlElement *bind_material = child(element, "bind_material");
      TiXmlElement *technique_common = child(bind_material, "technique_common");

      int num_bones = 0;
      for (TiXmlElement *skel_elem = child(element, "skeleton"); skel_elem; skel_elem = sibling(skel_elem, "skeleton")) {
        num_bones++;
      }

      //mesh *mesh = dict.get_mesh(controller_url);
      //if (!mesh || !mesh->get_skin()) return;
      //skin *skn = mesh->get_skin();

      skeleton *skel = new skeleton();
      TiXmlElement *skel_elem = child(element, "skeleton");
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
            skel->add_bone(node, parents[i]);
          }
        }
        skel_elem = sibling(skel_elem, "skeleton");
      }

      //const char *url = skin->Attribute("source");
      add_mesh_instances(technique_common, controller_url, node, skel, dict, s);
    }

    // utility to get a float
    float quick_float(TiXmlElement *parent, const char *name, float deflt=0) {
      TiXmlElement *child = parent->FirstChildElement(name);
      return child ? (float)atof(child->GetText()) : deflt;
    }

    // utility to get a float
    vec4 quick_vec(TiXmlElement *parent, const char *name) {
      TiXmlElement *child = parent->FirstChildElement(name);
      dynarray<float> v;
      if (child) atofv(v, child->GetText());
      unsigned s = v.size();
      return vec4(v[0], s > 1 ? v[1] : 0, s > 2 ? v[2] : 0, s > 3 ? v[3] : 1);
    }

    // add a camera to the scene
    void add_instance_camera(TiXmlElement *elem, scene_node *node, resource_dict &dict, visual_scene &s) {
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
        c->set_node(node);
        if (perspective) {
          float xfov = quick_float(params, "xfov");
          float yfov = quick_float(params, "yfov");
          c->set_perspective(xfov, yfov, aspect_ratio, n, f);
        } else {
          float xmag = quick_float(params, "xmag");
          float ymag = quick_float(params, "ymag");
          c->set_ortho(xmag, ymag, aspect_ratio, n, f);
        }
      }
    }

    // add a light to the scene
    void add_instance_light(TiXmlElement *elem, scene_node *node, resource_dict &dict, visual_scene &s) {
      const char *url = elem->Attribute("url");
      TiXmlElement *light_elem = find_id(url);
      if (!light_elem) return;

      light *_light = new light();
      light_instance *il = new light_instance(node, _light);
      s.add_light_instance(il);
      
      TiXmlElement *technique_common = child(light_elem, "technique_common");
      TiXmlElement *ambient = child(technique_common, "ambient");
      TiXmlElement *directional = child(technique_common, "directional");
      TiXmlElement *spot = child(technique_common, "spot");
      TiXmlElement *point = child(technique_common, "point");
      TiXmlElement *params = ambient ? ambient : directional ? directional : spot ? spot : point;

      _light->set_color(vec4(1, 1, 1, 1));
      if (params) {
        vec4 color = quick_vec(params, "color");
        _light->set_color(color);
      }

      if (directional || spot || point) {
        float constant_attenuation = quick_float(params, "constant_attenuation", 1);
        float linear_attenuation = quick_float(params, "linear_attenuation", 0);
        float quadratic_attenuation = quick_float(params, "quadratic_attenuation", 0);
        float falloff_angle = quick_float(params, "falloff_angle", 180);
        float falloff_exponent = quick_float(params, "falloff_exponent", 0);
        _light->set_attenuation(constant_attenuation, linear_attenuation, quadratic_attenuation);
        _light->set_falloff(falloff_angle, falloff_exponent);
      }
    }

    // add a geometry element to the list of mesh states
    void add_geometry(resource_dict &dict) {
      TiXmlElement *lib_geom = doc.RootElement()->FirstChildElement("library_geometries");
      if (!lib_geom) return;

      for (TiXmlElement *geometry = lib_geom->FirstChildElement(); geometry != NULL; geometry = geometry->NextSiblingElement()) {
        TiXmlElement *mesh_elem = child(geometry, "mesh");
        const char *id = geometry->Attribute("id");

        for (TiXmlElement *mesh_child = mesh_elem ? mesh_elem->FirstChildElement() : 0;
          mesh_child != NULL;
          mesh_child = mesh_child->NextSiblingElement()
        ) {
          if (is_mesh_component(mesh_child->Value())) {
            mesh *msh = new mesh();
            get_mesh_component(msh, id, mesh_child, NULL, dict);
          }
        }
      }
    }

    // add a geometry element to the list of mesh states
    void add_controllers(resource_dict &dict) {
      TiXmlElement *lib_ctrl = doc.RootElement()->FirstChildElement("library_controllers");
      if (!lib_ctrl) return;

      for (TiXmlElement *controller = lib_ctrl->FirstChildElement(); controller != NULL; controller = controller->NextSiblingElement()) {
        TiXmlElement *skin_elem = child(controller, "skin");
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

        mat4t modelToBind;

        if (skinst.bind_shape_matrix.size() >= 16) {
          modelToBind.init_transpose(&skinst.bind_shape_matrix[0]);
        } else {
          modelToBind.loadIdentity();
        }

        skin *mesh_skin = new skin(modelToBind);

        dynarray<string> joints;
        atonv(joints, skinst.joints);

        for (unsigned i = 0; i != joints.size(); ++i) {
          mat4t bindToModel;
          bindToModel.init_transpose(&skinst.inv_bind_matrices[i*16]);
          mesh_skin->add_joint(bindToModel, app_utils::get_atom(joints[i]));
        }

        TiXmlElement *vertex_weights = child(skin_elem, "vertex_weights");
        if (vertex_weights && geometry) {
          get_skin(controller, vertex_weights, &skinst);
          TiXmlElement *mesh_elem = child(geometry, "mesh");
          //const char *id = geometry->Attribute("id");

          for (TiXmlElement *mesh_child = mesh_elem ? mesh_elem->FirstChildElement() : 0;
            mesh_child != NULL;
            mesh_child = mesh_child->NextSiblingElement()
          ) {
            if (is_mesh_component(mesh_child->Value())) {
              mesh *msh = new mesh(mesh_skin);
              get_mesh_component(msh, controller_id, mesh_child, &skinst, dict);
            }
          }
        }
      }
    }

    // add <library_images> to the scene
    void add_images(resource_dict &dict) {
      TiXmlElement *lib_anim = doc.RootElement()->FirstChildElement("library_images");
      if (!lib_anim) return;

      for (TiXmlElement *elem = child(lib_anim, "image"); elem != NULL; elem = sibling(elem, "image")) {
        const char *url_attr = text(child(elem, "init_from"));
        if (url_attr) {
          string new_path;
          new_path.format("%s%s", doc_path.c_str(), url_attr);
          image *img = new image(new_path);
          dict.set_resource(attr(elem, "id"), img);
        }
      }
    }

    // add <library_animations> to the scene
    // collada animations range from sensible (array of matrices) to crazy (complex rotations and translations)
    void add_animations(resource_dict &dict) {
      TiXmlElement *lib_anim = doc.RootElement()->FirstChildElement("library_animations");
      if (!lib_anim) return;

      for (TiXmlElement *anim_elem = child(lib_anim, "animation"); anim_elem != NULL; anim_elem = sibling(anim_elem, "animation")) {
        animation *anim = new animation();
        const char *id = attr(anim_elem, "id");
        dict.set_resource(id, anim);
        if (debug > 0) log("animation %s\n", id);
        for (TiXmlElement *channel_elem = child(anim_elem, "channel"); channel_elem != NULL; channel_elem = sibling(channel_elem, "channel")) {
          const char *target = attr(channel_elem, "target");
          string node_name = target;
          string sub_target_name;
          string component_name;

          int slash = node_name.find("/");
          if (slash != -1) {
            node_name.truncate(slash);
            sub_target_name = target + slash + 1;
            int dot = sub_target_name.find(".");
            if (dot != -1) {
              sub_target_name.truncate(dot);
              component_name = target + slash + 1 + dot + 1;
            }
          }
          
          atom_t node_sid = app_utils::get_atom(node_name);
          atom_t sub_target_sid = app_utils::get_atom(sub_target_name);
          atom_t component_sid = app_utils::get_atom(component_name);
          
          if (debug > 0) log("  channel target %s %s %s\n", node_name.c_str(), sub_target_name.c_str(), component_name.c_str());
          TiXmlElement *sampler_elem = find_id(attr(channel_elem, "source"));
          if (sampler_elem) {
            dynarray<float> times;
            dynarray<float> values;
            //dynarray<string> interpolation;

            TiXmlElement *input = child(sampler_elem, "input");
            while (input) {
              const char *semantic = attr(input, "semantic");
              const char *source_id = attr(input, "source");
              if (!strcmp(semantic, "INPUT")) {
                TiXmlElement *float_array = child(find_id(source_id), "float_array");
                atofv(times, text(float_array));
              } else if (!strcmp(semantic, "OUTPUT")) {
                TiXmlElement *float_array = child(find_id(source_id), "float_array");
                atofv(values, text(float_array));
              } else if (!strcmp(semantic, "INTERPOLATION")) {
                /*TiXmlElement *name_array = child(find_id(source_id), "Name_array");
                if (name_array) {
                  atonv(interpolation, text(name_array));
                }*/
              }
              input = sibling(input, "input");
            }

            resource *target = dict.get_resource(node_name);
            anim->add_channel(target, node_sid, sub_target_sid, component_sid, times, values);
          }
        }
      }
    }

    // build the scene_node heirachy
    void build_heirachy(dynarray<TiXmlElement *> &node_elems, dynarray<scene_node *> &nodes, TiXmlElement *scene_element, resource_dict &dict, visual_scene &s) {
      // create a stack to avoid recursion (a bad thing in games)
      dynarray<TiXmlElement *> stack;
      dynarray<scene_node *> node_stack;
      stack.reserve(64);
      node_stack.reserve(64);

      node_stack.push_back(s.get_root_node());
      stack.push_back(scene_element);
      while (!stack.empty()) {
        TiXmlElement *parent_elem = stack.back();
        scene_node *parent = node_stack.back();
        stack.pop_back();
        node_stack.pop_back();
        TiXmlElement *node_elem = child(parent_elem, "node");
        while (node_elem) {
          mat4t nodeToParent;
          nodeToParent.loadIdentity();
          const char *sid = attr(node_elem, "sid");
          const char *id = attr(node_elem, "id");
          scene_node *new_node = new scene_node(nodeToParent, app_utils::get_atom(sid));
          if (debug > 0) log("add scene_node id=%s sid=%s\n", id, sid);
          dict.set_resource(id, new_node);
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
    void build_matrices(dynarray<TiXmlElement *> &node_elems, dynarray<scene_node *> &nodes, resource_dict &dict, visual_scene &s) {
      for (int ni = 0; ni != node_elems.size(); ++ni) {
        TiXmlElement *node_elem = node_elems[ni];
        scene_node *node = nodes[ni];
        mat4t &matrix = node->access_nodeToParent();
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
              matrix.rotate(temp_floats[3], temp_floats[0], temp_floats[1], temp_floats[2]);
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
    void build_instances(dynarray<TiXmlElement *> &node_elems, dynarray<scene_node *> &nodes, resource_dict &dict, visual_scene &s) {
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
          } else if (!strcmp(value, "instance_mesh")) {
            // we do not support instance_mesh yet as this requires a DAG
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
      for (TiXmlElement *input_elem = child(mesh_child, "input");
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
    void get_mesh_component(mesh *mesh, const char *id, TiXmlElement *mesh_child, skin_state *skinst, resource_dict &dict) {
      TiXmlElement *pelem = child(mesh_child, "p");

      if (!pelem) {
        printf("warning: no <p>\n");
        return;
      }

      // a geometry or controller is split up into its material groups
      // with a name of "geometry+material"
      // each requires a separate mesh instance to render
      const char *symbol = attr(mesh_child, "material");
      string new_url;
      const char *mesh_url = id;
      if (symbol) {
        new_url.format("%s+%s", id, symbol);
        mesh_url = new_url;
      }

      if (debug > 0 ) {
        log("created mesh %s\n", id);
      }

      dict.set_resource(mesh_url, mesh);

      parse_input_state state;
      state.s = mesh;
      while (pelem) {
        atoiv(state.p, pelem->GetText());
        pelem = sibling(pelem, "p");
      }
      state.input_stride = get_input_stride(mesh_child);
      //unsigned implicit_offset = 0;
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
      for (TiXmlElement *input = child(mesh_child, "input");
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
      for (TiXmlElement *input = child(mesh_child, "input");
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

      // skins need extra parameters for indices and weights
      // copy the processed blend vertices to the gl attributes using indices from the <p> array
      if (skinst) {
        unsigned p_size = state.p.size();
        unsigned num_vertices = p_size / state.input_stride;
        for (unsigned i = 0; i != num_vertices; ++i) {
          unsigned index = state.p[i * state.input_stride + state.vertex_input_offset];
          if (0) {
            log("i%d\n", index);
          }
          for (int j = 0; j != blendindices_stride; ++j) {
            state.vertices[state.attr_stride * i + blendindices_offset + j] = (float)state.skinst->gl_indices[index * blendindices_stride + j];
          }
          for (int j = 0; j != blendweight_stride; ++j) {
            state.vertices[state.attr_stride * i + blendweight_offset + j] = state.skinst->gl_weights[index * blendweight_stride + j];
          }
        }
      }

      TiXmlElement *vcount_elem = child(mesh_child, "vcount");

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

      if (debug > 0) {
        log("mesh component loaded with %d indices and %d floats for vertices\n", state.indices.size(), state.vertices.size());
      }

      mesh->allocate(vsize, isize);
      mesh->assign(vsize, isize, (unsigned char*)&state.vertices[0], (unsigned char*)&state.indices[0]);
      mesh->set_params(state.attr_stride * 4, num_indices, num_vertices, GL_TRIANGLES, GL_UNSIGNED_INT);
      mesh->calc_aabb();
      if (debug > 1) mesh->dump(log("mesh\n"));
    }

    // get blend weights and matrices from a skin
    // after this we are still not home yet as the weights need to be indexed by the POSITION of the skinned mesh.
    void get_skin(TiXmlElement *geometry, TiXmlElement *mesh_child, skin_state *skin) {
      TiXmlElement *pelem = child(mesh_child, "v");

      if (!pelem) {
        printf("warning: no <v>\n");
        return;
      }

      TiXmlElement *vcount_elem = child(mesh_child, "vcount");
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
      while (pelem) {
        atoiv(state.p, pelem->GetText());
        pelem = sibling(pelem, "p");
      }
      state.input_stride = get_input_stride(mesh_child);
      state.slot = 0;
      state.attr_offset = 0;
      state.skinst = skin;
      state.input_offset = 0;

      // build the raw skin paramerters
      for (TiXmlElement *input = child(mesh_child, "input");
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
        FILE *f = log("raw weights & indices\n");
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

    // add all the scenes from the collada file to the resources collection
    void add_scenes(resource_dict &dict) {
      TiXmlElement *lib = doc.RootElement()->FirstChildElement("library_visual_scenes");

      if (!lib) return;

      for (TiXmlElement *elem = lib->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement()) {
        dynarray<TiXmlElement *> node_elems;
        dynarray<scene_node *> nodes;
        visual_scene *scn = new visual_scene();
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
    bool load_xml(const char *url) {
      doc_path = url;
      doc_path.truncate(doc_path.filename_pos());
      const char *path = app_utils::get_path(url);
      char buf[256];
      getcwd(buf, sizeof(buf));
      doc.LoadFile(path);

      TiXmlElement *top = doc.RootElement();
      if (!top) {
        printf("file %s not found\n", path);
        return false;
      }

      if (strcmp(top->Value(), "COLLADA")) {
        printf("warning: not a collada file");
        return false;
      }

      find_ids(top);
      return true;
    }

    // once loaded, use this to access the first component in the mesh
    void get_mesh(mesh &s, const char *id, resource_dict &dict) {
      TiXmlElement *geometry = find_id(id);
      s.init();

      if (!geometry || strcmp(geometry->Value(), "geometry")) {
        printf("warning: geometry %s not found\n", id);
        return;
      }

      TiXmlElement *mesh = child(geometry, "mesh");
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
      TiXmlElement *ivs = child(scene, "instance_visual_scene");
      return ivs ? ivs->Attribute("url") : 0;
    }

    // extract resources from the collada file into a collection.
    void get_resources(resource_dict &dict) {
      add_images(dict);

      add_materials(dict);

      add_geometry(dict);

      add_controllers(dict);

      // scenes refer to all the above
      add_scenes(dict);

      // animations refer to all other objects
      add_animations(dict);
    }
  };
}}
