////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {
  /// example of using spring meshes arranged as tetrahedra.
  /// Note that we are not using a very sophisticated model which would model
  /// the deformation of the individual tertahedra, for example in preserving volume.
  class example_tetra : public app {
    enum { debug = 0 };
  public:
    /// this is called when we construct the class before everything is initialised.
    example_tetra(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();

      gravity = vec3(0, -10.0f, 0);

      #if 0
        vertices.push_back(vertex(vec3(0, 0, 0), 1, 0));
        vertices.push_back(vertex(vec3(0, -1, 0), 1, 1));
        vertices.push_back(vertex(vec3(0, -2, 0), 1, 1));

        //vertices.push_back(vertex(vec3(1, 0, 0), 1, 0));
        //vertices.push_back(vertex(vec3(1, -1, 0), 1, 1));
        //vertices.push_back(vertex(vec3(1, -2, 0), 1, 1));

        spring_const = 10.0f * 10.0f;
        damping_const = 0.0f;
        make_edge(0, 1);
        //make_edge(1, 2);

        //damping_const = 10.0f;
        //make_edge(3, 4);
        //make_edge(4, 5);
      #elif 0
        spring_const = 1000.0f * 10.0f;
        damping_const = 0.0f;

        // rope bridge
        size_t n = 4;
        for (size_t i = 0; i <= n; ++i) {
          vertices.push_back(vertex(vec3(i, 0, 0), 1, 1));
        }

        for (size_t i = 0; i != n; ++i) {
          make_edge(i, i+1);
        }

        vertices[0].gravity_mass = vertices[0].inv_mass = 0;
        vertices[n].gravity_mass = vertices[n].inv_mass = 0;
      #else
        // make a box-shaped tetra mesh.
        int num_segments = 10;
        float total_mass = 1.0f;
        float inv_mass = ( num_segments + 1 ) * 4 / total_mass;
        vec3 v0 = vec3(-num_segments/2.0f, 0, 0);
        vec3 v1 = vec3(-num_segments/2.0f, 1, 0);
        vec3 v2 = vec3(-num_segments/2.0f, 0, 1);
        vec3 v3 = vec3(-num_segments/2.0f, 1, 1);
        vec3 dx(1, 0, 0);
        for (int i = 0; i <= num_segments; ++i) {
          vertices.push_back(vertex(v0, 1, 1));
          vertices.push_back(vertex(v1, 1, 1));
          vertices.push_back(vertex(v2, 1, 1));
          vertices.push_back(vertex(v3, 1, 1));
          v0 += dx;
          v1 += dx;
          v2 += dx;
          v3 += dx;
        }

        #if 0
          for (int i = 0; i <= num_segments; ++i) {
            vertices[i*4 + 1].inv_mass = 0;
            vertices[i*4 + 3].inv_mass = 0;
          }
        #else
          // anchor corners by zeroing inv mass
          vertices[0].inv_mass = 0;
          vertices[1].inv_mass = 0;
          vertices[2].inv_mass = 0;
          vertices[3].inv_mass = 0;
          vertices[num_segments*4+0].inv_mass = 0;
          vertices[num_segments*4+1].inv_mass = 0;
          vertices[num_segments*4+2].inv_mass = 0;
          vertices[num_segments*4+3].inv_mass = 0;
        #endif

        spring_const = 10000.0f;
        damping_const = 10.0f;
        for (int i = 0; i != num_segments; ++i) {
          int s = i * 4;

          // the mask determines which faces will be rendered.
          unsigned mask = 0x02 | 0x04 | 0x10 | 0x20;
          if (i == 0) mask |= 0x01;
          if (i == num_segments-1) mask |= 0x08;
          make_cube(s, s+1, s+2, s+3, s+4, s+5, s+6, s+7, mask);
        }
      
      
        remove_duplicate_edges();
        //remove_internal_faces();
      #endif

      msh = new mesh();
      msh->clear_attributes();
      msh->add_attribute(attribute_pos, 3, GL_FLOAT, 0);
      msh->add_attribute(attribute_normal, 3, GL_FLOAT, 12);
      msh->add_attribute(attribute_uv, 2, GL_FLOAT, 24);
      msh->set_params(sizeof(mesh::vertex), faces.size() * 3, vertices.size(), GL_TRIANGLES, GL_UNSIGNED_INT);

      msh->allocate(vertices.size() * sizeof(mesh::vertex), faces.size() * sizeof(uint32_t) * 3);

      {
        gl_resource::wolock idx_lock(msh->get_indices());
        uint32_t *dest = idx_lock.u32();
        for (size_t i = 0; i != faces.size(); ++i) {
          face &f = faces[i];
          dest[0] = f.p0;
          dest[1] = f.p1;
          dest[2] = f.p2;
          dest += 3;
        }
      }

      material *red = new material(vec4(1, 0, 0, 1));
      scene_node *node = new scene_node();
      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, msh, red));
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      // update matrices. assume 30 fps.
      app_scene->update(1.0f/30);

      if (0) {
        simulate(0, 1.0f/30);
      } else {
        for (int i = 0; i != 10; ++i) {
          simulate(i, 1.0f/300);
        }
      }

      build_mesh();
      //msh->dump(log(""));
      //exit(1);

      // draw the scene
      app_scene->render((float)vx / vy);

      // tumble the box  (there is only one mesh instance)
      //scene_node *node = app_scene->get_mesh_instance(0)->get_node();
      //node->rotate(1, vec3(1, 0, 0));
      //node->rotate(1, vec3(0, 1, 0));
    }

  private:
    // scene for drawing box
    ref<visual_scene> app_scene;

    ref<mesh> msh;

    // a vertex in the tetrahedral spring system
    // note that we pack scalars in with the vectors for efficiency
    // as each vec3/vec4 is 16 bytes and is aligned...
    struct vertex {
      vec3p position;
      float radius;
      vec3p prev_position;
      float inv_mass;
      vec3p force;
      float gravity_mass;
      vec3p normal;

      vertex() {}
      vertex(vec3_in position, float radius, float inv_mass) :
        position(position), radius(radius), inv_mass(inv_mass)
      {
        gravity_mass = inv_mass ? 1.0f / inv_mass : 0.0f;
        prev_position = position;
      }
    };

    // an edge  in the tetrahedral spring system
    struct edge {
      int p0;
      int p1;
      float spring_const;
      float damping_const;
      float natural_length;
    };

    // a face for rendering
    struct face {
      int p0;
      int p1;
      int p2;
      //int winding;
    };

    dynarray<vertex> vertices;
    dynarray<edge> edges;
    dynarray<face> faces;

    float spring_const;
    float damping_const;
    vec3 gravity;

    void simulate(int step_num, float delta_t) {
      float rdelta_t = 1.0f / delta_t;
      // clear vertices and apply gravity
      float total_pe = 0;
      vec3 cofg(0, 0, 0);
      float total_mass = 0;
      for (size_t i = 0; i != vertices.size(); ++i) {
        vertex &v = vertices[i];
        v.force = gravity * v.gravity_mass;
        total_pe += dot(-(vec3)v.position, gravity) * v.gravity_mass;
        cofg += v.position * v.gravity_mass;
        total_mass += v.gravity_mass;
      }

      // apply spring forces
      for (size_t i = 0; i != edges.size(); ++i) {
        edge &e = edges[i];
        vec3 delta_x = (vec3)vertices[e.p1].position - (vec3)vertices[e.p0].position;
        vec3 v0_dt = (vec3)vertices[e.p0].position - (vec3)vertices[e.p0].prev_position;
        vec3 v1_dt = (vec3)vertices[e.p1].position - (vec3)vertices[e.p1].prev_position;
        vec3 deltav_dt = v1_dt - v0_dt;
        if (delta_x.squared() > 0.00001f) {
          vec3 dir = normalize(delta_x);
          float len = delta_x.length();
          float x = len - e.natural_length;
          float fx = x * e.spring_const;
          vec3 spring_force = dir * fx;
          vec3 damping_force;// = deltav_dt * (rdelta_t * e.damping_const);
          float spe = (0.5f * x) * fx;
          total_pe += spe;
          vertices[e.p0].force = (vec3)vertices[e.p0].force + spring_force; // - damping_force;
          vertices[e.p1].force = (vec3)vertices[e.p1].force - spring_force; // - damping_force;

          if (debug) {
            char tmp[10][256];
            log("  e %d %d len=%f nlen=%f spe=%f dir=%s fs=%s fd=%s\n", e.p0, e.p1, len, e.natural_length, spe,
              dir.toString(tmp[1], 256), spring_force.toString(tmp[2], 256), damping_force.toString(tmp[3], 256)
            );
          }
        }
      }

      // update velocities and position
      float total_ke = 0;
      for (size_t i = 0; i != vertices.size(); ++i) {
        vertex &v = vertices[i];
        // F = ma
        vec3 accel = v.force * v.inv_mass;
        // http://en.wikipedia.org/wiki/Verlet_integration
        vec3 new_pos = (vec3)v.position + (vec3)v.position - (vec3)v.prev_position + accel * (delta_t * delta_t);
        vec3 vel = (new_pos - (vec3)v.prev_position) * (rdelta_t * 0.5f);
        v.prev_position = v.position;
        v.position = new_pos;
        total_ke += vel.squared() * (v.gravity_mass * 0.5f);
        float ke = vel.squared() * (v.gravity_mass * 0.5f);
        float pe = dot(-(vec3)v.position, gravity) * v.gravity_mass;
        if (debug) {
          char tmp[4][256];
          log("v%d ke=%f gpe=%f p=%s pp=%s a=%s f=%s %f\n", (int)i, ke, pe, vec3(v.position).toString(tmp[0], 64), vec3(v.prev_position).toString(tmp[1], 64), vec3(accel).toString(tmp[2], 64), vec3(v.force).toString(tmp[3], 64), accel.y());
        }
        v.normal = vec3(0);
      }
      if (debug) {
        log("te=%f ke=%f pe=%f\n", total_ke + total_pe, total_ke, total_pe);
      }
    }

    void build_mesh() {
      // update normals (in tmp)
      for (size_t i = 0; i != faces.size(); ++i) {
        face &f = faces[i];
        vec3 p0 = vertices[f.p0].position;
        vec3 p1 = vertices[f.p1].position;
        vec3 p2 = vertices[f.p2].position;
        vec3 normal = cross(p1-p0, p2-p0);
        vertices[f.p0].normal = (vec3)vertices[f.p0].normal + normal;
        vertices[f.p1].normal = (vec3)vertices[f.p1].normal + normal;
        vertices[f.p2].normal = (vec3)vertices[f.p2].normal + normal;
        /*char normal[256];
        log("%d %d %d %s\n", f.p0, f.p1, f.p2, normal.toString(normal, 256));
        if (f.p0 == 4 ) log("v%d %s\n", f.p0, vertices[f.p0].normal.toString(tmp, 256));
        if (f.p1 == 4 ) log("v%d %s\n", f.p1, vertices[f.p1].normal.toString(tmp, 256));
        if (f.p2 == 4 ) log("v%d %s\n", f.p2, vertices[f.p2].normal.toString(tmp, 256));
        */
      }

      // build the geometry
      gl_resource::wolock vtx_lock(msh->get_vertices());
      float *dest = vtx_lock.f32();
      for (size_t i = 0; i != vertices.size(); ++i) {
        vertex &v = vertices[i];
        vec3 normal = normal.squared() < 0.0001 ? vec3(1, 0, 0) : normalize(v.normal);
        vec3 position = v.position;
        //char tmp[256];
        //log("v%d %s\n", i, normal.toString(tmp, 256));
        dest[0] = position.x();
        dest[1] = position.y();
        dest[2] = position.z();
        dest[3] = normal.x();
        dest[4] = normal.y();
        dest[5] = normal.z();
        dest[6] = 0;
        dest[7] = 0;
        dest += 8;
      }
    }

    //
    void make_edge(int p000, int p010) {
      assert(p000 != p010);
      edge e;
      e.p0 = std::min(p000, p010);
      e.p1 = std::max(p000, p010);
      e.natural_length = ((vec3)vertices[p000].position - (vec3)vertices[p010].position).length();
      e.spring_const = spring_const;
      e.damping_const = damping_const;
      edges.push_back(e);
    }

    // make a face, sorting the indices, but recording if we switch the winding order.
    void make_face(int a, int b, int c) {
      face f;
      /*
      // ignore this for now... it was impossible to remove crossed faces between cubes.
      // it might be worth changing the generation order for adjacent cubes, however
      // and putting this back in.
      int winding = 0;
      //if (a == 1 || b == 1 || c == 1) log("make_face %d %d %d ", a, b, c);
      if (b < a) { std::swap(a, b); winding ^= 1; }
      if (c < a) { std::swap(a, c); winding ^= 1; }
      if (c < b) { std::swap(b, c); winding ^= 1; }
      //if (a == 1 || b == 1 || c == 1) log(" ... %d %d %d\n", a, b, c);
      */
      f.p0 = a;
      f.p1 = b;
      f.p2 = c;
      //f.winding = winding;
      faces.push_back(f);
    }

    // b
    // a c
    //           d
    void make_tetra(int a, int b, int c, int d) {
      make_edge(a, b);
      make_edge(a, c);
      make_edge(a, d);
      make_edge(b, c);
      make_edge(b, d);
      make_edge(c, d);

      /*make_face(a, b, c);
      make_face(a, c, d);
      make_face(a, d, b);
      make_face(b, d, c);*/
    }

    // p010 p011
    // p000 p001
    //           p110 p111
    //           p100 p101
    // see http://www.youtube.com/watch?v=XAhgw2Z4T2M
    void make_cube(int p000, int p001, int p010, int p011, int p100, int p101, int p110, int p111, unsigned mask) {
      // we could just add edges here, but we may want to visualise the tetras.
      // note that it is unwise to add extra springs just for badness!
      // we could also simulate deformation of the tetras without using springs, for example
      // preserving volume.
      make_tetra(p000, p010, p001, p100); // <- corner 000
      make_tetra(p001, p111, p101, p100); // <- corner 101
      make_tetra(p001, p011, p111, p010); // <- corner 011
      make_tetra(p001, p010, p111, p100); // <- centre piece
      make_tetra(p010, p111, p100, p110); // <- corner 110

      // the mask selects rendering geometry: only on outside faces.
      if (mask & 0x01) { make_face(p000, p010, p001); make_face(p001, p010, p011); }
      if (mask & 0x02) { make_face(p000, p001, p100); make_face(p001, p101, p100); }
      if (mask & 0x04) { make_face(p000, p100, p010); make_face(p100, p110, p010); }
      if (mask & 0x08) { make_face(p100, p110, p101); make_face(p101, p110, p111); }
      if (mask & 0x10) { make_face(p010, p011, p110); make_face(p011, p111, p110); }
      if (mask & 0x20) { make_face(p001, p101, p011); make_face(p101, p111, p011); }
    }

    // we are likely to have a number of duplicate edges. sort and remove duplicates.
    void remove_duplicate_edges() {
      struct { bool operator()(const edge &a, const edge &b) { return a.p0 == b.p0 ? a.p1 < b.p1 : a.p0 < b.p0; } } pred;
      std::sort(edges.data(), edges.data() + edges.size(), pred);

      struct { bool operator()(const edge &a, const edge &b) { return a.p0 == b.p0 && a.p1 == b.p1; } } pred2;
      edge *p = std::unique(edges.data(), edges.data() + edges.size(), pred2);

      edges.resize(p - edges.data());
    }

    /*void remove_internal_faces() {
      struct { bool operator()(const face &a, const face &b) { return a.p0 == b.p0 ? ( a.p1 == b.p1 ? a.p2 < b.p2 : a.p1 < b.p1 ) : a.p0 < b.p0; } } pred;
      std::sort(faces.data(), faces.data() + faces.size(), pred);

      struct { bool operator()(const face &a, const face &b) { return a.p0 == b.p0 && a.p1 == b.p1 && a.p2 == b.p2; } } pred2;

      face *src = faces.data(), *dest = src, *end = src + faces.size();
      while (src < end) {
        if (src < end-1 && pred2(src[0], src[1])) {
          do {
            ++src;
          } while (src < end-1 && pred2(src[0], src[1]));
          ++src;
        } else {
          *dest++ = *src++;
        }
      }

      faces.resize(dest - faces.data());

      for (size_t i = 0; i != faces.size(); ++i) {
        face &f = faces[i];
        // preserve winding order of the face
        if (f.winding) std::swap(f.p1, f.p2);
      }
    }*/
  };
}
