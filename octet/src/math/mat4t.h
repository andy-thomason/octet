////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// 4x4 matrix class
//
//

// This is similar to the opengl mat4 matrix, except that it is a transpose.
// We use it exactly the same way except that multiplies are left to right.
// This makes understanding matrix transforms much easier. (ie. the matrix is row major)
//
// In this framework, matrices are all written "spaceToSpace" to show that they
// go from one space to another. eg. modelToWorld.
//
// Examples are cameraToArm = cameraToWorld * worldToHip * hipToArm
//
// note that the spaces go together like dominos.
//
// The inverse of the matrix transforms the other way. so inverse4x4(modelToWorld) == worldToModel
//
namespace octet { namespace math {
  /// 4x4 Matrix class
  class mat4t {
    // these vectors are the x, y, z, w components. w is the translation.
    vec4 v[4];
    static const char *Copyright() { return "Copyright(C) Andy Thomason 2012-2014"; }
  public:
    /// Construct an identity matrix
    mat4t() {
      OCTET_VEC4_CONST(v0, 1, 0, 0, 0)
      OCTET_VEC4_CONST(v1, 0, 1, 0, 0)
      OCTET_VEC4_CONST(v2, 0, 0, 1, 0)
      OCTET_VEC4_CONST(v3, 0, 0, 0, 1)
      v[0] = v0;
      v[1] = v1;
      v[2] = v2;
      v[3] = v3;
    }

    /// Construct a matrix from vectors
    mat4t(const vec4 &x, const vec4 &y, const vec4 &z, const vec4 &w)
    {
      v[0] = x; v[1] = y; v[2] = z; v[3] = w;
    }

    /// Construct a scalar matrix
    mat4t(float diag) {
      v[0] = vec4(diag, 0.0f, 0.0f, 0.0f);
      v[1] = vec4(0.0f, diag, 0.0f, 0.0f);
      v[2] = vec4(0.0f, 0.0f, diag, 0.0f);
      v[3] = vec4(0.0f, 0.0f, 0.0f, diag);
    }
  
    /// Construct a matrix from a quaternion.
    mat4t(const quat &r)
    {
      // http://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation
      float a = r[3], b = r[0], c = r[1], d = r[2];
      v[0] = vec4( a*a + b*b - c*c - d*d, 2 * ( b*c + a*d ), 2 * ( b*d - a*c ), 0.0f ),
      v[1] = vec4( 2 * ( b*c - a*d ), a*a - b*b + c*c - d*d, 2 * ( c*d + a*b ), 0.0f ),
      v[2] = vec4( 2 * ( b*d + a*c ), 2 * ( c*d - a*b ), a*a - b*b - c*c + d*d, 0.0f ),
      v[3] = vec4( 0.0f, 0.0f, 0.0f, 1.0f );
    }

    /// like the OpenGL 1.0 LoadIdentity
    mat4t &loadIdentity() {
      v[0][0] = 1; v[0][1] = 0; v[0][2] = 0; v[0][3] = 0;
      v[1][0] = 0; v[1][1] = 1; v[1][2] = 0; v[1][3] = 0;
      v[2][0] = 0; v[2][1] = 0; v[2][2] = 1; v[2][3] = 0;
      v[3][0] = 0; v[3][1] = 0; v[3][2] = 0; v[3][3] = 1;
      return *this;
    }

    /// in-place initialise
    void init(const vec4 &x, const vec4 &y, const vec4 &z, const vec4 &w)
    {
      v[0] = x; v[1] = y; v[2] = z; v[3] = w;
    }

    /// initialise the matrix from a collada matrix, for example.
    void init_transpose(const float *f)
    {
      init(
        vec4(f[0], f[4], f[8], f[12]),
        vec4(f[1], f[5], f[9], f[13]),
        vec4(f[2], f[6], f[10], f[14]),
        vec4(f[3], f[7], f[11], f[15])
      );
    }

    /// initialise the matrix from a C-style matrix
    void init_c_style(const float *f)
    {
      init(
        vec4(f[0], f[1], f[2], f[3]),
        vec4(f[4], f[5], f[6], f[7]),
        vec4(f[8], f[9], f[10], f[11]),
        vec4(f[12], f[13], f[14], f[15])
      );
    }

    /// Index the matrix rows.
    vec4 &operator[](int i) { return v[i]; }

    /// Read the matrix rows.
    const vec4 &operator[](int i) const { return v[i]; }

    /// Read a row.
    vec4 row(int i) const { return v[i]; }

    /// Read a column.
    vec4 column(int i) const { return vec4(v[0][i], v[1][i], v[2][i], v[3][i]); }

    /// Access the float values of the matrix (for OpenGL uniforms)
    float *get() { return &v[0][0]; }

    /// Read the float values of the matrix (for OpenGL uniforms)
    const float *get() const { return &v[0][0]; }
  
    /// OpenGL-style scale of this matrix
    mat4t &scale(float x, float y, float z) {
      v[0] = v[0] * x;
      v[1] = v[1] * y;
      v[2] = v[2] * z;
      /*for (int i = 0; i != 4; ++i) {
        v[i][0] *= x;
        v[i][1] *= y;
        v[i][2] *= z;
      }*/
      return *this;
    }
  
    /// OpenGL-style translate of this matrix
    mat4t &translate(float x, float y, float z) {
      v[3] = lmul(vec4(x,y,z,1.0f));
      return *this;
    }
  
    mat4t &translate(vec3_in value) {
      v[3] = lmul(vec4(value, 1.0f));
      return *this;
    }
  
    /// Multiply operator: note this treats matrices as row-major unlike gl matrices which are column-major
    mat4t operator*(const mat4t &r) const
    {
      mat4t res;
      for (int i = 0; i != 4; ++i) {
        res.v[i] = r[0] * v[i].xxxx() + r[1] * v[i].yyyy() + r[2] * v[i].zzzz() + r[3] * v[i].wwww();
      }
      return res;
    }
  
    /// Add operator
    mat4t operator+(const mat4t &r) const
    {
      return mat4t(
        v[0] + r.v[0],
        v[1] + r.v[1],
        v[2] + r.v[2],
        v[3] + r.v[3]
      );
    }
  
    /// In-place add operator
    mat4t &operator+=(const mat4t &r)
    {
      v[0] += r.v[0];
      v[1] += r.v[1];
      v[2] += r.v[2];
      v[3] += r.v[3];
      return *this;
    }

    /// Get the trace of the matrix
    vec4 trace() const {
      return vec4(v[0].x(), v[1].y(), v[2].z(), v[3].w());
    }
  
    /// Rotate the matrix about an axis in degrees
    mat4t &rotate(float angle, float x, float y, float z) {
      float c = cosf(angle * (3.14159265f/180));
      float s = sinf(angle * (3.14159265f/180));
      mat4t r(
        vec4(x*x*(1-c)+c,   x*y*(1-c)+z*s, x*z*(1-c)-y*s, 0.0f),
        vec4(x*y*(1-c)-z*s, y*y*(1-c)+c,   y*z*(1-c)+x*s, 0.0f),
        vec4(x*z*(1-c)+y*s, y*z*(1-c)-x*s, z*z*(1-c)+c,   0.0f),
        vec4(          0.0f,               0.0f,             0.0f, 1.0f)
      );
      *this = r * *this;

      /*
      // this may be faster
      float x1c = x * (1-c), y1c = y * (1-c), z1c = z * (1-c);
      vec4 v0 = v[0], v1 = v[1], v2 = v[2];
      v[0] = v0 * (x*x1c+c) + v1 * (x*y1c+z*s) + v2 * (x*z1c-y*s);
      v[1] = v0 * (x*y1c-z*s) + v1 * (y*y1c+c) + v2 * (y*z1c+x*s);
      v[2] = v0 * (x*z1c+y*s) + v1 * (y*z1c-x*s) + v2 * (z*z1c+c);
      */

      return *this;
    }

    /// Skew the matrix (as in Collada skew)
    mat4t &skew(float angle, float x1, float y1, float z1, float x2, float y2, float z2) {
      float t = tanf(angle * (3.14159265f/180));
      vec4 v(x1, y1, z1, 0.0f);
      vec4 w(x2, y2, z2, 0.0f);
      v = v.normalize() * t;
      w = w.normalize();
      mat4t r(
        vec4( 1 + v.x() * w.x(),     v.x() * w.y(),     v.x() * w.z(), 0.0f ), 
        vec4(     v.y() * w.x(), 1 + v.y() * w.y(),     v.y() * w.z(), 0.0f ), 
        vec4(     v.z() * w.x(),     v.z() * w.y(), 1 + v.z() * w.z(), 0.0f ), 
        vec4(             0.0f,             0.0f,             0.0f,             1.0f ) 
      );
      *this = r * *this;
      return *this;
    }

    /// Specialised xyz axis rotate. More efficent than rotate()
    mat4t &rotateSpecial(float cosAngle, float sinAngle, int a, int b) {
      vec4 t = v[a] * cosAngle + v[b] * sinAngle;
      v[b] = v[b] * cosAngle - v[a] * sinAngle;
      v[a] = t;
      return *this;
    }

    /// rotate by angle in degrees about X axis
    mat4t &rotateX(float angle) { return rotateSpecial(cosf(angle*(3.14159265f/180)), sinf(angle*(3.14159265f/180)), 1, 2); }

    /// rotate by angle in degrees about Y axis
    mat4t &rotateY(float angle) { return rotateSpecial(cosf(angle*(3.14159265f/180)), sinf(angle*(3.14159265f/180)), 2, 0); }

    /// rotate by angle in degrees about Z axis
    mat4t &rotateZ(float angle) { return rotateSpecial(cosf(angle*(3.14159265f/180)), sinf(angle*(3.14159265f/180)), 0, 1); }

    // accurate rotate by 90 degrees about X axis
    mat4t &rotateX90() { return rotateSpecial(0, 1, 1, 2); }

    // accurate rotate by 90 degrees about Y axis
    mat4t &rotateY90() { return rotateSpecial(0, 1, 2, 0); }

    // accurate rotate by 90 degrees about Z axis
    mat4t &rotateZ90() { return rotateSpecial(0, 1, 0, 1); }

    // accurate rotate by 180 degrees about X axis
    mat4t &rotateX180() { return rotateSpecial(-1, 0, 1, 2); }

    // accurate rotate by 180 degrees about Y axis
    mat4t &rotateY180() { return rotateSpecial(-1, 0, 2, 0); }

    // accurate rotate by 180 degrees about Z axis
    mat4t &rotateZ180() { return rotateSpecial(-1, 0, 0, 1); }

    /// Multiply by vector on the left
    // [l[0],l[1],l[2],l[3]] * [v[0],v[1],v[2],v[3]]
    vec4 lmul(const vec4 &l) const {
      //return v[0] * l[0] + v[1] * l[1] + v[2] * l[2] + v[3] * l[3];
      return v[0] * l.xxxx() + v[1] * l.yyyy() + v[2] * l.zzzz() + v[3] * l.wwww();
    }
  
    /// Multiply by vector on the right
    // [v[0],v[1],v[2],v[3]] * [r[0],r[1],r[2],r[3]]
    vec4 rmul(const vec4 &r) const {
      return vec4(
        v[0].dot(r),
        v[1].dot(r),
        v[2].dot(r),
        v[3].dot(r)
      );
    }
  
    /// Quick invert, assumes the matrix is a rotate and translate only.
    // works for orthonormal rotation component matrices
    void invertQuick(mat4t &d) const {
      // transpose x, y, z
      for (int i = 0; i != 3; ++i) {
        d[i] = vec4(v[0][i], v[1][i], v[2][i], 0.0f);
      }
      d[3] = vec4(0, 0, 0, 1);
      // translate by new matrix
      d[3] = d.lmul(vec4(-v[3][0], -v[3][1], -v[3][2], 1.0f));
    }

    /// get a transpose of the matrix
    mat4t transpose4x4() const {
      return mat4t( colx(), coly(), colz(), colw() );
    }

    /// Get the full inverse of the matrix
    mat4t inverse4x4() const {
      vec4 v0 = v[0];
      vec4 v1 = v[1];
      vec4 v2 = v[2];
      vec4 v3 = v[3];
      float rdet = 1.0f / det4x4();
      return mat4t(
        vec4(
          v1[2]*v2[3]*v3[1] - v1[3]*v2[2]*v3[1] + v1[3]*v2[1]*v3[2] - v1[1]*v2[3]*v3[2] - v1[2]*v2[1]*v3[3] + v1[1]*v2[2]*v3[3],
          v0[3]*v2[2]*v3[1] - v0[2]*v2[3]*v3[1] - v0[3]*v2[1]*v3[2] + v0[1]*v2[3]*v3[2] + v0[2]*v2[1]*v3[3] - v0[1]*v2[2]*v3[3],
          v0[2]*v1[3]*v3[1] - v0[3]*v1[2]*v3[1] + v0[3]*v1[1]*v3[2] - v0[1]*v1[3]*v3[2] - v0[2]*v1[1]*v3[3] + v0[1]*v1[2]*v3[3],
          v0[3]*v1[2]*v2[1] - v0[2]*v1[3]*v2[1] - v0[3]*v1[1]*v2[2] + v0[1]*v1[3]*v2[2] + v0[2]*v1[1]*v2[3] - v0[1]*v1[2]*v2[3]
        ) * rdet,
        vec4(
          v1[3]*v2[2]*v3[0] - v1[2]*v2[3]*v3[0] - v1[3]*v2[0]*v3[2] + v1[0]*v2[3]*v3[2] + v1[2]*v2[0]*v3[3] - v1[0]*v2[2]*v3[3],
          v0[2]*v2[3]*v3[0] - v0[3]*v2[2]*v3[0] + v0[3]*v2[0]*v3[2] - v0[0]*v2[3]*v3[2] - v0[2]*v2[0]*v3[3] + v0[0]*v2[2]*v3[3],
          v0[3]*v1[2]*v3[0] - v0[2]*v1[3]*v3[0] - v0[3]*v1[0]*v3[2] + v0[0]*v1[3]*v3[2] + v0[2]*v1[0]*v3[3] - v0[0]*v1[2]*v3[3],
          v0[2]*v1[3]*v2[0] - v0[3]*v1[2]*v2[0] + v0[3]*v1[0]*v2[2] - v0[0]*v1[3]*v2[2] - v0[2]*v1[0]*v2[3] + v0[0]*v1[2]*v2[3]
        ) * rdet,
        vec4(
          v1[1]*v2[3]*v3[0] - v1[3]*v2[1]*v3[0] + v1[3]*v2[0]*v3[1] - v1[0]*v2[3]*v3[1] - v1[1]*v2[0]*v3[3] + v1[0]*v2[1]*v3[3],
          v0[3]*v2[1]*v3[0] - v0[1]*v2[3]*v3[0] - v0[3]*v2[0]*v3[1] + v0[0]*v2[3]*v3[1] + v0[1]*v2[0]*v3[3] - v0[0]*v2[1]*v3[3],
          v0[1]*v1[3]*v3[0] - v0[3]*v1[1]*v3[0] + v0[3]*v1[0]*v3[1] - v0[0]*v1[3]*v3[1] - v0[1]*v1[0]*v3[3] + v0[0]*v1[1]*v3[3],
          v0[3]*v1[1]*v2[0] - v0[1]*v1[3]*v2[0] - v0[3]*v1[0]*v2[1] + v0[0]*v1[3]*v2[1] + v0[1]*v1[0]*v2[3] - v0[0]*v1[1]*v2[3]
        ) * rdet,
        vec4(
          v1[2]*v2[1]*v3[0] - v1[1]*v2[2]*v3[0] - v1[2]*v2[0]*v3[1] + v1[0]*v2[2]*v3[1] + v1[1]*v2[0]*v3[2] - v1[0]*v2[1]*v3[2],
          v0[1]*v2[2]*v3[0] - v0[2]*v2[1]*v3[0] + v0[2]*v2[0]*v3[1] - v0[0]*v2[2]*v3[1] - v0[1]*v2[0]*v3[2] + v0[0]*v2[1]*v3[2],
          v0[2]*v1[1]*v3[0] - v0[1]*v1[2]*v3[0] - v0[2]*v1[0]*v3[1] + v0[0]*v1[2]*v3[1] + v0[1]*v1[0]*v3[2] - v0[0]*v1[1]*v3[2],
          v0[1]*v1[2]*v2[0] - v0[2]*v1[1]*v2[0] + v0[2]*v1[0]*v2[1] - v0[0]*v1[2]*v2[1] - v0[1]*v1[0]*v2[2] + v0[0]*v1[1]*v2[2]
        ) * rdet
      );
    }

    /// Get the 3x3 adjoint matrix, used for generalized 3x3 invert.
    mat4t adjoint3x3() const {
      vec4 c0 = column(0);
      vec4 c1 = column(1);
      vec4 c2 = column(2);
      return mat4t(
        c1.cross(c2),
        c2.cross(c0),
        c0.cross(c1),
        vec4(0, 0, 0, 1)
      );
    }
  
    /// Get the determinant of a 3x3 matrix
    float det3x3() const {
      return v[0].cross(v[1]).dot(v[2]);
    }
  
    /// Get the complete 4x4 determinant
    float det4x4() const {
      vec4 v0 = v[0];
      vec4 v1 = v[1];
      vec4 v2 = v[2];
      vec4 v3 = v[3];
      return
        v0[3]*v1[2]*v2[1]*v3[0] - v0[2]*v1[3]*v2[1]*v3[0] - v0[3]*v1[1]*v2[2]*v3[0] + v0[1]*v1[3]*v2[2]*v3[0]+
        v0[2]*v1[1]*v2[3]*v3[0] - v0[1]*v1[2]*v2[3]*v3[0] - v0[3]*v1[2]*v2[0]*v3[1] + v0[2]*v1[3]*v2[0]*v3[1]+
        v0[3]*v1[0]*v2[2]*v3[1] - v0[0]*v1[3]*v2[2]*v3[1] - v0[2]*v1[0]*v2[3]*v3[1] + v0[0]*v1[2]*v2[3]*v3[1]+
        v0[3]*v1[1]*v2[0]*v3[2] - v0[1]*v1[3]*v2[0]*v3[2] - v0[3]*v1[0]*v2[1]*v3[2] + v0[0]*v1[3]*v2[1]*v3[2]+
        v0[1]*v1[0]*v2[3]*v3[2] - v0[0]*v1[1]*v2[3]*v3[2] - v0[2]*v1[1]*v2[0]*v3[3] + v0[1]*v1[2]*v2[0]*v3[3]+
        v0[2]*v1[0]*v2[1]*v3[3] - v0[0]*v1[2]*v2[1]*v3[3] - v0[1]*v1[0]*v2[2]*v3[3] + v0[0]*v1[1]*v2[2]*v3[3]
      ;
    }
  
    /// Get the general inverse of a 3x3 matrix
    mat4t inverse3x3() const {
      return adjoint3x3() * (1.0f/det3x3());
    }
  
    /// Get the  general inverse of 3x4 matrix (with v[3] = vec4(0, 0, 0, 1))
    mat4t inverse3x4() const {
      float rdet = 1.0f / det3x3();
      mat4t d = adjoint3x3();
      d[0] = d[0] * rdet;
      d[1] = d[1] * rdet;
      d[2] = d[2] * rdet;
      d[3] = d.lmul(vec4(-v[3][0], -v[3][1], -v[3][2], 1.0f));
      return d;
    }
  
    /// Get the absolute value of a matrix; useful for extents.
    mat4t abs() const { return mat4t(v[0].abs(), v[1].abs(), v[2].abs(), v[3].abs()); }
  
    // In place matrix multiply, as in glMultMatrix
    mat4t &multMatrix(const mat4t &r)
    {
      *this = r * *this;
      return *this;
    }
  
    /// As in glFrustum - make a perspective matrix used for conventional cameras
    /// this matrix makes a 4x4 vector which is then divided to get perspective
    mat4t &frustum(float left, float right, float bottom, float top, float n, float f)
    {
      float X = 2*n / (right-left);
      float Y = 2*n / (top-bottom);
      float A = (right+left) / (right-left);
      float B = (top+bottom) / (top-bottom);
      float C = -(f+n) / (f-n);
      float D = -2*f*n / (f-n);

      // before the divide:
      // xp = ( 2*n*x + (right+left) * z ) / (right-left);
      // yp = ( 2*n*y + (top+bottom) * z ) / (top-bottom);
      // zp = (-2*n*f -        (f+n) * z ) / (f-n)
      // wp = -z

      // after the divide:
      // xd = ( -2*n*x/z - (right+left) ) / (right-left);
      // yd = ( -2*n*y/z - (top+bottom) ) / (top-bottom);
      // zd = (  2*n*f/z        + (f+n) ) / (f-n)

      // special values of z:
      // if z == -n: zd = ( (f+n) - 2*f ) / (f-n) = -1   (zp=n)
      // if z == -f: zd = ( (f+n) - 2*n ) / (f-n) = 1    (zp=-f)
      // if z == -infinity: zd = (f+n) / (f-n)

      // important: we get more z values closer to the camera,
      // so choose f and especially n with care!

      mat4t mul(
        vec4( X, 0.0f, 0.0f,  0.0f ),
        vec4( 0.0f, Y, 0.0f,  0.0f ),
        vec4( A, B, C, -1.0f ),
        vec4( 0.0f, 0.0f, D,  0.0f )
      );
      *this = mul * *this;
      return *this;
    }
  
    /// As in glOrtho - make a non-shrinking camera matrix (wp=1)
    /// used for GUI displays and editors.
    mat4t &ortho(float left, float right, float bottom, float top, float nearVal, float farVal)
    {
      float X = 2.0f / (right-left);
      float Y = 2.0f / (top-bottom);
      float Z = 2.0f / (farVal-nearVal);
      float tx = -(right+left) / (right-left);
      float ty = -(top+bottom) / (top-bottom);
      float tz = -(farVal+nearVal) / (farVal-nearVal);
      mat4t mul(
        vec4( X,  0.0f,  0.0f,  0.0f ),
        vec4( 0.0f,  Y,  0.0f,  0.0f ),
        vec4( 0.0f,  0.0f,  Z,  0.0f ),
        vec4( tx, ty, tz, 1.0f )
      );
      *this = mul * *this;
      return *this;
    }
  
    /// multiply by scalar
    mat4t operator*(float r) const { return mat4t(v[0]*r, v[1]*r, v[2]*r, v[3]*r); }

    /// postmultiply by a vector
    vec4 operator*(const vec4 &r) const {
      return vec4(
        v[0].dot(r),
        v[1].dot(r),
        v[2].dot(r),
        v[3].dot(r)
      );
    }

    /// Convert a rotation matrix to a quaternion.
    quat toQuaternion() const {
      float trace = v[0][0] + v[1][1] + v[2][2];
      if (trace > 0)
      {
        // most common case - trace is positive - good for small rotations
        float rootTrPlus1 = sqrtf(trace + 1);
        float scale = 0.5f / rootTrPlus1;
        float x = (v[1][2] - v[2][1]) * scale;
        float y = (v[2][0] - v[0][2]) * scale;
        float z = (v[0][1] - v[1][0]) * scale;
        return quat(x, y, z, rootTrPlus1 * 0.5f);
      } else 
      {
        // less common, look for dominant diagonal term
	      int i = v[0][0] >= v[1][1] ? 0 : v[1][1] >= v[2][2] ? 1 : v[0][0] >= v[2][2] ? 0 : 2; 
	      int j = i + 1 >= 3 ? i + 1 - 3 : i + 1; 
	      int k = i + 2 >= 3 ? i + 2 - 3 : i + 2; 
        float rootTrPlus1 = sqrtf(v[i][i] - v[j][j] - v[k][k] + 1);
        float scale = 0.5f / rootTrPlus1;
	      float t[3];
	      t[i] = rootTrPlus1 * 0.5f;
	      t[j] = (v[j][i] + v[i][j]) * scale;
	      t[k] = (v[k][i] + v[i][k]) * scale;
        return quat(t[0], t[1], t[2], (v[j][k] - v[k][j]) * scale);
		  }
    }
  
    /// Convert to a string for debugging.
    const char *toString(char *dest, size_t len) const
    {
      char buf[4][256];
      snprintf(
        dest, len, "[%s, %s, %s, %s]",
        v[0].toString(buf[0], sizeof(buf[0])),
        v[1].toString(buf[1], sizeof(buf[1])),
        v[2].toString(buf[2], sizeof(buf[2])),
        v[3].toString(buf[3], sizeof(buf[3]))
      );
      return dest;
    }

    /// helper function for building a simple camera
    static mat4t build_projection_matrix(const mat4t &modelToWorld, const mat4t &cameraToWorld, float n = 0.1f, float f = 1000.0f)
    {
      // flip it around to transform from world to camera
      mat4t worldToCamera;
      cameraToWorld.invertQuick(worldToCamera);

      // build a projection matrix to add perspective
      mat4t cameraToProjection;
      cameraToProjection.loadIdentity();
      cameraToProjection.frustum(-n, n, -n, n, n, f);

      // model -> world -> camera -> projection
      return modelToWorld * worldToCamera * cameraToProjection;
    }

    /// helper function for building a simple camera
    static mat4t build_camera_matrices(mat4t &modelToCamera, mat4t &worldToCamera, const mat4t &modelToWorld, const mat4t &cameraToWorld, float n = 0.1f, float f = 1000.0f)
    {
      // flip it around to transform from world to camera
      cameraToWorld.invertQuick(worldToCamera);

      modelToCamera = modelToWorld * worldToCamera;

      // build a projection matrix to add perspective
      mat4t cameraToProjection;
      cameraToProjection.loadIdentity();
      cameraToProjection.frustum(-n, n, -n, n, n, f);

      // model -> world -> camera -> projection
      return modelToCamera * cameraToProjection;
    }

    /// Gram Schmidt orthogonalisation: make a pure rotation.
    mat4t normalize_3x3() {
      // note: it may be best to normalize z first.
      vec4 u0 = v[0];
      vec4 u1 = v[1] - u0 * (u0.dot(v[1])/u0.squared());
      vec4 u2 = v[2] - u0 * (u0.dot(v[2])/u0.squared()) - u1 * (u1.dot(v[2])/u1.squared());
      return mat4t(u0.normalize(), u1.normalize(), u2.normalize(), vec4(0, 0, 0, 1));
    }

    /// QR decomposition (after normalize_3x3)
    /// get R component (upper triangular)
    mat4t get_skew(const mat4t &rotation) {
      return xyz() * rotation.transpose4x4();
    }

    /// Power method to find approximate pincipal axis.
    /// Only works for matrices where there is a single dominant eigenvalue.
    /// ie. no good for rotations.
    vec4 get_principal_axis(unsigned steps=5) {
      vec4 t = v[2].normalize();
      for (unsigned i = 0; i != 5; ++i) {
        t = (t * *this).normalize();
      }
      return t;
    }

    /// Assuming this is a rotation matrix, get the axis and angle (in degrees).
    vec3 get_rotation(float &angle) {
      quat q = toQuaternion();

      // for stability, use atan2
      vec3 axis = q.xyz();
      float length = axis.length();
      angle = atan2f(length, q[3]) * (360.0f/3.14159265f);
      return length > 0.000001f ? axis / length : vec3(0, 0, 1);
    }

    mat4t &lookat(vec3_in target, vec3_in up=vec3(0, 1, 0)) {
      vec3 z = normalize(v[3].xyz() - target);
      vec3 x = normalize(cross(up, z));
      vec3 y = cross(z, x);
      v[2] = vec4(z, 0);
      v[0] = vec4(x, 0);
      v[1] = vec4(y, 0);
      return *this;
    }

    /// get 2x2 sub-matrix
    mat4t xy() const { return mat4t(v[0].xy00(), v[1].xy00(), vec4(0, 0, 1, 0), vec4(0, 0, 0, 1)); }

    /// get 3x3 sub-matrix
    mat4t xyz() const { return mat4t(v[0].xyz0(), v[1].xyz0(), v[2].xyz0(), vec4(0, 0, 0, 1)); }

    // get X row
    const vec4 &x() const { return v[0]; }

    // get Y row
    const vec4 &y() const { return v[1]; }

    // get Z row
    const vec4 &z() const { return v[2]; }

    // get W row
    const vec4 &w() const { return v[3]; }

    // access X row
    vec4 &x() { return v[0]; }

    // access Y row
    vec4 &y() { return v[1]; }

    // access Z row
    vec4 &z() { return v[2]; }

    // access W row
    vec4 &w() { return v[3]; }

    // get X column
    vec4 colx() const { return vec4( v[0][0], v[1][0], v[2][0], v[3][0] ); }

    // get X column
    vec4 coly() const { return vec4( v[0][1], v[1][1], v[2][1], v[3][1] ); }

    // get X column
    vec4 colz() const { return vec4( v[0][2], v[1][2], v[2][2], v[3][2] ); }

    // get X column
    vec4 colw() const { return vec4( v[0][3], v[1][3], v[2][3], v[3][3] ); }
  };

  /// vector times a matrix (premultiplication)
  inline vec4 vec4::operator*(const mat4t &r) const
  {
    return r.lmul(*this);
  }

  /// Outer product of two 4x4 vectors.
  inline mat4t outer(const vec4 &lhs, const vec4 &rhs) {
    return mat4t(
      lhs * rhs.x(),
      lhs * rhs.y(),
      lhs * rhs.z(),
      lhs * rhs.w()
    );
  }

  /// Multiply vec3 by 3x4 matrix.
  inline vec3 operator*(const vec3 &lhs, const mat4t &rhs) {
    //return rhs[0].xyz() * lhs[0] + rhs[1].xyz() * lhs[1] + rhs[2].xyz() * lhs[2] + rhs[3].xyz();
    return (rhs[0] * lhs.xxxx() + rhs[1] * lhs.yyyy() + rhs[2] * lhs.zzzz() + rhs[3]).xyz();
  }

  /// Get 3x4 inverse
  static inline mat4t inverse3x4(const mat4t &v) {
    return v.inverse3x4();
  }

  /// Get full 4x4 inverse.
  static inline mat4t inverse4x4(const mat4t &v) {
    return v.inverse4x4();
  }
} }
