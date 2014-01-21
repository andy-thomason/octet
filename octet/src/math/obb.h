////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Axis aligned bounding box
//

namespace octet { namespace math {
  // Orientated bounding box.
  class obb {
    vec3 center;        // world space center
    vec3 half_extent;   // object space half extents
    vec3 axes[3];       // world space axes

    enum { debug = 0 };
  public:
    obb() {
    }

    // construct from aabb and matrix.
    obb(aabb_in bb_, mat4t_in mat) {
      axes[0] = mat[0].xyz();
      axes[1] = mat[1].xyz();
      axes[2] = mat[2].xyz();
      half_extent = bb_.get_half_extent();
      center = bb_.get_center() * mat;
      //char tmp[256];
      //log("mat.w() = %s\n", mat.w().toString(tmp, sizeof(tmp)));
      //log("center = %s\n", center.toString(tmp, sizeof(tmp)));
    }

    // construct from center, extents and matrix.
    obb(vec3_in center_, vec3_in half_extent_, mat4t_in mat) {
      axes[0] = mat[0].xyz();
      axes[1] = mat[1].xyz();
      axes[2] = mat[2].xyz();
      half_extent = half_extent_;
      center = center_ * mat;
    }

    const vec3 get_min() const {
      const vec3 &h = half_extent;
      vec3 half = abs(axes[0] * h.xxx()) + abs(axes[1] * h.yyy()) + abs(axes[2] * h.zzz());
      return center - half;
    }

    const vec3 get_max() const {
      const vec3 &h = half_extent;
      vec3 half = abs(axes[0] * h.xxx()) + abs(axes[1] * h.yyy()) + abs(axes[2] * h.zzz());
      return center + half;
    }

    const vec3 get_center() const {
      return center;
    }

    const vec3 get_half_extent() const {
      return half_extent;
    }

    const char *toString(char *dest, size_t len) const {
      char tmp[5][64];
      snprintf(dest, len, "[obb %s %s %s %s %s]",
        center.toString(tmp[0], sizeof(tmp[0])),
        half_extent.toString(tmp[1], sizeof(tmp[1])),
        axes[0].toString(tmp[2], sizeof(tmp[2])),
        axes[1].toString(tmp[3], sizeof(tmp[3])),
        axes[2].toString(tmp[4], sizeof(tmp[4]))
      );
      return dest;
    }

    bool intersects(const vec3 &rhs) const {
      vec3 diff = abs(center - rhs);
      vec3 proj(dot(axes[0], diff), dot(axes[1], diff), dot(axes[2], diff));
      return all(proj <= half_extent);
    }

    // much of the function below is derived from:
    // http://www.jkh.me/files/tutorials/Separating%20Axis%20Theorem%20for%20Oriented%20Bounding%20Boxes.pdf
    //
    bool intersects(const obb &b) const {
      const obb &a = *this;
      vec3 diff = a.center - b.center;

      if (debug) {
        char tmp[256];
        //log("intersects: diff=%s\n", diff.toString(tmp, sizeof(tmp)));
        log("intersects: a=%s\n", a.toString(tmp, sizeof(tmp)));
        log("intersects: b=%s\n", b.toString(tmp, sizeof(tmp)));
      }

      const vec3 &ax = a.axes[0], &ay = a.axes[1], &az = a.axes[2];
      const vec3 &bx = b.axes[0], &by = b.axes[1], &bz = b.axes[2];
      const vec3 &ah = a.half_extent;
      const vec3 &bh = b.half_extent;

      // todo: use transposed axes. this will make this more computationally efficient.
      // diff projected onto a and diff projected onto b
      vec3 da(dot(diff, ax), dot(diff, ay), dot(diff, az));
      vec3 db(dot(diff, bx), dot(diff, by), dot(diff, bz));

      // axes of b projected onto axes of b
      vec3 r_x(dot(ax,bx), dot(ay,bx), dot(az,bx));
      vec3 r_y(dot(ax,by), dot(ay,by), dot(az,by));
      vec3 r_z(dot(ax,bz), dot(ay,bz), dot(az,bz));

      // separate faces of a from corners of b and vice versa
      bvec3 afaces = abs(da) > ah + bh.xxx() * abs(r_x) + bh.yyy() * abs(r_y) + bh.zzz() * abs(r_z);
      bvec3 bfaces = abs(db) > bh + ah.xxx() * abs(r_x) + ah.yyy() * abs(r_y) + ah.zzz() * abs(r_z);
      if (any(afaces | bfaces)) return false;

      // axes of a projected onto axes of a
      vec3 rx(r_x.x(), r_y.x(), r_z.x());
      vec3 ry(r_x.y(), r_y.y(), r_z.y());
      vec3 rz(r_x.z(), r_y.z(), r_z.z());

      // separate edges of a from edges of b
      bvec3 xedges = abs(da.zzz() * ry - da.yyy() * rz) > ah.yyy() * abs(rz) + ah.zzz() * abs(ry) + abs_cross(bh, abs(rx));
      bvec3 yedges = abs(da.xxx() * rz - da.zzz() * rx) > ah.xxx() * abs(rz) + ah.zzz() * abs(rx) + abs_cross(bh, abs(ry));
      bvec3 zedges = abs(da.yyy() * rx - da.xxx() * ry) > ah.xxx() * abs(ry) + ah.yyy() * abs(rx) + abs_cross(bh, abs(rz));
      return !any(xedges | yedges | zedges);
    }
  };

  #if OCTET_UNIT_TEST
    class obb_unit_test {
    public:
      obb_unit_test() {
        {
          obb a(vec3(0, 0, 0), vec3(1, 1, 1), mat4t());

          obb b(vec3(0, 0, 0), vec3(1, 1, 1), mat4t());
          assert(a.intersects(b));
          obb c(vec3(1, 0, 0), vec3(1, 1, 1), mat4t());
          assert(a.intersects(c));
          obb d(vec3(2, 0, 0), vec3(1, 1, 1), mat4t());
          assert(!a.intersects(d));
        }
      }
    };
    static obb_unit_test obb_unit_test;
  #endif
} }
