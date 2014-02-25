////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet { namespace scene {
  /// Particle system: billboards, trails and cloth.
  /// Note all particles in the system must use the same material, but you
  /// can use a custom shader to select different effects.
  class mesh_particle_system : public mesh {
  public:
    /// general particle, billboard, trail, cloth etc.
    struct particle {
      int link;
      vec3p pos;
      particle() {}
    };

    /// traditional sprite-like particle, ie. smoke, fire, sparks etc.
    /// link unused
    struct billboard_particle : particle {
      vec2p size;             /// half-size in world space
      vec2p uv_bottom_left;   /// texture location
      vec2p uv_top_right;     /// texture location
      uint32_t angle;         /// rotation angle 2^32 = 360 degrees
      bool enabled;
      billboard_particle() {}
    };

    /// trail-like particle, tyre streaks, missile trails, lasers, volumetric lights, hair etc.
    /// link points to previous particle in trail.
    struct trail_particle : particle {
      vec3p axis;
      float size;
      vec2p uv_top;
      vec2p uv_bottom;
    };

    /// cloth-like particle
    /// link points to left hand particle (x), y_link points to particle below (y).
    struct cloth_particle : particle {
      int y_link;
      vec2p uv;
    };

    /// animator for particles
    /// link points to particle
    struct particle_animator {
      int link;
      vec3p vel;
      vec3p acceleration;
      uint32_t lifetime;      /// time to live in frames
      uint32_t age;           /// how many frames has this particle lived;
      uint32_t spin;
    };

    /// animator for cloth particles
    /// left, bottom link to other particle animators
    struct cloth_particle_animator : particle_animator {
      int left;
      int bottom;
      float mass;
      vec2p angle_stiffness;
      vec2p spacing_stiffness;
      vec2p spacing;
    };

    /// sphere collider, used to prevent penetration of particles
    struct sphere_collider {
      float restitution;
      float friction;
      sphere geom;
    };
  private:

    // POD (plain-old-data) structure dynarray of camera-facing particles
    dynarray<billboard_particle> billboard_particles;
    int free_billboard_particle;

    // POD structure dynarray of trail particles.
    dynarray<trail_particle> trail_particles;
    int free_trail_particle;

    // POD structure dynarray of animators for particles.
    dynarray<particle_animator> particle_animators;
    int free_particle_animator;

    // camera matrix
    mat4t cameraToWorld;

    void init(const aabb &size, int bbcap, int tpcap, int pacap) {
      set_default_attributes();
      set_aabb(size);
      billboard_particles.reserve(bbcap);
      trail_particles.reserve(tpcap);
      particle_animators.reserve(pacap);
      free_billboard_particle = -1;
      free_trail_particle = -1;
      free_particle_animator = -1;

      unsigned vsize = (bbcap * 4 + tpcap * 2) * sizeof(vertex);
      unsigned isize = (bbcap * 6 + tpcap * 6) * sizeof(uint32_t);
      mesh::allocate(vsize, isize);
    }

    // pool allocation of particles.
    // note: we won't allocate beyond the capacity
    template <class Type> int allocate(dynarray<Type> &array, int &free) {
      int result = free;
      if (free != -1) {
        free = array[free].link;
      } else if (array.size() < array.capacity()) {
        result = (int)array.size();
        array.resize(result+1);
      }
      return result;
    }

    // return to pool
    template <class Type> void free(dynarray<Type> &array, int &free, int element) {
      array[free].link = free;
      free = element;
    }

  public:
    RESOURCE_META(mesh_particle_system)

    /// Default constructor
    mesh_particle_system(aabb_in size=aabb(vec3(0, 0, 0), vec3(1, 1, 1)), int bbcap=256, int tpcap=256, int pacap=256) {
      init(size, bbcap, tpcap, pacap);
    }

    /// Update the vertices for newtonian physics.
    void animate(float time_step) {
      for (unsigned i = 0; i != particle_animators.size(); ++i) {
        particle_animator &g = particle_animators[i];
        if (g.link >= 0) {
          billboard_particle &p = billboard_particles[g.link];
          if (g.age >= g.lifetime) {
            p.enabled = false;
            free(billboard_particles, free_billboard_particle, g.link);
            g.link = -1;
            free(particle_animators, free_particle_animator, i);
          } else {
            p.pos = (vec3)p.pos + (vec3)g.vel * time_step;
            g.vel = (vec3)g.vel + (vec3)g.acceleration * time_step;
            p.angle += (uint32_t)(g.spin * time_step);
            g.age++;
          }
        }
      }
    }

    /// camera-facing particles need the camera matrix to generate world space geometry.
    void set_cameraToWorld(mat4t_in mx) {
      cameraToWorld = mx;
    }

    /// Generate mesh from particles
    virtual void update() {
      //unsigned np = billboard_particles.size();
      //unsigned vsize = billboard_particles.capacity() * sizeof(vertex) * 4;
      //unsigned isize = billboard_particles.capacity() * sizeof(uint32_t) * 4;

      gl_resource::wolock vlock(get_vertices());
      vertex *vtx = (vertex*)vlock.u8();
      gl_resource::wolock ilock(get_indices());
      uint32_t *idx = ilock.u32();
      unsigned num_vertices = 0;
      unsigned num_indices = 0;

      vec3 cx = cameraToWorld.x().xyz();
      vec3 cy = cameraToWorld.y().xyz();
      vec3p n = cameraToWorld.z().xyz();

      for (unsigned i = 0; i != billboard_particles.size(); ++i) {
        billboard_particle &p = billboard_particles[i];
        if (p.enabled) {
          vec2 size = p.size;
          vec3 dx = size.x() * cx;
          vec3 dy = size.y() * cy;
          vec2 bl = p.uv_bottom_left;
          vec2 tr = p.uv_top_right;
          vec2 tl = vec2(bl.x(), tr.y());
          vec2 br = vec2(tr.x(), bl.y());
          vtx->pos = (vec3)p.pos - dx + dy; vtx->normal = n; vtx->uv = tl; vtx++;
          vtx->pos = (vec3)p.pos + dx + dy; vtx->normal = n; vtx->uv = tr; vtx++;
          vtx->pos = (vec3)p.pos + dx - dy; vtx->normal = n; vtx->uv = br; vtx++;
          vtx->pos = (vec3)p.pos - dx - dy; vtx->normal = n; vtx->uv = bl; vtx++;
          idx[0] = num_vertices; idx[1] = num_vertices+1; idx[2] = num_vertices+2;
          idx[3] = num_vertices; idx[4] = num_vertices+2; idx[5] = num_vertices+3;
          idx += 6;
          num_vertices += 4;
          num_indices += 6;
        }
      }

      set_num_vertices(num_vertices);
      set_num_indices(num_indices);
      //dump(log("mesh\n"));
    }

    /// Add a billboard particle. Returns -1 if capacity reached.
    int add_billboard_particle(const billboard_particle &p) {
      int i = allocate(billboard_particles, free_billboard_particle);
      if (i != -1) {
        billboard_particles[i] = p;
      }
      return i;
    }

    /// Add a particle animator. Returns -1 if capacity reached.
    int add_particle_animator(const particle_animator &p) {
      int i = allocate(particle_animators, free_particle_animator);
      if (i != -1) {
        particle_animators[i] = p;
      }
      return i;
    }

    /// Add a trail particle. Returns -1 if capacity reached.
    int add_trail_particle(const trail_particle &p) {
      int i = allocate(trail_particles, free_trail_particle);
      if (i != -1) {
        trail_particles[i] = p;
      }
      return i;
    }

    billboard_particle &access_billboard_particle(int i) { return billboard_particles[i]; }
    trail_particle &access_trail_particle(int i) { return trail_particles[i]; }
    particle_animator &access_particle_animator(int i) { return particle_animators[i]; }

    /// Serialise
    void visit(visitor &v) {
      mesh::visit(v);
      /*
      v.visit(billboard_particles);
      v.visit(free_billboard_particle);
      v.visit(trail_particles);
      v.visit(free_trail_particle);
      v.visit(particle_animators);
      v.visit(free_particle_animator);
      v.visit(cameraToWorld);
      */
    }
  };
}}

