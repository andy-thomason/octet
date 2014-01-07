////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// obb collision testing.
//
// Level: 2
//
// Work for a paper on OBB collision

namespace octet {
  class fcs_file : public resource {
    struct param {
      std::string short_name;
      std::string name;
      unsigned bits;
      unsigned amp_type;
      bool is_log;
      unsigned range;
      float gain;

      param() { bits = 32; amp_type = 0; is_log = false; range = 0; gain = 1.0f; }
    };

    dynarray<float> data;
    dynarray<param> params;
    char datatype;
    char byteord;

    char key[65536];
    char value[65536];


    const uint8_t *read_segment(char *dest, size_t size, const uint8_t *src) {
      if (*src != 12) return src+1;
      ++src;
      unsigned i = 0;
      while (*src != 12) {
        if (i+1 < size) dest[i++] = *src;
        src++;
      }
      dest[i] = 0;
      return src;
    }

    void process_text(const uint8_t *text_min, const uint8_t *text_max) {
      const uint8_t *src = text_min;
      src = read_segment(value, sizeof(value), src);
      while (src < text_max) {
        if (value[0] != '$') {
          printf("odd text segment %s\n", value); return;
        }
        strcpy(key, value+1);
        //printf("%s,", value);
        unsigned arg = 0;
        while (src < text_max) {
          src = read_segment(value, sizeof(value), src);
          if (value[0] == '$') break;
          //printf("%s,", value);

          if (!strcmp(key, "DATATYPE") && arg == 0) {
            datatype = value[0];
            //printf("DATATYPE %s\n", value);
          } else if (!strcmp(key, "BYTEORD") && arg == 0) {
            byteord = value[0];
            //printf("BYTEORD %s\n", value);
          } else if (!strcmp(key, "NEXTDATA") && arg == 0) {
            //printf("NEXTDATA %s\n", value);
          } else if (!strcmp(key, "PAR") && arg == 0) {
            params.resize(atoi(value));
            //printf("PAR %s\n", value);
          } else if (key[0] == 'P' && key[1] >= '0' && key[1] <= '9') {
            unsigned pnum = 0;
            const char *src = key + 1;
            for (; *src >= '0' && *src <= '9'; ++src) {
              pnum = pnum * 10 + *src - '0';
            }
            param &p = params[pnum - 1];
            if (!strcmp(src, "B")) {
              p.bits = atoi(value);
            } else if (!strcmp(src, "E")) {
              //printf("%s %d %s\n", key, arg, value);
              //p.amp_type = value;
            } else if (!strcmp(src, "G")) {
              //printf("%s %d %s\n", key, arg, value);
              if (arg == 0) {
                p.gain = (float)atof(value);
              } else if (arg == 2) {
                p.is_log = strcmp(value, "LOG") == 0;
              }
            } else if (!strcmp(src, "R")) {
              //c
              //p.amp_type = value;
            } else if (!strcmp(src, "S")) {
              p.short_name = value;
            } else if (!strcmp(src, "N")) {
              p.name = value;
            } else if (!strcmp(src, "V")) {
              //printf("%s %d %s\n", key, arg, value);
            } else {
              printf("unknown: %s %d %s\n", key, arg, value);
            }
          }

          arg++;
        }
        //printf("\n");
      }
    }
    void process_data(const uint8_t *data_min, const uint8_t *data_max) {
      switch (datatype) {
        case 'F': {
          data.reserve((data_max - data_min) / sizeof(float));
          for (const uint8_t *src = data_min; src+3 < data_max; src += 4) {
            union { float f; uint8_t u[4]; } u;
            if (byteord == '1') {
              u.u[0] = src[0]; u.u[1] = src[1]; u.u[2] = src[2]; u.u[3] = src[3];
            } else {
              u.u[0] = src[3]; u.u[1] = src[2]; u.u[2] = src[1]; u.u[3] = src[0];
            }
            data.push_back(u.f);
          }
        } break;
        default: {
          printf("unsupported data type %c\n", datatype);
        } break;
      }
    }

    void write_csv(const char *filename) {
      FILE *csv = fopen(filename, "wb");
      if (!csv) { printf("unable to open %s\n", filename); return; }

      int np = (int)params.size();
      int nevents = (int)(data.size() / params.size());

      for (int j = 0; j != np; ++j) {
        fprintf(csv, j==0 ? "%s" : ",%s", params[j].name.c_str());
      }
      fprintf(csv, "\n");

      for (int j = 0; j != np; ++j) {
        fprintf(csv, j==0 ? "%s" : ",%s", params[j].is_log ? "LOG" : "LIN");
      }
      fprintf(csv, "\n");

      for (int i = 0; i != nevents; ++i) {
        for (int j = 0; j != np; ++j) {
          fprintf(csv, j==0 ? "%f" : ",%f", data[i*np+j]);
        }
        fprintf(csv, "\n");
      }
      fclose(csv);
    }
  public:
    fcs_file(const char *fcs) {
      // swallow the file
      FILE *fp = fopen(fcs, "rb");
      fseek(fp, 0, SEEK_END);
      long len = ftell(fp);
      fseek(fp, 0, SEEK_SET);
      uint8_t *file_data = (uint8_t *)malloc((size_t)len);
      fread(file_data, 1, (size_t)len, fp);
      fclose(fp);

      // read header
      int verh, verl, textb, texte, datab, datae;
      int n = _snscanf((char*)file_data, 58, "FCS%d.%d %d %d %d %d", &verh, &verl, &textb, &texte, &datab, &datae);
      if (n != 6) { printf("not a FCS file\n"); return ; }

      process_text(file_data + textb, file_data + texte);
      process_data(file_data + datab, file_data + datae);

      printf("%d params, %d data items\n", params.size(), data.size());

      //write_csv(out);
    }

    int get_num_values() {
      return (int)(data.size() / params.size());
    }

    float get_value(int row, int col) {
      assert((unsigned)col < params.size());
      return data[params.size() * row + col];
    }

    float get_gain(int col) {
      assert((unsigned)col < params.size());
      return (float)params[col].gain;
    }

    void get_mean(dynarray<float> &mean) {
      int num_params = (int)params.size();
      mean.resize(num_params);
      for (int i = 0; i != num_params; ++i) {
        mean[i] = 0;
      }

      int num_values = get_num_values();
      for (int j = 0; j != num_values; ++j) {
        for (int i = 0; i != num_params; ++i) {
          mean[i] += data[j*num_params + i];
        }
      }

      float scale = 1.0f / num_values;
      for (int i = 0; i != num_params; ++i) {
        mean[i] *= scale;
      }
    }
  };

  class flow_cytometry : public app {
    typedef mat4t mat4t;
    typedef vec4 vec4;
    typedef animation animation;
    typedef scene_node scene_node;

    // shaders to draw triangles
    bump_shader object_shader;
    bump_shader skin_shader;

    mouse_ball ball;

    ivec3 samples;

    ref<scene> app_scene;
    ref<mesh_instance> axis_mi;
    ref<mesh_instance> points_mi;

    ref<fcs_file> file;

    void update() {
      mesh_points *points_mesh = points_mi->get_mesh()->get_mesh_points();

      vec4 white(1, 1, 1, 1);
      int num_values = file->get_num_values();
      //vec3 scale(20.0f/file->get_gain(1), 20.0f/file->get_gain(2), 20.0f/file->get_gain(3));
      vec3 big(0.0f);
      vec3 invalid(262143.0f);
      dynarray<float> mean;
      file->get_mean(mean);

      for (int i = 0; i != num_values; ++i) {
        vec3 pt(
          file->get_value(i, samples[0]),
          file->get_value(i, samples[1]),
          file->get_value(i, samples[2])
        );
        if (any(pt != invalid)) {
          big = max(big, pt);
          char tmp[256], tmp2[256];
          //app_utils::log("%s %s\n", pt.toString(tmp, sizeof(tmp)), big.toString(tmp2, sizeof(tmp2)));
        }
      }

      vec3 scale = vec3(
        10.0f / mean[samples[0]],
        10.0f / mean[samples[1]],
        -10.0f / mean[samples[2]]
      );

      for (int i = 0; i != num_values; ++i) {
        vec3 pt(
          file->get_value(i, samples[0]),
          file->get_value(i, samples[1]),
          file->get_value(i, samples[2])
        );
        if (any(pt != invalid)) {
          points_mesh->add_point(pt * scale, white);
        }
      }
      points_mesh->update();
      points_mesh->set_mode(GL_POINTS);
    }

    void load_file(const char *filename) {
      file = new fcs_file(filename);
      update();
    }


  public:
    // this is called when we construct the class
    flow_cytometry(int argc, char **argv) : app(argc, argv), ball(), samples(1, 2, 3) {
    }

    // this is called once OpenGL is initialized
    void app_init() {
      // set up the shaders
      object_shader.init(false);
      skin_shader.init(true);

      app_scene =  new scene();
      app_scene->create_default_camera_and_lights();
      scene_node *points_node = app_scene->add_scene_node();
      scene_node *axis_node = app_scene->add_scene_node();
      material *mat = new material(vec4(1, 1, 1, 1));

      mat4t mx;
      mx.loadIdentity();
      mesh_points *points_mesh = new mesh_points();
      //points_mesh->dump(app_utils::log("points_mesh\n"));

      mesh_points *axis_mesh = new mesh_points();
      vec4 white(1, 1, 1, 1);
      axis_mesh->add_point(vec3(0, 0, 0), white);
      axis_mesh->add_point(vec3(10, 0, 0), white);
      axis_mesh->add_point(vec3(0, 0, 0), white);
      axis_mesh->add_point(vec3(0, 10, 0), white);
      axis_mesh->add_point(vec3(0, 0, 0), white);
      axis_mesh->add_point(vec3(0, 0, -10), white);
      axis_mesh->set_mode(GL_LINES);
      axis_mesh->update();

      axis_mi = app_scene->add_mesh_instance(new mesh_instance(points_node, points_mesh, mat));
      points_mi = app_scene->add_mesh_instance(new mesh_instance(axis_node, axis_mesh, mat));

      camera_instance *cam = app_scene->get_camera_instance(0);
      scene_node *node = cam->get_node();
      node->access_nodeToParent().translate(0, 0, 5);
      mat4t cameraToWorld = node->get_nodeToParent();
      ball.init(this, cameraToWorld.w().length(), 360.0f);

      load_file("c:/projects/flow/26529.fcs");
    }

    // this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      dynarray<string> &queue = access_load_queue();
      if (queue.size()) {
        // replace scene
        string url;
        url.urlencode(queue[0]);
        load_file(url);
        queue.resize(0);
      }

      if (app_scene && app_scene->get_num_camera_instances()) {
        int vx = 0, vy = 0;
        get_viewport_size(vx, vy);
        app_scene->begin_render(vx, vy, vec4(0.0f, 0.0f, 0.0f, 1.0f));

        camera_instance *cam = app_scene->get_camera_instance(0);
        scene_node *node = cam->get_node();
        mat4t &cameraToWorld = node->access_nodeToParent();
        ball.update(cameraToWorld);

        // update matrices. assume 30 fps.
        app_scene->update(1.0f/30);

        app_scene->render(object_shader, skin_shader, *cam, (float)vx / vy);

        //mesh_voxels *points_mesh = axis_mi->get_mesh()->get_mesh_voxels();
        //mesh_voxels *axis_mesh = points_mi->get_mesh()->get_mesh_voxels();
      }
    }
  };
}
