////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Axis aligned bounding box
//

namespace octet { namespace math {
  // ray for casting of
  class ray {
    vec3 distance;
    vec3 origin;
  public:
    ray() {
      origin = vec3(0, 0, 0);
      distance = vec3(0, 0, 0);
    }

    ray(const vec3 &start, const vec3 &end) {
      origin = start;
      distance = end - start;
    }

    aabb get_aabb() const {
      vec3 min_aabb = min(origin, origin + distance);
      vec3 max_aabb = max(origin, origin + distance);
      return aabb((min_aabb+max_aabb)*0.5f, (max_aabb-max_aabb)*0.5f);
    }

    ray get_transform(const mat4t &mat) const {
      return ray((origin.xyz1() * mat).xyz(), (distance.xyz0() * mat).xyz());
    }

    const char *toString(char *dest, size_t len) const {
      static char tmp[64];
      static char tmp2[64];
      snprintf(dest, len, "[%s, %s]", origin.toString(tmp, sizeof(tmp)), distance.toString(tmp2, sizeof(tmp2)));
      return dest;
    }

    // intersection tests
    //
    // these are nearly all equivalent to seeing if the Minkovski
    // difference contains the origin.
    //
    // http://en.wikipedia.org/wiki/Minkowski_sum
    //
    // for example:
    //
    // Minkovski sum of a point and a box is the same box move by the point.
    // Minkovski sum of a sphere and a box is a larger box with rounded corners and edges.
    // Minkovski sum of a ray and a box is two boxes with their vertices connected by the ray.
    //
    // see http://www.codercorner.com/RayAABB.cpp for a similar implementation

    bool intersects(const aabb &rhs) const {
      // test ray against faces
      vec3 start = origin - rhs.get_center();
      vec3 end = origin + distance - rhs.get_center();
      vec3 ray_min = min(start, end);
      vec3 half = rhs.get_half_extent();
      bvec3 can_hit_box = (abs(ray_min) < half) | (start * end < vec3(0, 0, 0));

      // test ray against edges
      vec3 abs_dist = abs(distance);
      vec3 d_cross_s = cross(distance, start);
      vec3 d_cross_h = abs_cross(abs_dist, half);
      bvec3 inside_edges = abs(d_cross_s) <= d_cross_h;

      return all(can_hit_box & inside_edges);
    }
    
    // ray-sphere is the same as capsule-point
    bool intersects(sphere_in rhs) const {
      // todo:
      vec3 diff = origin - rhs.get_center();
      float radius2 = squared(rhs.get_radius());
      //float lambda = dot(diff, distance) / squared(distance);
      //float clamped = min(0.0f, max(lambda, 1.0f));
      //vec3 nearest_point = distance * clamped;
      return squared(diff - distance) < radius2;
    }
    
    // return "a" for origin + distance * a
    rational intersection(const aabb &rhs) const {
      vec3 org = origin - rhs.get_center();
      vec3 half = rhs.get_half_extent();

      // solve: diff + a * distance = +/- half   for each of x, y, z
      // a = ((+/-)half - diff) / distance

      // we want to avoid divide like the plague, so use rational numbers
      vec3 hmo = half - abs(org);
      rational ax = rational(hmo.x(), distance.x());
      rational ay = rational(hmo.y(), distance.y());
      rational az = rational(hmo.z(), distance.z());

      return min(ax, ay, az);
    }

    vec3 get_start() const {
      return origin;
    }

    vec3 get_end() const {
      return origin + distance;
    }

    vec3 get_distance() const {
      return origin + distance;
    }
  };

  static inline void unit_test_ray() {
    aabb my_aabb(vec3(1, 1, 1), vec3(1, 2, 3));
    for (int i = 0; i != 360; i += 30) {
      vec3 dist = vec3(cos(i*(3.131592653f/180))*4.0f, sin(i*(3.131592653f/180))*4.0f, 0.0f);
      vec3 org = vec3(3, 1, 0);
      ray my_ray(org, dist);
      rational pt = my_ray.intersection(my_aabb);
      vec3 ipt = org + dist * pt;
      printf("%d %12.7f [%12.7f %12.7f] [%12.7f %12.7f]\n", i, (float)pt, pt.numer(), pt.denom(), ipt.x(), ipt.y());
    }
  }
} }

