////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Axis aligned bounding box
//

namespace octet { namespace math {
  // Axis aligned bounding box. Used to find the size and location of objects.
  class aabb {
    // we store aabbs this way because it makes ray tests and other operations much simpler.
    vec3 center;
    vec3 half_extent;
  public:
    // Default constructor: an empty bounding box
    aabb() {
      center = vec3(0, 0, 0);
      half_extent = vec3(0, 0, 0);
    }

    // Constructor, given a center and size
    aabb(const vec3 &center_, const vec3 &half_extent_) {
      center = center_;
      half_extent = half_extent_;
    }

    template <class _It> aabb(_It a, _It b) {
      if (a != b) {
        vec3 min3 = *a, max3 = *a;
        std::for_each(a+1, b, [&](vec3_in v){ min3 = math::min(min3, v); max3 = math::max(max3, v); });
        center = (max3 + min3) * 0.5f;
        half_extent = (max3 - min3) * 0.5f;
      }
    }

    // Find the union of two axis aligned bounding boxes
    aabb get_union(const aabb &rhs) {
      vec3 min = get_min().min(rhs.get_min());
      vec3 max = get_max().max(rhs.get_max());
      return aabb(( min + max ) * 0.5f, ( max - min ) * 0.5f);
    }

    // Get least value of x, y and z in the object
    const vec3 get_min() const {
      return center - half_extent;
    }

    // Get highest value of x, y and z in the object
    const vec3 get_max() const {
      return center + half_extent;
    }

    // Get the center of the bounding box
    const vec3 get_center() const {
      return center;
    }

    // Get the distance from the center to the edge of the box in x, y and z
    const vec3 get_half_extent() const {
      return half_extent;
    }

    // Get a transformed object, give a matrix "mat"
    aabb get_transform(const mat4t &mat) const {
      vec3 half =
        half_extent.x() * abs(mat.x().xyz()) +
        half_extent.y() * abs(mat.y().xyz()) +
        half_extent.z() * abs(mat.z().xyz())
      ;
      return aabb((center.xyz1() * mat).xyz(), half);
    }

    // Get a string representation of the object.
    // Requires a buffer (dest, len)
    const char *toString(char *dest, size_t len) const {
      char tmp[2][64];
      snprintf(dest, len, "[%s, %s]", center.toString(tmp[0], sizeof(tmp[0])), half_extent.toString(tmp[1], sizeof(tmp[1])));
      return dest;
    }

    // Return true if this point is in the object.
    bool intersects(const vec3 &rhs) const {
      vec3 diff = abs(center - rhs);
      vec3 limit = half_extent;
      return all(diff <= limit);
    }

    // return true if this AABB intersects the object
    bool intersects(const aabb &rhs) const {
      vec3 diff = abs(center - rhs.center);
      vec3 limit = half_extent + rhs.half_extent;
      return all(diff <= limit);
    }

    // Obb collision test.
    // Deprecated.
    // <a href="http://www.jkh.me/files/tutorials/Separating%20Axis%20Theorem%20for%20Oriented%20Bounding%20Boxes.pdf">based on this</a>
    // Note: this assumes that mxa and mxb are orthonormal.
    // If the matrices have any scale, this will not work.
    // This is equivalent to constructing the minkowski difference
    // ie. add the first cube to all the corners of the second making 30 faces.
    bool intersects_old(const aabb &b, const mat4t &mxa, const mat4t &mxb) const {
      const aabb &a = *this;
      vec3 ca = a.center * mxa;
      vec3 cb = b.center * mxb;
      vec3 diff = cb - ca;

      const vec3 &ax = mxa.x().xyz(), &ay = mxa.y().xyz(), &az = mxa.z().xyz();
      const vec3 &bx = mxb.x().xyz(), &by = mxb.y().xyz(), &bz = mxb.z().xyz();
      //const vec3 &ah = a.half_extent;
      //const vec3 &bh = b.half_extent;

      float dax = dot(diff, ax), day = dot(diff, ay), daz = dot(diff, az);
      float dbx = dot(diff, bx), dby = dot(diff, by), dbz = dot(diff, bz);

      float ahx = a.half_extent.x(), ahy = a.half_extent.y(), ahz = a.half_extent.z();
      float bhx = b.half_extent.x(), bhy = b.half_extent.y(), bhz = b.half_extent.z();

      float rxx = dot(ax,bx), rxy = dot(ax,by), rxz = dot(ax,bz);
      float ryx = dot(ay,bx), ryy = dot(ay,by), ryz = dot(ay,bz);
      float rzx = dot(az,bx), rzy = dot(az,by), rzz = dot(az,bz);
      vec3 r_x(dot(ax,bx), dot(ay,bx), dot(az,bx));
      vec3 r_y(dot(ax,by), dot(ay,by), dot(az,by));
      vec3 r_z(dot(ax,bz), dot(ay,bz), dot(az,bz));
      vec3 rx(r_x.x(), r_y.x(), r_z.x());
      vec3 ry(r_x.y(), r_y.y(), r_z.y());
      vec3 rz(r_x.z(), r_y.z(), r_z.z());

      float axx = abs(rxx), axy = abs(rxy), axz = abs(rxz);
      float ayx = abs(ryx), ayy = abs(ryy), ayz = abs(ryz);
      float azx = abs(rzx), azy = abs(rzy), azz = abs(rzz);

      // faces of a
      int t1 = fgt(abs(dax), ahx + bhx * axx + bhy * axy + bhz * axz); // L = ax
      int t2 = fgt(abs(day), ahy + bhx * ayx + bhy * ayy + bhz * ayz); // L = ay
      int t3 = fgt(abs(daz), ahz + bhx * azx + bhy * azy + bhz * azz); // L = az

      // faces of b
      int t4 = fgt(abs(dbx), bhx + ahx * axx + ahy * axy + ahz * axz); // L = bx
      int t5 = fgt(abs(dby), bhy + ahx * ayx + ahy * ayy + ahz * ayz); // L = by
      int t6 = fgt(abs(dbz), bhz + ahx * azx + ahy * azy + ahz * azz); // L = bz

      // x edges of a
      int t7 = fgt(abs(daz * ryx - day * rzx),  ahy * azx + ahz * ayx + bhy * axz + bhz * axy); // L = cross(ax, bx)
      int t8 = fgt(abs(daz * ryy - day * rzy),  ahy * azy + ahz * ayy + bhx * axz + bhz * axx); // L = cross(ax, by)
      int t9 = fgt(abs(daz * ryz - day * rzz),  ahy * azz + ahz * ayz + bhx * axy + bhy * axx); // L = cross(ax, bz)

      // y edges of a
      int t10 = fgt(abs(dax * rzx - daz * rxx),  ahx * azx + ahz * axx + bhy * ayz + bhz * ayy); // L = cross(ay, bx)
      int t11 = fgt(abs(dax * rzy - daz * rxy),  ahx * azy + ahz * axy + bhx * ayz + bhz * ayx); // L = cross(ay, by)
      int t12 = fgt(abs(dax * rzz - daz * rxz),  ahx * azz + ahz * axz + bhx * ayy + bhy * ayx); // L = cross(ay, bz)

      // z edges of a
      int t13 = fgt(abs(day * rxx - dax * ryx),  ahx * ayx + ahy * axx + bhy * azz + bhz * azy); // L = cross(az, bx)
      int t14 = fgt(abs(day * rxy - dax * ryy),  ahx * ayy + ahy * axy + bhx * azz + bhz * azx); // L = cross(az, by)
      int t15 = fgt(abs(day * rxz - dax * ryz),  ahx * ayz + ahy * axz + bhx * azy + bhy * azx); // L = cross(az, bz)

      printf("%d %d %d  %d %d %d  %d %d %d  %d %d %d  %d %d %d (%d)\n", t1<0, t2<0, t3<0, t4<0, t5<0, t6<0, t7<0, t8<0, t9<0, t10<0, t11<0, t12<0, t13<0, t14<0, t15 < 0, (t1 & t2 & t3 & t4 & t5 & t6 & t7 & t8 & t9 & t10 & t11 & t12 & t13 & t14 & t15) < 0);

      return (t1 & t2 & t3 & t4 & t5 & t6 & t7 & t8 & t9 & t10 & t11 & t12 & t13 & t14 & t15) < 0;
    }

    // Obb collision test.
    // <a href="http://www.jkh.me/files/tutorials/Separating%20Axis%20Theorem%20for%20Oriented%20Bounding%20Boxes.pdf">based on this</a>
    // Note: this assumes that mxa and mxb are orthonormal.
    // If the matrices have any scale, this will not work.
    // This is equivalent to constructing the minkowski difference
    // ie. add the first cube to all the corners of the second making 30 faces.
    bool intersects(const aabb &b, const mat4t &mxa, const mat4t &mxb) const {
      const aabb &a = *this;
      vec3 ca = a.center * mxa;
      vec3 cb = b.center * mxb;
      vec3 diff = cb - ca;

      const vec3 &ax = mxa.x().xyz(), &ay = mxa.y().xyz(), &az = mxa.z().xyz();
      const vec3 &bx = mxb.x().xyz(), &by = mxb.y().xyz(), &bz = mxb.z().xyz();
      vec3 ah = a.half_extent;
      vec3 bh = b.half_extent;

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

      /*int t1 = afaces[0], t2 = afaces[1], t3 = afaces[2];
      int t4 = bfaces[0], t5 = bfaces[1], t6 = bfaces[2];
      int t7 = xedges[0], t8 = xedges[1], t9 = xedges[2];
      int t10 = yedges[0], t11 = yedges[1], t12 = yedges[2];
      int t13 = zedges[0], t14 = zedges[1], t15 = zedges[2];

      printf("%d %d %d  %d %d %d  %d %d %d  %d %d %d  %d %d %d (%d)\n", t1<0, t2<0, t3<0, t4<0, t5<0, t6<0, t7<0, t8<0, t9<0, t10<0, t11<0, t12<0, t13<0, t14<0, t15 < 0, (t1 & t2 & t3 & t4 & t5 & t6 & t7 & t8 & t9 & t10 & t11 & t12 & t13 & t14 & t15) < 0);

      return (t1 & t2 & t3 & t4 & t5 & t6 & t7 & t8 & t9 & t10 & t11 & t12 & t13 & t14 & t15) < 0;
      */
    }

	  template <class sink_t> void get_geometry(sink_t &sink, int) {
      static const float vertices[4 * 6 * 8] = {
        // front
        #undef OCTET_FACE
        #define OCTET_FACE(X, Y, U, V) X, Y, 1,  0, 0, 1, U, V,
        OCTET_FACE( 1,  1,  1, 1 ) OCTET_FACE( 1, -1,  1, 0 ) OCTET_FACE(-1, -1,  0, 0 ) OCTET_FACE(-1,  1,  0, 1 )

        // back
        #undef OCTET_FACE
        #define OCTET_FACE(X, Y, U, V) -(X), (Y), -1,  0, 0, -1, U, V,
        OCTET_FACE( 1,  1,  1, 1 ) OCTET_FACE( 1, -1,  1, 0 ) OCTET_FACE(-1, -1,  0, 0 ) OCTET_FACE(-1,  1,  0, 1 )

        // top
        #undef OCTET_FACE
        #define OCTET_FACE(X, Y, U, V) Y, 1, X,  0, 1, 0, U, V,
        OCTET_FACE( 1,  1,  1, 1 ) OCTET_FACE( 1, -1,  1, 0 ) OCTET_FACE(-1, -1,  0, 0 ) OCTET_FACE(-1,  1,  0, 1 )

        // bottom
        #undef OCTET_FACE
        #define OCTET_FACE(X, Y, U, V) (Y), -1, -(X),  0, 0, -1, U, V,
        OCTET_FACE( 1,  1,  1, 1 ) OCTET_FACE( 1, -1,  1, 0 ) OCTET_FACE(-1, -1,  0, 0 ) OCTET_FACE(-1,  1,  0, 1 )

        // right
        #undef OCTET_FACE
        #define OCTET_FACE(X, Y, U, V) 1, X, Y,  1, 0, 0, U, V,
        OCTET_FACE( 1,  1,  1, 1 ) OCTET_FACE( 1, -1,  1, 0 ) OCTET_FACE(-1, -1,  0, 0 ) OCTET_FACE(-1,  1,  0, 1 )

        // left
        #undef OCTET_FACE
        #define OCTET_FACE(X, Y, U, V) -1, -(X), (Y),  -1, 0, 0, U, V,
        OCTET_FACE( 1,  1,  1, 1 ) OCTET_FACE( 1, -1,  1, 0 ) OCTET_FACE(-1, -1,  0, 0 ) OCTET_FACE(-1,  1,  0, 1 )

        #undef OCTET_FACE
      };

      // 3 0
      // 2 1
      static const uint32_t indices[6 * 6] = {
        0+4*0, 1+4*0, 3+4*0,  1+4*0, 2+4*0, 3+4*0,
        0+4*1, 1+4*1, 3+4*1,  1+4*1, 2+4*1, 3+4*1,
        0+4*2, 1+4*2, 3+4*2,  1+4*2, 2+4*2, 3+4*2,
        0+4*3, 1+4*3, 3+4*3,  1+4*3, 2+4*3, 3+4*3,
        0+4*4, 1+4*4, 3+4*4,  1+4*4, 2+4*4, 3+4*4,
        0+4*5, 1+4*5, 3+4*5,  1+4*5, 2+4*5, 3+4*5,
      };

      sink.reserve(6 * 4, 6 * 6);

      for (unsigned i = 0; i != 6 * 4; ++i) {
        const float *fs = vertices + i * 8;
        sink.add_vertex(center + vec3(fs[0], fs[1], fs[2]) * half_extent, vec3(fs[3], fs[4], fs[5]), vec3(fs[6], fs[7], 0));
      }

      for (unsigned i = 0; i != 6 * 6; i += 3) {
        sink.add_triangle(indices[i+0], indices[i+1], indices[i+2]);
      }
    }
  };

  std::ostream &operator <<(std::ostream &os, aabb_in rhs) {
    char tmp[256];
    os << rhs.toString(tmp, sizeof(tmp));
    return os;
  }
} }

