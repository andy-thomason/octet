////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// half space in 3d. 
//

namespace octet { namespace math {
  // half_space: used for simple collision tests
  class half_space : public plane {
  public:
    half_space(vec3_in normal_=vec3(0, 0, 1), float offset_=0) : plane(normal_, offset_) {
    }

    // point and plane
    bool intersects(const vec3 &rhs) const {
      return dot(rhs, get_normal()) >= get_offset();
    }

    // equivalent to a point - halfspace
    bool intersects(const aabb &rhs) const {
      float fatness = sum(abs(get_normal() * rhs.get_half_extent()));
      float distance = dot(get_normal(), rhs.get_center()) - get_offset();
      return distance >= -fatness;
    }

    // equivalent to a point - fat plane test
    bool intersects(const sphere &rhs) const {
      float distance = dot(get_normal(), rhs.get_center()) - get_offset();
      return distance >= -rhs.get_radius();
    }
  };
} }

