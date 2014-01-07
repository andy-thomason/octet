////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Axis aligned bounding box
//

namespace octet {
  // Orientated bounding box.
  class obb {
    vec3 center;        // world space center
    vec3 half_extent;   // object space half extents
    vec3 axes[3];       // world space axes
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
      char tmp[5][32];
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

      const vec3 &ax = a.axes[0], &ay = a.axes[1], &az = a.axes[2];
      const vec3 &bx = b.axes[0], &by = b.axes[1], &bz = b.axes[2];
      const vec3 &ah = a.half_extent;
      const vec3 &bh = b.half_extent;

      // todo: use transposed axes
      vec3 da(dot(diff, ax), dot(diff, ay), dot(diff, az));
      vec3 db(dot(diff, bx), dot(diff, by), dot(diff, bz));

      // axx * bxx + axy * bxy + axz * bxz
      vec3 r_x(dot(ax,bx), dot(ay,bx), dot(az,bx));
      vec3 r_y(dot(ax,by), dot(ay,by), dot(az,by));
      vec3 r_z(dot(ax,bz), dot(ay,bz), dot(az,bz));

      bvec3 afaces = abs(da) > half_extent + bh.xxx() * abs(r_x) + bh.yyy() * abs(r_y) + bh.zzz() * abs(r_z);
      bvec3 bfaces = abs(db) > b.half_extent + ah.xxx() * abs(r_x) + ah.yyy() * abs(r_y) + ah.zzz() * abs(r_z);
      if (!all(afaces & bfaces)) return false;

      vec3 rx(r_x.x(), r_y.x(), r_z.x());
      vec3 ry(r_x.y(), r_y.y(), r_z.y());
      vec3 rz(r_x.z(), r_y.z(), r_z.z());

      bvec3 xedges = abs(da.zzz() * ry - da.yyy() * rz) > ah.yyy() * abs(rz) + ah.zzz() * abs(ry) + abs_cross(bh, abs(rx));
      bvec3 yedges = abs(da.xxx() * rz - da.zzz() * rx) > ah.xxx() * abs(rz) + ah.zzz() * abs(rx) + abs_cross(bh, abs(ry));
      bvec3 zedges = abs(da.yyy() * rx - da.xxx() * ry) > ah.xxx() * abs(ry) + ah.yyy() * abs(rx) + abs_cross(bh, abs(rz));
      return all(xedges & yedges & zedges);
    }
  };
}
