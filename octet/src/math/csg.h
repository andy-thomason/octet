////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Combine two shapes (constructive solid geometry)
//

namespace octet {
  // boolean combination of two shapes
  template <class shapea_t, class shapeb_t, class op_t> class csg {
    const shapea_t &a;
    const shapeb_t &b;
  public:
    csg(const shapea_t &a_in, const shapeb_t &b_in) : a(a_in), b(b_in) {
    }

    const vec3 get_min() const {
      return min(a.get_min(), b.get_min());
    }

    const vec3 get_max() const {
      return max(a.get_max(), b.get_max());
    }

    const char *toString(char *dest, size_t len) const {
      char tmp[128];
      char tmp2[128];
      snprintf(dest, len, "[%s x %s]", a.toString(tmp, sizeof(tmp)), b.toString(tmp2, sizeof(tmp2)));
      return dest;
    }

    bool intersects(const vec3 &rhs) const {
      return op_t(a.intersects(rhs), b.intersects(rhs));
    }
  };

}

