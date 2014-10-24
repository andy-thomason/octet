////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// load an OBJ file.
//
namespace octet { namespace loaders {
  /// Class for loading OBJ files.
  class obj_loader {
  public:
    obj_loader() {
    }

    /// Load an OBJ file
    /// http://en.wikipedia.org/wiki/Wavefront_.obj_file
    bool load(const char *url, resource_dict &dict, visual_scene *scene) {
      dynarray<uint8_t> file;
      app_utils::get_url(file, url);
      if (file.size() == 0) return false;

      const uint8_t *eof = file.data() + file.size();
      this->dict = &dict;
      material_index = 0;
      
      for (const uint8_t *src = file.data(); src != eof; ) {
        while (*src == ' ' && src != eof) ++src;
        const uint8_t *begin = src;
        while (*src != '\n' && *src != '\r' && src != eof) ++src;
        const uint8_t *end = src;
        src += src != eof && *src == '\r';
        src += src != eof && *src == '\n';
        if (begin != end ) switch (begin[0]) {
          case '#': {
            fwrite(begin, 1, end-begin, stdout);
          } break;
          case 'o': {
            flush();
            fwrite(begin, 1, end-begin, stdout);
            if (begin[1] == ' ') obj_name.assign((const char*)begin + 2, (const char*)end);
            node = new scene_node(mat4t(), atom_);
          } break;
          case 'g': {
            fwrite(begin, 1, end-begin, stdout);
            if (begin[1] == ' ') group_name.assign((const char*)begin + 2, (const char*)end);
          } break;
          case 'v': {
            //fwrite(begin, 1, end-begin, stdout);
            if (begin[1] == ' ') {
              atofv(values, begin+2, end);
              if (values.size() == 3) {
                src_vertices.push_back(vec3p(values[0], values[1], values[2]));
              }
            } else if (begin[1] == 't' && begin[2] == ' ') {
              atofv(values, begin+3, end);
              if (values.size() == 2) {
                src_uvs.push_back(vec2p(values[0], values[1]));
              }
            } else if (begin[1] == 'n' && begin[2] == ' ') {
              atofv(values, begin+3, end);
              if (values.size() == 3) {
                src_normals.push_back(vec3p(values[0], values[1], values[2]));
              }
            }
          } break;
          case 'f': {
            //fwrite(begin, 1, end-begin, stdout);
            if (begin[1] == ' ') {
              unsigned slashes = 0;
              mesh::vertex v[6];
              atoiv(ivalues, slashes, begin + 2, end);
              size_t num_values = ivalues.size() - slashes;
              size_t num_comps = ivalues.size() / num_values;
              size_t num_idx = ivalues.size() / num_comps;
              if (num_idx > 4 || num_comps > 3) {
                printf("warning: bad obj file face\n");
                return false;
              }
              mesh::vertex vtx[4];
              memset(vtx, 0, sizeof(vtx));
              if (num_comps >= 1) {
                size_t d = 0;
                for (size_t i = 0; i < ivalues.size(); i += num_comps) {
                  int iv = ivalues[i];
                  vtx[d++].pos = src_vertices[iv < 0 ? src_vertices.size() + iv : iv];
                }
              }
              if (num_comps >= 2) {
                size_t d = 0;
                for (size_t i = 1; i < ivalues.size(); i += num_comps) {
                  int iv = ivalues[i];
                  vtx[d++].uv = src_uvs[iv < 0 ? src_uvs.size() + iv : iv];
                }
              }
              if (num_comps >= 3) {
                size_t d = 0;
                for (size_t i = 2; i < ivalues.size(); i += num_comps) {
                  int iv = ivalues[i];
                  vtx[d++].normal = src_normals[iv < 0 ? src_normals.size() + iv : iv];
                }
              }

              face f;
              f.vtx[0] = vtx[0];
              f.vtx[1] = vtx[1];
              f.vtx[2] = vtx[2];
              f.material_index = material_index;
              faces.push_back(f);
              if (num_idx == 4) {
                f.vtx[0] = vtx[0];
                f.vtx[1] = vtx[2];
                f.vtx[2] = vtx[3];
                f.material_index = material_index;
                faces.push_back(f);
              }
            }
          } break;
          default: {
            fwrite(begin, 1, end-begin, stdout);
            printf("unknown\n");
            return false;
          } break;
          case 'u': {
            fwrite(begin, 1, end-begin, stdout);
            if (begin+7 < end && !memcmp(begin, "usemtl ", 7)) {
              size_t len = end - (begin+7);
              size_t i = 0;
              for (; i != materials.size(); ++i) {
                if (
                  materials[i].size() == len &&
                  !memcmp(materials[i].c_str(), begin+7, len)
                ) {
                  break;
                }
              }

              if (i == materials.size()) {
                materials.push_back(string((const char*)begin+7, (unsigned)len));
              }

              material_index = (uint32_t)i;
            }
          } break;
        }
      }
      flush();

      return true;
    }
  private:
    dynarray<vec3p> src_vertices;
    dynarray<vec3p> src_normals;
    dynarray<vec2p> src_uvs;
    dynarray<uint32_t> src_material;
    dynarray<string> materials;

    string obj_name;
    string group_name;
    resource_dict *dict;
    scene_node *node;

    struct face {
      mesh::vertex vtx[3];
      int material_index;

      bool operator <(const face &rhs) {
        return material_index < rhs.material_index;
      }
    };

    dynarray<face> faces;
    dynarray<uint32_t> indices;
    dynarray<float> values;
    dynarray<int> ivalues;
    uint32_t material_index;

    // convert an ascii sequence of integers like "1 3 9 12 34" to an array of integers
    void atoiv(dynarray<int> &values, unsigned &slashes, const uint8_t *src, const uint8_t *end) {
      values.resize(0);
      slashes = 0;

      while (src != end && *src > 0 && *src <= ' ') ++src;
      while(src != end) {
        int whole = 0, msign = 1;
        if (src != end && *src == '-') { msign = -1; src++; }
        while (src != end && *src >= '0' && *src <= '9') whole = whole * 10 + (*src++ - '0');
        values.push_back(whole * msign);
        while (src != end && *src > 0 && *src <= ' ') ++src;
        if (src != end && *src == '/') { slashes++; src++; }
      }
    }

    void atofv(dynarray<float> &values, const uint8_t *src, const uint8_t *end) {
      values.resize(0);

      while (*src > 0 && *src <= ' ') ++src;
      while(src != end) {
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

    void flush() {
      std::sort(faces.begin(), faces.end());
      face *f = 0;
      gl_resource *vertices = new gl_resource(GL_ARRAY_BUFFER, faces.size() * sizeof(f->vtx));

      {
        gl_resource::wolock vl(vertices);
        for (size_t i = 0; i != faces.size(); ++i) {
          memcpy(vl.u8() + i * sizeof(f->vtx), faces[i].vtx, sizeof(f->vtx));
        }
      }

      int prev_mi = -1;
      for (size_t i = 0; i != faces.size(); ++i) {
        if (faces[i].material_index != prev_mi) {
          mesh *msh = new mesh();
          msh->set_default_attributes();
          msh->set_vertices(vertices);
          material *mat = new material(vec4(0.5f, 0.5f, 0.5f, 1));
          mesh_instance *mi = new mesh_instance(node, msh, mat);
        }
      }

      faces.resize(0);
      src_vertices.resize(0);
      src_uvs.resize(0);
      src_normals.resize(0);
    }
  };
}}
