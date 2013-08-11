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
    vec3 center;
    vec3 half_extent;
  public:
    aabb() {
      center = vec3(0, 0, 0);
      half_extent = vec3(0, 0, 0);
    }

    aabb(const vec3 &center_, const vec3 &half_extent_) {
      center = center_;
      half_extent = half_extent_;
    }

    // find the union of two axis aligned bounding boxes
    aabb get_union(const aabb &rhs) {
      vec3 min = get_min().min(rhs.get_min());
      vec3 max = get_max().max(rhs.get_max());
      return aabb(( min + max ) * 0.5f, ( max - min ) * 0.5f);
    }

    const vec3 get_min() const {
      return center - half_extent;
    }

    const vec3 get_max() const {
      return center + half_extent;
    }

    const vec3 get_center() const {
      return center;
    }

    const vec3 get_half_extent() const {
      return half_extent;
    }

    aabb get_transform(const mat4t &mat) const {
      vec3 half =
        half_extent.x() * abs(mat.x().xyz()) +
        half_extent.y() * abs(mat.y().xyz()) +
        half_extent.z() * abs(mat.z().xyz())
      ;
      return aabb((center.xyz1() * mat).xyz(), half);
    }

    const char *toString() const {
      static char tmp[256];
      sprintf(tmp, "[%s, %s]", center.toString(), half_extent.toString());
      return tmp;
    }

    bool intersects(const vec3 &rhs) const {
      vec3 diff = abs(center - rhs);
      vec3 limit = half_extent;
      return all(diff <= limit);
    }

    bool intersects(const aabb &rhs) const {
      vec3 diff = abs(center - rhs.center);
      vec3 limit = half_extent + rhs.half_extent;
      return all(diff <= limit);
    }
  };
}

