////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Axis aligned bounding box
//

namespace octet { namespace math {
  // Z-Axis aligned cylinder
  class zcylinder {
    // we store aabbs this way because it makes ray tests and other operations much simpler.
    vec3 center;
    float half_extent;
    float radius;
  public:
    // Constructor, given a center and size
    zcylinder(vec3_in center_=vec3(0, 0, 0), float radius_=1, float half_extent_=1) {
      center = center_;
      half_extent = half_extent_;
      radius = radius_;
    }

    aabb get_aabb() const {
      return aabb(center, vec3(radius, radius, half_extent));
    }

    float get_radius() const {
      return radius;
    }

    float get_half_extent() const {
      return half_extent;
    }

    // Get a string representation of the object.
    // Requires a buffer (dest, len)
    const char *toString(char *dest, size_t len) const {
      char tmp[2][64];
      snprintf(dest, len, "[zcylinder: %s, %f, %f]", center.toString(tmp[0], sizeof(tmp[0])), half_extent, radius);
      return dest;
    }

    template <class sink_t> void get_geometry(sink_t &sink, int steps) const {
      sink.reserve(steps * 4, steps * 6 + (steps - 2) * 6);

      float k = (3.14159265f * 2) / steps;
      float ku = 1.0f / steps;
      for (unsigned i = 0; i != steps; ++i) {
        float c = cosf(i * k);
        float s = sinf(i * k);
        sink.add_vertex(center + vec3(c*radius, s*radius, -half_extent), vec3(c, s, 0), vec3(i * ku, 0, 0));
        sink.add_vertex(center + vec3(c*radius, s*radius,  half_extent), vec3(c, s, 0), vec3(i * ku, 1, 0));
        sink.add_vertex(center + vec3(c*radius, s*radius, -half_extent), vec3(0, 0, -1), vec3(c, s, -1));
        sink.add_vertex(center + vec3(c*radius, s*radius,  half_extent), vec3(0, 0,  1), vec3(c, s,  1));
      }

      // sides
      for (unsigned i = 0; i != steps; ++i) {
        unsigned i0 = i*4;
        unsigned i1 = (i+1 == steps) ? 0*4 : (i+1)*4;
        sink.add_triangle(i0+0, i1+0, i1+1);
        sink.add_triangle(i0+0, i1+1, i0+1);
      }

      // bottom
      for (unsigned i = 0; i != steps-2; ++i) {
        sink.add_triangle(2, i*4+6, i*4+10);
      }

      // top
      for (unsigned i = 0; i != steps-2; ++i) {
        sink.add_triangle(3, i*4+7, i*4+11);
      }
    }
  };
} }

