////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Vector class
//

namespace octet {
  class mat4t;

  #if defined( USE_SSE )
  class vec4 {
    static const char *Copyright() { return "Copyright(C) Andy Thomason 2012, 2013"; }
    __m128 v128;
  public:
    vec4() {}
    vec4(__m128 value) { v128 = value; }
    vec4(float x, float y, float z, float w) { v128 = _mm_setr_ps(x, y, z, w); };
    float &operator[](int i) { return v128.m128_f32[i]; }
    const float &operator[](int i) const { return v128.m128_f32[i]; }
    vec4 operator*(float r) const { return vec4(_mm_mul_ps(v128, _mm_set_ss(r))); }
    vec4 operator*(const mat4t &r) const;
    vec4 operator+(const vec4 &r) const { return vec4(_mm_add_ps(v128, r.v128)); }
    vec4 operator-(const vec4 &r) const { return vec4(_mm_sub_ps(v128, r.v128)); }
    vec4 operator*(const vec4 &r) const { return vec4(_mm_mul_ps(v128, r.v128)); }
    vec4 operator-() const { return vec4(_mm_sub_ps(_mm_setzero_ps(), v128)); }
    vec4 &operator+=(const vec4 &r) { v128 = _mm_add_ps(v128, r.v128); return *this; }
    vec4 &operator-=(const vec4 &r) { v128 = _mm_sub_ps(v128, r.v128); return *this; }
    vec4 qconj() const { __m128 n = { -1, -1, -1, 1 }; return vec4(_mm_mul_ps(v128, n)); }
    float dot(const vec4 &r) const { __m128 p = _mm_mul_ps(v128, r.v128); p = _mm_add_ps(p,_mm_shuffle_ps(p, _MM_SHUFFLE(2,3,0,1))); p = _mm_add_ps(p,_mm_shuffle_ps(p, _MM_SHUFFLE(0,2,0,2))); return p.m128_f32[0]; }
    vec4 perspectiveDivide() const { float r = 1.0f / v128.m128_f32[3]; return *this * r; }
    vec4 normalize() { return *this * lengthRecip(); }
    float length() { return sqrtf(dot(*this)); }
    float lengthRecip() { return 1.0f/sqrtf(dot(*this)); }
    float lengthSquared() { return dot(*this); }
    vec4 abs() const { return vec4(fabsf(v[0]), fabsf(v[1]), fabsf(v[2]), fabsf(v[3])); }
    bool operator <(const vec4 &r) { return v[0] < r.v[0] && v[1] < r.v[1] && v[2] < r.v[2] && v[3] < r.v[3]; }
    bool operator <=(const vec4 &r) { return v[0] <= r.v[0] && v[1] <= r.v[1] && v[2] <= r.v[2] && v[3] <= r.v[3]; }
    vec4 xyz() const { return vec4(v[0], v[1], v[2], 0); }
    vec4 qmul(const vec4 &r) const {
      return vec4(
	      v[0] * r.v[3] + v[3] * r.v[0] + v[1] * r.v[2] - v[2] * r.v[1],
		    v[1] * r.v[3] + v[3] * r.v[1] + v[2] * r.v[0] - v[0] * r.v[2],
		    v[2] * r.v[3] + v[3] * r.v[2] + v[0] * r.v[1] - v[1] * r.v[0],
		    v[3] * r.v[3] - v[0] * r.v[0] - v[1] * r.v[1] - v[2] * r.v[2]
      );
    }
    vec4 cross(const vec4 &r) const {
      return vec4(
        v[1] * r.v[2] - v[2] * r.v[1],
	      v[2] * r.v[0] - v[0] * r.v[2],
	      v[0] * r.v[1] - v[1] * r.v[0],
	      0
	    );
    }

    float *get() { return &v[0]; }

    const float *get() const { return &v[0]; }

    /*void dump() const {
      printf("{%.3f, %.3f, %.3f, %.3f}\n", v[0], v[1], v[2], v[3]);
    }*/

    const char *toString() const
    {
      static char buf[4][32];
      static int i = 0;
      char *dest = buf[i++&3];
      sprintf(dest, "{%.3f, %.3f, %.3f, %.3f}", v[0], v[1], v[2], v[3]);
      return dest;
    }
  };

  #else

  class vec4 {
    static const char *Copyright() { return "Copyright(C) Andy Thomason 2011-2013"; }
    float v[4];
  public:
    vec4() {}
    vec4(float x, float y, float z, float w) { v[0] = x; v[1] = y; v[2] = z; v[3] = w; };
    float &operator[](int i) { return v[i]; }
    const float &operator[](int i) const { return v[i]; }
    vec4 operator+(float r) const { return vec4(v[0]+r, v[1]+r, v[2]+r, v[3]+r); }
    vec4 operator-(float r) const { return vec4(v[0]-r, v[1]-r, v[2]-r, v[3]-r); }
    vec4 operator*(float r) const { return vec4(v[0]*r, v[1]*r, v[2]*r, v[3]*r); }
    vec4 operator/(float r) const { float rcp = 1.0f / r; return vec4(v[0]*rcp, v[1]*rcp, v[2]*rcp, v[3]*rcp); }
    vec4 operator*(const mat4t &r) const;
    vec4 operator+(const vec4 &r) const { return vec4(v[0]+r.v[0], v[1]+r.v[1], v[2]+r.v[2], v[3]+r.v[3]); }
    vec4 operator-(const vec4 &r) const { return vec4(v[0]-r.v[0], v[1]-r.v[1], v[2]-r.v[2], v[3]-r.v[3]); }
    vec4 operator*(const vec4 &r) const { return vec4(v[0]*r.v[0], v[1]*r.v[1], v[2]*r.v[2], v[3]*r.v[3]); }
    vec4 operator-() const { return vec4(-v[0], -v[1], -v[2], -v[3]); }
    vec4 &operator+=(const vec4 &r) { v[0] += r.v[0]; v[1] += r.v[1]; v[2] += r.v[2]; v[3] += r.v[3]; return *this; }
    vec4 &operator-=(const vec4 &r) { v[0] -= r.v[0]; v[1] -= r.v[1]; v[2] -= r.v[2]; v[3] -= r.v[3]; return *this; }
    vec4 &operator*=(const vec4 &r) { v[0] *= r.v[0]; v[1] *= r.v[1]; v[2] *= r.v[2]; v[3] *= r.v[3]; return *this; }
    vec4 qconj() const { return vec4(-v[0], -v[1], -v[2], v[3]); }
    float dot(const vec4 &r) const { return v[0] * r.v[0] + v[1] * r.v[1] + v[2] * r.v[2] + v[3] * r.v[3]; }
    float sum() const { return v[0] + v[1] + v[2] + v[3]; }
    vec4 perspectiveDivide() const { float r = 1.0f / v[3]; return vec4(v[0]*r, v[1]*r, v[2]*r, v[3]*r); }
    vec4 normalize() { return *this * lengthRecip(); }
    vec4 min(const vec4 &r) const { return vec4(v[0] < r[0] ? v[0] : r[0], v[1] < r[1] ? v[1] : r[1], v[2] < r[2] ? v[2] : r[2], v[3] < r[3] ? v[3] : r[3]); }
    vec4 max(const vec4 &r) const { return vec4(v[0] >= r[0] ? v[0] : r[0], v[1] >= r[1] ? v[1] : r[1], v[2] >= r[2] ? v[2] : r[2], v[3] >= r[3] ? v[3] : r[3]); }
    float length() { return sqrtf(dot(*this)); }
    float lengthRecip() { return 1.0f/sqrtf(dot(*this)); }
    float squared() { return dot(*this); }
    vec4 abs() const { return vec4(fabsf(v[0]), fabsf(v[1]), fabsf(v[2]), fabsf(v[3])); }
    //bool operator <(const vec4 &r) { return v[0] < r.v[0] && v[1] < r.v[1] && v[2] < r.v[2] && v[3] < r.v[3]; }
    //bool operator <=(const vec4 &r) { return v[0] <= r.v[0] && v[1] <= r.v[1] && v[2] <= r.v[2] && v[3] <= r.v[3]; }
    vec4 xy() const { return vec4(v[0], v[1], 0, 0); }
    vec4 xyz() const { return vec4(v[0], v[1], v[2], 0); }
    vec4 xyz1() const { return vec4(v[0], v[1], v[2], 1); }
    float &x() { return v[0]; }
    float &y() { return v[1]; }
    float &z() { return v[2]; }
    float &w() { return v[3]; }
    float x() const { return v[0]; }
    float y() const { return v[1]; }
    float z() const { return v[2]; }
    float w() const { return v[3]; }

    vec4 qmul(const vec4 &r) const {
      return vec4(
	      v[0] * r.v[3] + v[3] * r.v[0] + v[1] * r.v[2] - v[2] * r.v[1],
		    v[1] * r.v[3] + v[3] * r.v[1] + v[2] * r.v[0] - v[0] * r.v[2],
		    v[2] * r.v[3] + v[3] * r.v[2] + v[0] * r.v[1] - v[1] * r.v[0],
		    v[3] * r.v[3] - v[0] * r.v[0] - v[1] * r.v[1] - v[2] * r.v[2]
      );
    }

    vec4 cross(const vec4 &r) const {
      return vec4(
        v[1] * r.v[2] - v[2] * r.v[1],
	      v[2] * r.v[0] - v[0] * r.v[2],
	      v[0] * r.v[1] - v[1] * r.v[0],
	      0
	    );
    }

    float *get() { return &v[0]; }

    const float *get() const { return &v[0]; }

    /*void dump() const {
      printf("{%.3f, %.3f, %.3f, %.3f}\n", v[0], v[1], v[2], v[3]);
    }*/

    const char *toString() const
    {
      static char buf[4][64];
      static int i = 0;
      char *dest = buf[i++&3];
      sprintf(dest, "{%.3f, %.3f, %.3f, %.3f}", v[0], v[1], v[2], v[3]);
      return dest;
    }
  };
  #endif

  class quat : public vec4
  {
  public:
    quat(float x, float y, float z, float w) : vec4(x, y, z, w) {}
    quat(const vec4 &r) { *(vec4*)this = r; }
    quat operator*(const quat &r) const { return quat(qmul(r)); }
    quat operator*(float r) const { return quat((vec4&)*this * r); }
    quat &operator*=(const quat &r) { *(vec4*)this = qmul(r); return *this; }
    quat conjugate() const { return qconj(); }
    vec4 rotate(const vec4 &r) const { return (*this * r) * conjugate(); }
  };

  class ivec4 {
    int v[4];
  public:
    ivec4() {}

    ivec4(int x, int y, int z, int w) { v[0] = x; v[1] = y; v[2] = z; v[3] = w; };

    ivec4(const vec4 &rhs) {
      v[0] = (int)rhs[0];
      v[1] = (int)rhs[1];
      v[2] = (int)rhs[2];
      v[3] = (int)rhs[3];
    }

    int &operator[](int i) { return v[i]; }
    const int &operator[](int i) const { return v[i]; }
    ivec4 operator+(int r) const { return ivec4(v[0]+r, v[1]+r, v[2]+r, v[3]+r); }
    ivec4 operator-(int r) const { return ivec4(v[0]-r, v[1]-r, v[2]-r, v[3]-r); }
    ivec4 operator*(int r) const { return ivec4(v[0]*r, v[1]*r, v[2]*r, v[3]*r); }
    ivec4 operator+(const ivec4 &r) const { return ivec4(v[0]+r.v[0], v[1]+r.v[1], v[2]+r.v[2], v[3]+r.v[3]); }
    ivec4 operator-(const ivec4 &r) const { return ivec4(v[0]-r.v[0], v[1]-r.v[1], v[2]-r.v[2], v[3]-r.v[3]); }
    ivec4 operator*(const ivec4 &r) const { return ivec4(v[0]*r.v[0], v[1]*r.v[1], v[2]*r.v[2], v[3]*r.v[3]); }
    ivec4 operator-() const { return ivec4(-v[0], -v[1], -v[2], -v[3]); }
    ivec4 &operator+=(const ivec4 &r) { v[0] += r.v[0]; v[1] += r.v[1]; v[2] += r.v[2]; v[3] += r.v[3]; return *this; }
    ivec4 &operator-=(const ivec4 &r) { v[0] -= r.v[0]; v[1] -= r.v[1]; v[2] -= r.v[2]; v[3] -= r.v[3]; return *this; }
    ivec4 &operator*=(const ivec4 &r) { v[0] *= r.v[0]; v[1] *= r.v[1]; v[2] *= r.v[2]; v[3] *= r.v[3]; return *this; }
    int dot(const ivec4 &r) const { return v[0] * r.v[0] + v[1] * r.v[1] + v[2] * r.v[2] + v[3] * r.v[3]; }
    ivec4 min(const ivec4 &r) const { return ivec4(v[0] < r[0] ? v[0] : r[0], v[1] < r[1] ? v[1] : r[1], v[2] < r[2] ? v[2] : r[2], v[3] < r[3] ? v[3] : r[3]); }
    ivec4 max(const ivec4 &r) const { return ivec4(v[0] >= r[0] ? v[0] : r[0], v[1] >= r[1] ? v[1] : r[1], v[2] >= r[2] ? v[2] : r[2], v[3] >= r[3] ? v[3] : r[3]); }
    int squared() { return dot(*this); }
    ivec4 abs() const { return ivec4(v[0] < 0 ? -v[0] : v[0], v[1] < 0 ? -v[1] : v[1], v[2] < 0 ? -v[2] : v[2], v[3] < 0 ? -v[3] : v[3]); }
    ivec4 xy() const { return ivec4(v[0], v[1], 0, 0); }
    ivec4 xyz() const { return ivec4(v[0], v[1], v[2], 0); }
    ivec4 xyz1() const { return ivec4(v[0], v[1], v[2], 1); }
    int &x() { return v[0]; }
    int &y() { return v[1]; }
    int &z() { return v[2]; }
    int &w() { return v[3]; }
    int x() const { return v[0]; }
    int y() const { return v[1]; }
    int z() const { return v[2]; }
    int w() const { return v[3]; }
  };

  class bvec4 {
    int v[4];
  public:
    bvec4() {}

    bvec4(bool x, bool y, bool z, bool w) { v[0] = x ? -1 : 0; v[1] = y ? -1 : 0; v[2] = z ? -1 : 0; v[3] = w ? -1 : 0; };
    bvec4(int x, int y, int z, int w) { v[0] = x; v[1] = y; v[2] = z; v[3] = w; };

    int &operator[](int i) { return v[i]; }
    const int &operator[](int i) const { return v[i]; }
    bvec4 operator&(int r) const { return bvec4(v[0]&r, v[1]&r, v[2]&r, v[3]&r); }
    bvec4 operator|(int r) const { return bvec4(v[0]|r, v[1]|r, v[2]|r, v[3]|r); }
    bvec4 operator&(const bvec4 &r) const { return bvec4(v[0]&r.v[0], v[1]&r.v[1], v[2]&r.v[2], v[3]&r.v[3]); }
    bvec4 operator|(const bvec4 &r) const { return bvec4(v[0]|r.v[0], v[1]|r.v[1], v[2]|r.v[2], v[3]|r.v[3]); }
    bvec4 &operator&=(const bvec4 &r) { v[0] &= r.v[0]; v[1] &= r.v[1]; v[2] &= r.v[2]; v[3] &= r.v[3]; return *this; }
    bvec4 &operator|=(const bvec4 &r) { v[0] |= r.v[0]; v[1] |= r.v[1]; v[2] |= r.v[2]; v[3] |= r.v[3]; return *this; }
    bvec4 xy() const { return bvec4(v[0], v[1], 0, 0); }
    bvec4 xyz() const { return bvec4(v[0], v[1], v[2], 0); }
    bvec4 xyz1() const { return bvec4(v[0], v[1], v[2], 1); }
    int &x() { return v[0]; }
    int &y() { return v[1]; }
    int &z() { return v[2]; }
    int &w() { return v[3]; }
    int x() const { return v[0]; }
    int y() const { return v[1]; }
    int z() const { return v[2]; }
    int w() const { return v[3]; }
  };

  inline bvec4 operator>(const vec4 &lhs, const vec4 &rhs) { return bvec4(lhs.x()>rhs.x(), lhs.y()>rhs.y(), lhs.z()>rhs.z(), lhs.w()>rhs.w()); }
  inline bvec4 operator<(const vec4 &lhs, const vec4 &rhs) { return bvec4(lhs.x()<rhs.x(), lhs.y()<rhs.y(), lhs.z()<rhs.z(), lhs.w()<rhs.w()); }
  inline bvec4 operator>=(const vec4 &lhs, const vec4 &rhs) { return bvec4(lhs.x()>=rhs.x(), lhs.y()>=rhs.y(), lhs.z()>=rhs.z(), lhs.w()>=rhs.w()); }
  inline bvec4 operator<=(const vec4 &lhs, const vec4 &rhs) { return bvec4(lhs.x()<=rhs.x(), lhs.y()<=rhs.y(), lhs.z()<=rhs.z(), lhs.w()<=rhs.w()); }
  inline bvec4 operator==(const vec4 &lhs, const vec4 &rhs) { return bvec4(lhs.x()==rhs.x(), lhs.y()==rhs.y(), lhs.z()==rhs.z(), lhs.w()==rhs.w()); }
  inline bvec4 operator!=(const vec4 &lhs, const vec4 &rhs) { return bvec4(lhs.x()!=rhs.x(), lhs.y()!=rhs.y(), lhs.z()!=rhs.z(), lhs.w()!=rhs.w()); }

  inline bvec4 operator>(const ivec4 &lhs, const ivec4 &rhs) { return bvec4(lhs.x()>rhs.x(), lhs.y()>rhs.y(), lhs.z()>rhs.z(), lhs.w()>rhs.w()); }
  inline bvec4 operator<(const ivec4 &lhs, const ivec4 &rhs) { return bvec4(lhs.x()<rhs.x(), lhs.y()<rhs.y(), lhs.z()<rhs.z(), lhs.w()<rhs.w()); }
  inline bvec4 operator>=(const ivec4 &lhs, const ivec4 &rhs) { return bvec4(lhs.x()>=rhs.x(), lhs.y()>=rhs.y(), lhs.z()>=rhs.z(), lhs.w()>=rhs.w()); }
  inline bvec4 operator<=(const ivec4 &lhs, const ivec4 &rhs) { return bvec4(lhs.x()<=rhs.x(), lhs.y()<=rhs.y(), lhs.z()<=rhs.z(), lhs.w()<=rhs.w()); }
  inline bvec4 operator==(const ivec4 &lhs, const ivec4 &rhs) { return bvec4(lhs.x()==rhs.x(), lhs.y()==rhs.y(), lhs.z()==rhs.z(), lhs.w()==rhs.w()); }
  inline bvec4 operator!=(const ivec4 &lhs, const ivec4 &rhs) { return bvec4(lhs.x()!=rhs.x(), lhs.y()!=rhs.y(), lhs.z()!=rhs.z(), lhs.w()!=rhs.w()); }
}

