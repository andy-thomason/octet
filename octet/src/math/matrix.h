////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
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
// The inverse of the matrix transforms the other way. so inverse4x4(modelToWorld) == worldToModel
//
class mat4t {
  // these vectors are the x, y, z, w components. w is the translation.
  vec4 v[4];
  static const char *Copyright() { return "Copyright(C) Andy Thomason 2012, 2013"; }
public:
  mat4t() {}
  mat4t(const vec4 &x, const vec4 &y, const vec4 &z, const vec4 &w)
  {
    v[0] = x; v[1] = y; v[2] = z; v[3] = w;
  }
  
  mat4t(const quat &r)
  {
    // http://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation
    float a = r[3], b = r[0], c = r[1], d = r[2];
    v[0] = vec4( a*a + b*b - c*c - d*d, 2 * ( b*c + a*d ), 2 * ( b*d - a*c ), 0 ),
    v[1] = vec4( 2 * ( b*c - a*d ), a*a - b*b + c*c - d*d, 2 * ( c*d + a*b ), 0 ),
    v[2] = vec4( 2 * ( b*d + a*c ), 2 * ( c*d - a*b ), a*a - b*b - c*c + d*d, 0 ),
    v[3] = vec4( 0, 0, 0, 1 );
  }

  // like the OpenGL 1.0 LoadIdentity
  mat4t &loadIdentity() {
    v[0][0] = 1; v[0][1] = 0; v[0][2] = 0; v[0][3] = 0;
    v[1][0] = 0; v[1][1] = 1; v[1][2] = 0; v[1][3] = 0;
    v[2][0] = 0; v[2][1] = 0; v[2][2] = 1; v[2][3] = 0;
    v[3][0] = 0; v[3][1] = 0; v[3][2] = 0; v[3][3] = 1;
    return *this;
  }

  // in-place initialise
  void init(const vec4 &x, const vec4 &y, const vec4 &z, const vec4 &w)
  {
    v[0] = x; v[1] = y; v[2] = z; v[3] = w;
  }

  // initialise the matrix one row at a time
  void init_row_major(const float *f)
  {
    init(
      vec4(f[0], f[4], f[8], f[12]),
      vec4(f[1], f[5], f[9], f[13]),
      vec4(f[2], f[6], f[10], f[14]),
      vec4(f[3], f[7], f[11], f[15])
    );
  }

  // initialise the matrix one column at a time
  void init_col_major(const float *f)
  {
    init(
      vec4(f[0], f[1], f[2], f[3]),
      vec4(f[4], f[5], f[6], f[7]),
      vec4(f[8], f[9], f[10], f[11]),
      vec4(f[12], f[13], f[14], f[15])
    );
  }

  vec4 &operator[](int i) { return v[i]; }
  const vec4 &operator[](int i) const { return v[i]; }

  vec4 row(int i) { return v[i]; }
  vec4 column(int i) { return vec4(v[0][i], v[1][i], v[2][i], v[3][i]); }
  const vec4 row(int i) const { return v[i]; }
  const vec4 column(int i) const { return vec4(v[0][i], v[1][i], v[2][i], v[3][i]); }

  float *get() { return &v[0][0]; }
  const float *get() const { return &v[0][0]; }
  
  // OpenGL-style scale of this matrix
  mat4t &scale(float x, float y, float z) {
    for (int i = 0; i != 4; ++i) {
      v[i][0] *= x;
      v[i][1] *= y;
      v[i][2] *= z;
    }
    return *this;
  }
  
  // OpenGL-style translate of this matrix
  mat4t &translate(float x, float y, float z) {
    v[3] = lmul(vec4(x,y,z,1));
    return *this;
  }
  
  // note this treats matrices as row-major unlike gl matrices which are column-major
  mat4t operator*(const mat4t &r) const
  {
    mat4t res;
    for (int i = 0; i != 4; ++i) {
      res.v[i] = r[0] * v[i][0] + r[1] * v[i][1] + r[2] * v[i][2] + r[3] * v[i][3];
    }
    return res;
  }
  
  mat4t operator+(const mat4t &r) const
  {
    return mat4t(
      v[0] + r.v[0],
      v[1] + r.v[1],
      v[2] + r.v[2],
      v[3] + r.v[3]
    );
  }
  
  // generalized single axis rotate
  mat4t &rotate(float x, float y, float z, float angle) {
    float c = cosf(angle * (3.14159265f/180));
    float s = sinf(angle * (3.14159265f/180));
    mat4t r(
      vec4(x*x*(1-c)+c,   y*x*(1-c)+z*s, x*z*(1-c)-y*s, 0),
      vec4(x*y*(1-c)-z*s, y*y*(1-c)+c,   y*z*(1-c)+x*s, 0),
      vec4(x*z*(1-c)+y*s, y*z*(1-c)-x*s, z*z*(1-c)+c,   0),
      vec4(          0,               0,             0, 1)
    );
    return multMatrix(r);
  }

  // specialised xyz axis rotate
  mat4t &rotateSpecial(float cosAngle, float sinAngle, int a, int b) {
    vec4 t = v[a] * cosAngle + v[b] * sinAngle;
    v[b] = v[b] * cosAngle - v[a] * sinAngle;
    v[a] = t;
    return *this;
  }

  // rotate by angle in degrees
  mat4t &rotateX(float angle) { return rotateSpecial(cosf(angle*(3.14159265f/180)), sinf(angle*(3.14159265f/180)), 1, 2); }
  mat4t &rotateY(float angle) { return rotateSpecial(cosf(angle*(3.14159265f/180)), sinf(angle*(3.14159265f/180)), 2, 0); }
  mat4t &rotateZ(float angle) { return rotateSpecial(cosf(angle*(3.14159265f/180)), sinf(angle*(3.14159265f/180)), 0, 1); }

  // accurate rotate by 90
  mat4t &rotateX90() { return rotateSpecial(0, 1, 1, 2); }
  mat4t &rotateY90() { return rotateSpecial(0, 1, 2, 0); }
  mat4t &rotateZ90() { return rotateSpecial(0, 1, 0, 1); }

  // accurate rotate by 180
  mat4t &rotateX180() { return rotateSpecial(-1, 0, 1, 2); }
  mat4t &rotateY180() { return rotateSpecial(-1, 0, 2, 0); }
  mat4t &rotateZ180() { return rotateSpecial(-1, 0, 0, 1); }

  // multiply by vector on the left
  // [l[0],l[1],l[2],l[3]] * [v[0],v[1],v[2],v[3]]
  vec4 lmul(const vec4 &l) const {
    return v[0] * l[0] + v[1] * l[1] + v[2] * l[2] + v[3] * l[3];
  }
  
  // multiply by vector on the right
  // [v[0],v[1],v[2],v[3]] * [r[0],r[1],r[2],r[3]]
  vec4 rmul(const vec4 &r) const {
    return vec4(
      v[0].dot(r),
      v[1].dot(r),
      v[2].dot(r),
      v[3].dot(r)
    );
  }
  
  // quick invert, assumes the matrix is a rotate and translate only.
  // works for orthonormal rotation component matrices
  void invertQuick(mat4t &d) const {
    // transpose x, y, z
    for (int i = 0; i != 3; ++i) {
      d[i] = vec4(v[0][i], v[1][i], v[2][i], 0);
    }
    d[3] = vec4(0, 0, 0, 1);
    // translate by new matrix
    d[3] = d.lmul(vec4(-v[3][0], -v[3][1], -v[3][2], 1));
  }

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

  // 3x3 adjoint matrix, used for generalized 3x3 invert
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
  
  // determinant of 3x3 matrix
  float det3x3() const {
    return v[0].cross(v[1]).dot(v[2]);
  }
  
  // complete 4x4 determinant
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
  
  // general inverse of 3x3 matrix
  mat4t inverse3x3() const {
    return adjoint3x3() * (1.0f/det3x3());
  }
  
  // general inverse of 3x4 matrix (with v[3] = vec4(0, 0, 0, 1))
  mat4t inverse3x4() const {
    float rdet = 1.0f / det3x3();
    mat4t d = adjoint3x3();
    d[0] = d[0] * rdet;
    d[1] = d[1] * rdet;
    d[2] = d[2] * rdet;
    d[3] = d.lmul(vec4(-v[3][0], -v[3][1], -v[3][2], 1));
    return d;
  }
  
  // absolute of matrix useful for extents.
  mat4t abs() const { return mat4t(v[0].abs(), v[1].abs(), v[2].abs(), v[3].abs()); }
  
  // as in glMultMatrix
  mat4t &multMatrix(const mat4t &r)
  {
    *this = r * *this;
    return *this;
  }
  
  // as in glFrustum - make a perspective matrix used for conventional cameras
  // this matrix makes a 4x4 vector which is then divided to get perspective
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

    // important: we get more z values closer to the camera, so choose f and n with care!

    mat4t mul(
      vec4( X, 0, 0,  0 ),
      vec4( 0, Y, 0,  0 ),
      vec4( A, B, C, -1 ),
      vec4( 0, 0, D,  0 )
    );
    *this = mul * *this;
    return *this;
  }
  
  // as in glOrtho - make a non-shrinking camera matrix (wp=1)
  // used for GUI displays and editors.
  mat4t &ortho(float left, float right, float bottom, float top, float nearVal, float farVal)
  {
    float X = 2.0f / (right-left);
    float Y = 2.0f / (top-bottom);
    float Z = 2.0f / (nearVal-farVal);
    float tx = -(right+left) / (right-left);
    float ty = -(top+bottom) / (top-bottom);
    float tz = (nearVal+farVal) / (nearVal-farVal);
    mat4t mul(
      vec4( X, 0, 0, tx ),
      vec4( 0, Y, 0, ty ),
      vec4( 0, 0, Z, tz ),
      vec4( 0, 0, 0,  1 )
    );
    *this = mul * *this;
    return *this;
  }
  
  // multiply by scalar
  mat4t operator*(float r) const { return mat4t(v[0]*r, v[1]*r, v[2]*r, v[3]*r); }

  // postmultiply vector
  vec4 operator*(const vec4 &r) const {
    return vec4(
      v[0].dot(r),
      v[1].dot(r),
      v[2].dot(r),
      v[3].dot(r)
    );
  }

  // convert rotation matrix to quaternion
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
  
  const char *toString() const
  {
    static char buf[4][256];
    static int i = 0;
    char *dest = buf[i++&3];
    sprintf(dest, "{%s %s %s %s}", v[0].toString(), v[1].toString(), v[2].toString(), v[3].toString());
    return dest;
  }

  /*void dump() {
    printf("{"); v[0].dump(); printf(", "); v[1].dump(); printf(", "); v[2].dump(); printf(", "); v[3].dump(); printf("}\n");
  }*/
  // helper function for building a simple camera
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

  // helper function for building a simple camera
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

  mat4t xy() const { return mat4t(v[0], v[1], vec4(0, 0, 0, 0), vec4(0, 0, 0, 0)); }
  mat4t xyz() const { return mat4t(v[0], v[1], v[2], vec4(0, 0, 0, 0)); }
  mat4t xyz1() const { return mat4t(v[0], v[1], v[2], vec4(0, 0, 0, 1)); }
  vec4 &x() { return v[0]; }
  vec4 &y() { return v[1]; }
  vec4 &z() { return v[2]; }
  vec4 &w() { return v[3]; }
};

// vector times a matrix (premultiplication)
inline vec4 vec4::operator*(const mat4t &r) const
{
  return r.lmul(*this);
}
