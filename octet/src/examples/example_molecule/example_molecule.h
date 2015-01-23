////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {
  template <int _Len> float atof(const char (&x)[_Len]) {
    char tmp[_Len+1];
    memcpy(tmp, x, _Len);
    tmp[_Len] = 0;
    return (float)std::atof(x);
  }

  /// Scene containing a box with octet.
  class example_molecule : public app {
    // scene for drawing box
    ref<visual_scene> app_scene;

    class molecule : public resource {
      aabb bbox;
      std::vector<vec4> pos;
      std::vector<vec4> bvh;
    public:
      molecule(const char *begin, const char *end) {
        vec3 min, max;
        bool first = true;
        for (const char *p = begin; p != end; ) {
          const char *b = p;
          while (p != end && *p != '\n') ++p;
          if (!memcmp(b, "ATOM  ", 6)) {
            // example: ATOM      1  CA  THR A   5      24.188 -18.763  55.368  1.00 46.05           C  
            struct fmt {
              char Record[6-1+1];         //1 - 6 Record name "ATOM "
              char serial[11-7+1];        //7 - 11 Integer serial Atom serial number.
              char pad12;
              char atomname[16-13+1];    //13 - 16 Atom name Atom name.
              char altloc[17-17+1];      //17 Character altLoc Alternate location indicator.
              char resname[20-18+1];     //18 - 20 Residue name resName Residue name.
              char pad21;
              char chainID[22-22+1];     //22 Character chainID Chain identifier.
              char resSeq[26-23+1];      //23 - 26 Integer resSeq Residue sequence number.
              char AChar[27-27+1];       //27 AChar iCode Code for insertion of residues.
              char pad28_30[30-28+1];
              char x[38-31+1];           //31 - 38 Real(8.3) x Orthogonal coordinates for X in Angstroms.
              char y[46-39+1];           //39 - 46 Real(8.3) y Orthogonal coordinates for Y in Angstroms.
              char z[54-47+1];           //47 - 54 Real(8.3) z Orthogonal coordinates for Z in Angstroms.
              char occupancy[60-55+1];   //55 - 60 Real(6.2) occupancy Occupancy.
              char tempFactor[60-55+1];  //61 - 66 Real(6.2) tempFactor Temperature factor.
              char pad67[76-67+1];
              char element[78-77+1];     //77 - 78 LString(2) element Element symbol, right-justified.
              char charge[80-79+1];      //79 - 80 LString(2) charge Charge on the atom.
            };

            static const struct { char name[3]; uint32_t colour; } colours[] = {
              {" H", 0xcccccc}, {" C", 0xaaaaaa}, {" O", 0xcc0000}, {" N", 0x0000cc}, {" S", 0xcccc00},
              {" P", 0x6622cc}, {" F", 0x00cc00}, {"CL", 0x00cc00}, {"BR", 0x882200}, {" I", 0x6600aa},
              {"FE", 0xcc6600}, {"CA", 0x8888aa}, {"", 0}
            };

            // Reference: glMol / A. Bondi, J. Phys. Chem., 1964, 68, 441.
            static const struct { char name[3]; float radius; } radii[] = {
              {" H", 1.2f}, {"LI", 1.82f}, {"NA", 2.27f}, {" K", 2.75f}, {" C", 1.7f}, {" N", 1.55f}, {" O", 1.52f},
              {" F", 1.47f}, {" P", 1.80f}, {" S", 1.80f}, {"CL", 1.75f}, {"BR", 1.85f}, {"SE", 1.90f},
              {"ZN", 1.39f}, {"CU", 1.4f}, {"NI", 1.63f}
            };

            const fmt *f = (const fmt*)b;
            float x = atof(f->x);
            float y = atof(f->y);
            float z = atof(f->z);
            vec3 p(x, y, z);
            if (first) {
              min = max = p;
              first = false;
            } else {
              min = math::min(min, p);
              max = math::max(max, p);
            }
            vec4 colour(0.5f, 0.5f, 0.5f, 1.0f);
            for (auto c : colours) {
              if (c.name[0] == f->element[0] && c.name[1] == f->element[1]) {
                colour = vec4((c.colour >> 16) * (1.0f/255), ((c.colour >> 8) & 0xff) * (1.0f/255), (c.colour & 0xff) * (1.0f/255), 1.0f);
                break;
              }
            }
            float radius = 1.0f;
            for (auto r : radii) {
              if (r.name[0] == f->element[0] && r.name[1] == f->element[1]) {
                radius = r.radius;
                break;
              }
            }
            pos.emplace_back(x, y, z, radius);
            //pos.emplace_back(colour);
          }
          if (p != end) ++p;
        }
        bbox = aabb((min+max)*0.5f, (max-min)*0.5f);
        std::cout << min << "\n";
        std::cout << max << "\n";
        std::cout << (int)pos.size() << "\n";

        typedef std::vector<vec4>::iterator it;
        struct stack_t {
          it begin;
          it end;
          int level;
          stack_t(it begin, it end, int level) : begin(begin), end(end), level(level) {}
        };
        std::deque<stack_t> stack;

        pos.resize(1024+64);
        int pos_size = (int)pos.size();
        it start = pos.begin();

        int lg_pos_size = 1;
        while (1<<(lg_pos_size+1) < pos_size) ++lg_pos_size;

        stack.emplace_back(pos.begin(), pos.end(), 0);
        while (!stack.empty()) {
          it begin = stack.back().begin;
          it end = stack.back().end;
          int level = stack.back().level;
          stack.pop_back();
          aabb bb(begin, end);

          vec3 ha = bb.get_half_extent();
          vec3 cen = bb.get_center();
          float r = 0;
          std::for_each(begin, end, [&r, cen](vec4_in a) { r = std::max(r, math::length(a.xyz() - cen) + a.w()); } );

          bvh.emplace_back(bb.get_center(), r);
          //fred << bvh.size() << " " << bb.get_min() << " " << bb.get_max() << " " << bvh.back() << " " << end-begin << "\n";
          char tmp[256];
          log("%d %s %d\n", (int)bvh.size(), bvh.back().toString(tmp, sizeof(tmp)), end-begin);
          fflush(log(""));

          if (begin+1 != end) {
            // sort by the longest dimension.
            if (ha.x() >= ha.y() && ha.x() >= ha.z()) {
              std::sort(begin, end, [](vec4_in a, vec4_in b) {return a.x() < b.x();});
            } else if(ha.y() >= ha.x() && ha.y() >= ha.z()) {
              std::sort(begin, end, [](vec4_in a, vec4_in b) {return a.y() < b.y();});
            } else {
              std::sort(begin, end, [](vec4_in a, vec4_in b) {return a.z() < b.z();});
            }

            if (level <= lg_pos_size) {
              // split the atoms into equal sized groups
              //size_t mid = end - begin == 2 ? 1 : end - begin == 3 ? 2 : (end - begin + 3)/2 & ~1;
              ptrdiff_t mid = std::min((ptrdiff_t)1 << (lg_pos_size - level), end - begin);
              if (mid >= 1) {
                stack.emplace_front(begin, begin + mid, level+1);
              }

              if (end - (begin + mid) >= 1) {
                stack.emplace_front(begin + mid, end, level+1);
              }
            }
          }
        }
      }

      const std::vector<vec4> &get_pos() const {
        return pos;
      }

      const std::vector<vec4> &get_bvh() const {
        return bvh;
      }

      // get all spheres in this radius
      template <class _OutIt> void query(_OutIt dest, vec3_in centre, float radius) {
        /*int stack[32];
        int stack_in = 0;
        int stack_out = 0;
        stack[stack_in++] = 0;
        while (stack_out != stack_in) {
          int i = stack[stack_out++];
          if (squared(bvh[i].xyz() - centre) <= squared(radius + bvh[i].w)) {
            //stack[stack_in++] = 
            //stack[stack_in++] = 
          }
        }
        */
      }
    };

    ref<material> custom_mat;

    ref<param_uniform> camera_pos;
    ref<param_uniform> spheres;
    ref<param_uniform> num_spheres;

    ref<molecule> mol;
  public:
    /// this is called when we construct the class before everything is initialised.
    example_molecule(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();
      app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, 0, 50));
      app_scene->get_camera_instance(0)->set_far_plane(400);
      app_scene->get_camera_instance(0)->set_near_plane(0.1f);

      dynarray<uint8_t> buf;
      app_utils::get_url(buf, "assets/pdb1fha.ent");
      mol = new molecule((const char*)buf.data(), (const char*)buf.data() + buf.size());

      param_shader *shader = new param_shader("shaders/default.vs", "shaders/raycast_molecule.fs");
      custom_mat = new material(vec4(1, 1, 1, 1), shader);
      atom_t atom_camera_pos = app_utils::get_atom("camera_pos");
      atom_t atom_spheres = app_utils::get_atom("spheres");
      atom_t atom_num_spheres = app_utils::get_atom("num_spheres");
      camera_pos = custom_mat->add_uniform(nullptr, atom_camera_pos, GL_FLOAT_VEC3, 1, param::stage_fragment);
      spheres = custom_mat->add_uniform(nullptr, atom_spheres, GL_FLOAT_VEC4, 200, param::stage_fragment);
      num_spheres = custom_mat->add_uniform(nullptr, atom_num_spheres, GL_INT, 1, param::stage_fragment);

      mesh_box *box = new mesh_box(vec3(20));
      scene_node *node = new scene_node();
      app_scene->add_child(node);
      //app_scene->add_mesh_instance(new mesh_instance(node, box, custom_mat));

      //mesh *mol = new mesh(
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      glCullFace(GL_BACK);
      glFrontFace(GL_CW);
      glEnable(GL_CULL_FACE);

      /*scene_node *camera_node = app_scene->get_camera_instance(0)->get_node();
      scene_node *box_node = app_scene->get_mesh_instance(0)->get_node();

      // camera position in model space
      vec3 pos = box_node->inverse_transform(camera_node->get_position());
      custom_mat->set_uniform(camera_pos, &pos, sizeof(pos));

      // exp(-r^2*k)=t
      // -r^2*k=log(t) k = -log(t)/(r^2)
      //static const vec4 data[] = { vec4(0, 0, 0, -0.3f), vec4(1, 0, 0, 1), vec4(-3, 0, 0, -0.6f), vec4(1, 1, 0, 1), vec4(0, 3, 0, -0.3f), vec4(1, 1, 1, 1), vec4(0, -3, 0, -0.4f), vec4(0, 1, 1, 1) };
      //custom_mat->set_uniform(spheres, data, sizeof(data));
      std::vector<vec4> data;
      for (int y = 0; y != 10; ++y) {
        for (int x = 0; x != 10; ++x) {
          data.push_back(vec4((x-4.5f)*1.6f, (y-4.5f)*1.6f, 0, -8.0f));
          data.push_back(vec4(1, 0, 0, 1));
        }
      }

      custom_mat->set_uniform(spheres, data.data(), data.size()*sizeof(data[0]));
      int ns = 100;
      custom_mat->set_uniform(num_spheres, &ns, sizeof(ns));

      // update matrices. assume 30 fps.
      app_scene->update(1.0f/30);

      // draw the scene
      app_scene->render((float)vx / vy);

      // tumble the box  (there is only one mesh instance)
      box_node->rotate(1, vec3(1, 0, 0));
      box_node->rotate(1, vec3(0, 1, 0));*/
    }
  };
}
