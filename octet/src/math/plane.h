////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// plane in 3d space
//

namespace octet { namespace math {
  // plane: used for simple collision tests
  class plane {
    // dot(p, normal) == offset
    vec3 normal;
    float offset;
  public:
    plane(vec3_in normal_=vec3(0, 0, 1), float offset_=0) {
      normal = normal_;
      offset = offset_;
    }

    vec3_ret get_normal() const {
      return normal;
    }

    float get_offset() const {
      return offset;
    }

    const vec3 get_min() const {
      return vec3(-1e37f, -1e37f, -1e37f);
    }

    const vec3 get_max() const {
      return vec3(1e37f, 1e37f, 1e37f);
    }

    // get transformed plane assuming mat is orthonormal.
    plane get_transform(const mat4t &mat) const {
      // dot(p, normal) == offset
      // dot(
      //vec3 n(dot(mat.x().xyz(), normal), dot(mat.y().xyz(), normal), dot(mat.z().xyz(), normal));
      return plane();
    }

    const char *toString(char *dest, size_t len) const {
      char tmp[64];
      snprintf(dest, len, "[%s, %f]", normal.toString(tmp, sizeof(tmp)), offset);
      return dest;
    }

    // point and plane
    bool intersects(const vec3 &rhs) const {
      return dot(rhs, normal) + offset == 0; // really this is nearly always false. Perhaps you need a half space?
    }

    // equivalent to a point - fat plane test
    bool intersects(const aabb &rhs) const {
      float fatness = sum(abs(normal * rhs.get_half_extent()));
      float distance = abs(dot(normal, rhs.get_center()) + offset);
      return distance <= fatness;
    }

    // equivalent to a point - fat plane test
    bool intersects(const sphere &rhs) const {
      float distance = abs(dot(normal, rhs.get_center()) + offset);
      return distance <= rhs.get_radius();
    }

    void flip() {
      normal = -normal;
      offset = -offset;
    }
  };
} }

