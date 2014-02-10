////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Sphere in 3d space
//

namespace octet { namespace math {
  /// Polygon: one polygon in 3d: vertices in a loop.
  class polygon {
    dynarray<vec3p> vertices;
    unsigned ref_count;
  public:
    /// construct a new polygon with optional capacity
    polygon(int capacity=16) {
      vertices.reserve(capacity);
      ref_count = 0;
    }

    /// clear the polygon.
    void reset() {
      vertices.resize(0);
    }

    /// add a vertex to the polygon.
    void add_vertex(vec3_in vtx) {
      vertices.push_back((vec3p)vtx);
    }

    /// generate a polygon clipped by a half-space.
    void clip(polygon &result, half_space_in half) const {
      if (&result == this) return;

      result.reset();

      if (vertices.size()) {
        vec3 p0 = vertices[vertices.size()-1];
        bool p0_in = half.intersects(p0);
        for (unsigned i = 0; i < vertices.size(); ++i) {
          vec3 p1 = vertices[i];
          bool p1_in = half.intersects(p1);
          if (p0_in != p1_in) {
            rational r = half.get_clip_point(p0, p1);
            vec3 point_on_plane = mix(p0, p1, r);
            //printf("clip=%f pop=%f\n", (float)r, dot(point_on_plane, half.get_normal()) + half.get_offset());
            result.add_vertex(point_on_plane);
          }
          if (p1_in) {
            result.add_vertex(p1);
          }
          p0_in = p1_in;
          p0 = p1;
        }
      }
    }

    /// Grow or shrink a polygon depending on winding order and the up vector.
    void grow(polygon &result, vec3_in up, float offset) const {
      if (&result == this) return;

      result.reset();

      if (vertices.size()) {
        vec3 p0 = vertices[vertices.size()-1];
        for (unsigned i = 0; i < vertices.size(); ++i) {
          vec3 p1 = vertices[i];
          vec3 p2 = vertices[i+1 < vertices.size() ? i+1 : 0];
          vec3 n0 = cross(p1 - p0, up);
          vec3 n1 = cross(p2 - p1, up);
          vec3 normal = normalize(n0 + n1);
          result.add_vertex(p1 + normal * offset);
          p0 = p1;
        }
      }
    }

    /// Give this resource an extra life; see the %ref class.
    void add_ref() {
      ref_count++;
    }

    /// Remove a life from this resource and delete it if it is dead; see the %ref class.
    void release() {
      if (--ref_count == 0) {
        delete this;
      }
    }

    /// get the number of vertices in this polygon
    unsigned get_num_vertices() const {
      return (unsigned)vertices.size();
    }

    /// get an individual vertex.
    vec3 get_vertex(unsigned i) const {
      return (vec3)vertices[i];
    }

    /// calculate the aabb of the polygon
    aabb calc_aabb() const {
      aabb result;
      if (vertices.size()) {
        vec3 vmin = vertices[0];
        vec3 vmax = vmin;
        for (unsigned i = 1; i < vertices.size(); ++i) {
          vec3 pos = vertices[i];
          vmin = min(vmin, pos);
          vmax = max(vmax, pos);
        }
        result = aabb((vmin + vmax) * 0.5f, (vmax - vmin) * 0.5f);
      }
      return result;
    }

  };
} }

