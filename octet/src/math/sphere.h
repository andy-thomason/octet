////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Sphere in 3d space
//

namespace octet { namespace math {
  // Sphere: used for simple collision tests
  class sphere {
    vec3 center;
    float radius;
  public:
    sphere(vec3_in center_=vec3(0, 0, 0), float radius_=1) {
      center = center_;
      radius = radius_;
    }

    vec3_ret get_center() const {
      return center;
    }

    float get_radius() const {
      return radius;
    }

    vec3 get_min() const {
      return center - radius;
    }

    vec3 get_max() const {
      return center + radius;
    }

    aabb get_aabb() const {
      return aabb(center, vec3(radius));
    }

    // get transformed sphere assuming mat is orthonormal.
    sphere get_transform(const mat4t &mat) const {
      return sphere(get_center(), get_radius());
    }

    const char *toString(char *dest, size_t len) const {
      static char tmp[64];
      snprintf(dest, len, "[%s, %f]", center.toString(tmp, sizeof(tmp)), radius);
      return dest;
    }

    // point and sphere
    bool intersects(const vec3 &rhs) const {
      //printf("%f %f %f %f %d\n", center.x(), center.y(), center.z(), radius, squared(rhs - get_center()) <= squared(get_radius()));
      return squared(rhs - get_center()) <= squared(get_radius());
    }

    // equivalent to point and rounded corner box.
    bool intersects(const aabb &rhs) const {
      vec3 diff = abs(get_center() - rhs.get_center());
      vec3 closest = min(diff, rhs.get_half_extent());
      float d2 = squared(get_center() - closest);
      return d2 <= squared(get_radius());
    }

    // equivalent to point and large sphere
    bool intersects(const sphere &rhs) const {
      float d2 = squared(get_center() - rhs.get_center());
      return d2 <= squared(get_radius() + rhs.get_radius());
    }


    template <class sink_t> void get_geometry(sink_t &sink, int max_level) const {
      static const float phi = 1.61803f;
      static const float icosahedron[] = {
        0, +1, +phi,    0, -1, +phi,    0, +1, -phi,    0, -1, -phi,
        +1, +phi, 0,    -1, +phi, 0,    +1, -phi, 0,    -1, -phi, 0,
        +phi, 0, +1,    +phi, 0, -1,    -phi, 0, +1,    -phi, 0, -1,
      };

      static uint8_t icosahedron_indices[] = {  
        0,  8,  1,      0,  1, 10,      0,  4,  8,      0,  5,  4,  
        0, 10,  5,      2, 11,  3,      2,  5, 11,      2,  4,  5,  
        2,  9,  4,      2,  3,  9,      1,  7, 10,      1,  6,  7,  
        1,  8,  6,      3,  6,  9,      3,  7,  6,      3, 11,  7,  
        4,  9,  8,      5, 10, 11,      6,  8,  9,      7, 11, 10
      };
      add_shape(sink, icosahedron, 12, icosahedron_indices, 20, max_level);
    }

  private:
    template <class sink_t> void add_triangle(sink_t &sink, int a, int b, int c, int level, int max_level) const {
      //log("add_triangle %d %d %d %d\n", a, b, c, level);
      if (level == max_level) {
        sink.add_triangle(a, b, c);
      } else {
        //     a
        //     /\
        //    /__\
        //   /_\/_\
        // c        b
        vec3 pa = sink.get_vertex(a).pos;
        vec3 pb = sink.get_vertex(b).pos;
        vec3 pc = sink.get_vertex(c).pos;

        vec3 ab = normalize((pa + pb) * 0.5f);
        vec3 bc = normalize((pb + pc) * 0.5f);
        vec3 ca = normalize((pc + pa) * 0.5f);

        int nab = (int)sink.add_vertex(pos(ab), ab, uv(ab));
        int nbc = (int)sink.add_vertex(pos(bc), bc, uv(bc));
        int nca = (int)sink.add_vertex(pos(ca), ca, uv(ca));

        add_triangle(sink, a, nab, nca, level+1, max_level);
        add_triangle(sink, b, nbc, nab, level+1, max_level);
        add_triangle(sink, c, nca, nbc, level+1, max_level);
        add_triangle(sink, nab, nbc, nca, level+1, max_level);
      }
    }

    vec3 pos(vec3_in normal) const {
      return normal * radius;
    }

    static vec3 uv(vec3_in normal) {
      // mercatoresque projection.
      return vec3(atan2(normal.x(), normal.z())*0.1591549f+0.5f, (normal.y()+1)*0.5f, 0 );
    }

    template <class sink_t> void add_shape(sink_t &sink, const float *vertices, unsigned nv, const uint8_t *indices, unsigned ni, int max_level) const {
      unsigned tot_ni = 3*ni << (max_level*2);
      unsigned tot_nv = nv;
      for (int i = 0; i < max_level; ++i) {
        tot_nv += (ni*3) << (i*2);
      }

      sink.reserve(tot_nv, tot_ni);

      int ix = 0, vx = 0;
      for (unsigned i = 0; i != nv; ++i) {
        vec3 n = normalize(vec3(vertices[0], vertices[1], vertices[2]));
        sink.add_vertex(pos(n), n, uv(n));
        vertices += 3;
      }

      for (unsigned i = 0; i != ni; ++i) {
        add_triangle(sink, indices[0], indices[1], indices[2], 0, max_level);
        indices += 3;
      }
    }

  };
} }

