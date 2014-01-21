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
  };
} }

