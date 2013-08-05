////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Axis aligned bounding box
//

namespace octet {
  // Axis aligned bounding box. Used to find the size and location of objects.
  class aabb {
    // we store aabbs this way because it makes ray tests and other operations much simpler.
    vec4 center;
    vec4 half_extent;
  public:
    aabb() {
      center = vec4(0, 0, 0, 1);
      half_extent = vec4(0, 0, 0, 0);
    }

    aabb(const vec4 &center_, const vec4 &half_extent_) {
      center = center_.xyz1();
      half_extent = half_extent_.xyz();
    }

    // find the union of two axis aligned bounding boxes
    aabb get_union(const aabb &rhs) {
      vec4 min = get_min().min(rhs.get_min());
      vec4 max = get_max().max(rhs.get_max());
      return aabb(( min + max ) * 0.5f, ( max - min ) * 0.5f);
    }

    const vec4 get_min() const {
      return center - half_extent;
    }

    const vec4 get_max() const {
      return center + half_extent;
    }

    const vec4 get_center() const {
      return center;
    }

    const vec4 get_half_extent() const {
      return half_extent;
    }

    aabb get_transform(const mat4t &mat) const {
      return aabb(center * mat, half_extent * mat);
    }

    const char *toString() const {
      static char tmp[256];
      sprintf(tmp, "[%s, %s]", center.toString(), half_extent.toString());
    }

    bool intersects(const aabb &rhs) const {
      vec4 diff = abs(center - rhs.center);
      vec4 limit = half_extent + rhs.half_extent;
      return all(diff <= limit);
    }

    //bool intersects(const ray &rhs) {
    //}
  };
}

