////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Vector class
//

namespace octet { namespace math {
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

} }

