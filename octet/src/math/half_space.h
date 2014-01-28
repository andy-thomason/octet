////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// half space in 3d. 
//

namespace octet { namespace math {
  /// half_space: used for simple collision tests
  /// dot(normal, x) + offset >= 0 if point is in the halfspace.
  class half_space : public plane {
  public:
    half_space(vec3_in normal_, float offset_) : plane(normal_, offset_) {
    }

    /// Is point on positive side of plane?
    bool intersects(const vec3 &rhs) const {
      return dot(rhs, get_normal()) + get_offset() >= 0;
    }

    /// Is aabb partly on positive side of plane?
    /// equivalent to a point - halfspace
    bool intersects(const aabb &rhs) const {
      float fatness = sum(abs(get_normal() * rhs.get_half_extent()));
      float distance = dot(get_normal(), rhs.get_center()) + get_offset();
      return distance >= -fatness;
    }

    /// Is sphere partly on positive side of plane?
    /// equivalent to a point - fat plane test
    bool intersects(const sphere &rhs) const {
      float distance = dot(get_normal(), rhs.get_center()) + get_offset();
      return distance >= -rhs.get_radius();
    }

    /// Assuming p0 and p1 are either side of the plane, return the crossing point
    /// such that lerp(result, p0, p1) lies on the plane.
    rational get_clip_point(vec3_in p0, vec3_in p1) const {
      // d0 = p0x * nx + p0y * ny
      //  0 =  px * nx +  py * ny
      // d1 = p1x * nx + p1y * ny
      float dot0 = p0.dot(get_normal());
      float dot1 = p1.dot(get_normal());
      return rational(dot0 + get_offset(), dot0 - dot1);
    }
  };
} }

