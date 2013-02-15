////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// 4x4 matrix class
//
//
class mat4 {
  vec4 v[4];
  static const char *Copyright() { return "Copyright(C) Andy Thomason 2012"; }
public:
  mat4() {}
  mat4(const vec4 &x, const vec4 &y, const vec4 &z, const vec4 &w)
  {
    v[0] = x; v[1] = y; v[2] = z; v[3] = w;
  }
  
  mat4(const quat &r)
  {
    // http://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation
    float a = r[3], b = r[0], c = r[1], d = r[2];
    v[0] = vec4( a*a + b*b - c*c - d*d, 2 * ( b*c + a*d ), 2 * ( b*d - a*c ), 0 ),
    v[1] = vec4( 2 * ( b*c - a*d ), a*a - b*b + c*c - d*d, 2 * ( c*d + a*b ), 0 ),
    v[2] = vec4( 2 * ( b*d + a*c ), 2 * ( c*d - a*b ), a*a - b*b - c*c + d*d, 0 ),
    v[3] = vec4( 0, 0, 0, 1 );
  }

  mat4 &loadIdentity() {
    v[0][0] = 1; v[0][1] = 0; v[0][2] = 0; v[0][3] = 0;
    v[1][0] = 0; v[1][1] = 1; v[1][2] = 0; v[1][3] = 0;
    v[2][0] = 0; v[2][1] = 0; v[2][2] = 1; v[2][3] = 0;
    v[3][0] = 0; v[3][1] = 0; v[3][2] = 0; v[3][3] = 1;
    return *this;
  }

  vec4 &operator[](int i) { return v[i]; }
  const vec4 &operator[](int i) const { return v[i]; }

  vec4 row(int i) { return v[i]; }
  vec4 column(int i) { return vec4(v[0][i], v[1][i], v[2][i], v[3][i]); }
  const vec4 row(int i) const { return v[i]; }
  const vec4 column(int i) const { return vec4(v[0][i], v[1][i], v[2][i], v[3][i]); }

  float *get() { return &v[0][0]; }
  const float *get() const { return &v[0][0]; }
  
  mat4 &scale(float x, float y, float z) {
    for (int i = 0; i != 4; ++i) {
      v[i][0] *= x;
      v[i][1] *= y;
      v[i][2] *= z;
    }
    return *this;
  }
  
  mat4 &translate(float x, float y, float z) {
    v[3] = lmul(vec4(x,y,z,1));
    return *this;
  }
  
  mat4 operator*(const mat4 &r) const
  {
    mat4 res;
    for (int i = 0; i != 4; ++i) {
      res.v[i] = r[0] * v[i][0] + r[1] * v[i][1] + r[2] * v[i][2] + r[3] * v[i][3];
    }
    return res;
  }
  
  // generalized single axis rotate
  mat4 &rotate(float cosAngle, float sinAngle, int a, int b) {
    vec4 t = v[a] * cosAngle + v[b] * sinAngle;
    v[b] = v[b] * cosAngle - v[a] * sinAngle;
    v[a] = t;
    return *this;
  }

  // rotate by angle in degrees
  mat4 &rotateX(float angle) { return rotate(cosf(angle*(3.14159265f/180)), sinf(angle*(3.14159265f/180)), 1, 2); }
  mat4 &rotateY(float angle) { return rotate(cosf(angle*(3.14159265f/180)), sinf(angle*(3.14159265f/180)), 2, 0); }
  mat4 &rotateZ(float angle) { return rotate(cosf(angle*(3.14159265f/180)), sinf(angle*(3.14159265f/180)), 0, 1); }

  // accurate rotate by 90
  mat4 &rotateX90() { return rotate(0, 1, 1, 2); }
  mat4 &rotateY90() { return rotate(0, 1, 2, 0); }
  mat4 &rotateZ90() { return rotate(0, 1, 0, 1); }

  // accurate rotate by 180
  mat4 &rotateX180() { return rotate(-1, 0, 1, 2); }
  mat4 &rotateY180() { return rotate(-1, 0, 2, 0); }
  mat4 &rotateZ180() { return rotate(-1, 0, 0, 1); }

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
  void invertQuick(mat4 &d) const {
    // transpose x, y, z
    for (int i = 0; i != 3; ++i) {
      d[i] = vec4(v[0][i], v[1][i], v[2][i], 0);
    }
    d[3] = vec4(0, 0, 0, 1);
    // translate by new matrix
    d[3] = d.lmul(vec4(-v[3][0], -v[3][1], -v[3][2], 1));
  }

  // 3x3 adjoint matrix, used for generalized 3x3 invert
  mat4 adjoint3x3() const {
    vec4 c0 = column(0);
    vec4 c1 = column(1);
    vec4 c2 = column(2);
    return mat4(
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
  
  // general inverse of 3x3 matrix
  mat4 inverse3x3() const {
    return adjoint3x3() * (1.0f/det3x3());
  }
  
  // general inverse of 3x4 matrix
  mat4 inverse3x4() const {
    float rdet = 1.0f / det3x3();
    mat4 d = adjoint3x3();
    d[0] = d[0] * rdet;
    d[1] = d[1] * rdet;
    d[2] = d[2] * rdet;
    d[3] = d.lmul(vec4(-v[3][0], -v[3][1], -v[3][2], 1));
    return d;
  }
  
  // absolute of matrix useful for extents.
  mat4 abs() const { return mat4(v[0].abs(), v[1].abs(), v[2].abs(), v[3].abs()); }
  
  // as in glMultMatrix
  mat4 &multMatrix(const mat4 &r)
  {
    *this = r * *this;
    return *this;
  }
  
  // as in glFrustum - make a perspective matrix
  // used for conventional cameras
  mat4 &frustum(float left, float right, float bottom, float top, float nearVal, float farVal)
  {
    float X = 2*nearVal/(right-left);
    float Y = 2*nearVal/(top-bottom);
    float A = (right+left) / (right-left);
    float B = (top+bottom) / (top-bottom);
    float C = -(farVal+nearVal) / (farVal-nearVal);
    float D = -2*farVal*nearVal / (farVal-nearVal);
    mat4 mul(
      vec4( X, 0, 0,  0 ),
      vec4( 0, Y, 0,  0 ),
      vec4( A, B, C, -1 ),
      vec4( 0, 0, D,  0 )
    );
    *this = mul * *this;
    return *this;
  }
  
  // as in glOrtho - make a non-shrinking camera matrix
  // used for GUI displays and editors.
  mat4 &ortho(float left, float right, float bottom, float top, float nearVal, float farVal)
  {
    float X = 2.0f / (right-left);
    float Y = 2.0f / (top-bottom);
    float Z = 2.0f / (nearVal-farVal);
    float tx = -(right+left) / (right-left);
    float ty = -(top+bottom) / (top-bottom);
    float tz = (nearVal+farVal) / (nearVal-farVal);
    mat4 mul(
      vec4( X, 0, 0, tx ),
      vec4( 0, Y, 0, ty ),
      vec4( 0, 0, Z, tz ),
      vec4( 0, 0, 0,  1 )
    );
    *this = mul * *this;
    return *this;
  }
  
  // multiply by scalar
  mat4 operator*(float r) const { return mat4(v[0]*r, v[1]*r, v[2]*r, v[3]*r); }

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
  static mat4 build_projection_matrix(const mat4 &modelToWorld, const mat4 &cameraToWorld, float n = 0.1f, float f = 1000.0f)
  {
    // flip it around to transform from world to camera
    mat4 worldToCamera;
    cameraToWorld.invertQuick(worldToCamera);

    // build a projection matrix to add perspective
    mat4 cameraToProjection;
    cameraToProjection.loadIdentity();
    cameraToProjection.frustum(-n, n, -n, n, n, f);

    // model -> world -> camera -> projection
    return modelToWorld * worldToCamera * cameraToProjection;
  }

  // helper function for building a simple camera
  static mat4 build_camera_matrices(mat4 &modelToCamera, mat4 &worldToCamera, const mat4 &modelToWorld, const mat4 &cameraToWorld, float n = 0.1f, float f = 1000.0f)
  {
    // flip it around to transform from world to camera
    cameraToWorld.invertQuick(worldToCamera);

    modelToCamera = modelToWorld * worldToCamera;

    // build a projection matrix to add perspective
    mat4 cameraToProjection;
    cameraToProjection.loadIdentity();
    cameraToProjection.frustum(-n, n, -n, n, n, f);

    // model -> world -> camera -> projection
    return modelToCamera * cameraToProjection;
  }

  mat4 xy() const { return mat4(v[0], v[1], vec4(0, 0, 0, 0), vec4(0, 0, 0, 0)); }
  mat4 xyz() const { return mat4(v[0], v[1], v[2], vec4(0, 0, 0, 0)); }
  mat4 xyz1() const { return mat4(v[0], v[1], v[2], vec4(0, 0, 0, 1)); }
  vec4 &x() { return v[0]; }
  vec4 &y() { return v[1]; }
  vec4 &z() { return v[2]; }
  vec4 &w() { return v[3]; }
};

inline vec4 vec4::operator*(const mat4 &r) const
{
  return r.lmul(*this);
}
