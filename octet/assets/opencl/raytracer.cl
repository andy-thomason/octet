
typedef struct tri_context {
  int dom_axis;
  float xplane, yplane, zplane;
  float pxa,  pya,  dxdya,  pxb,  pyb,  dxdyb;
  int x0p, x1p;
  int x0, x1, x2;
  int xmin, xmax;
  unsigned triangle;
  int width;
  float4 normal;
  __global unsigned char *image;

  // open addressing octree
  __global unsigned *octree;
  unsigned octree_size;
} tri_context;

// add a single voxel to the scene
void add_voxel(tri_context *tc, int x, int y, int z) {
  // if we have rasterized from the x or y axis views, swap the z coordinate.
  if (tc->dom_axis == 0) { int t = x; x = z; z = t; }
  else if (tc->dom_axis == 1) { int t = y; y = z; z = t; }

  // for debuging, just poke colours into the image.
  // note that x & y  must be positive
  if ( (unsigned)(x|y) < tc->width && tc->normal.z > 0) {
    tc->image[(y * tc->width + x) * 3 + 0] = tc->triangle * 16;
    tc->image[(y * tc->width + x) * 3 + 1] = tc->triangle & -16;
    tc->image[(y * tc->width + x) * 3 + 2] = tc->triangle >> 4 & -16;
  }

  if ((x|y|z) >= (1<<8)) return;

  //if (tc->octree[0] >= 65536) return;
  printf("%03x %03x %03x\n", x, y, z);
  //return;

  // traditional octree
  // entry 0 is current size of tree
  // entry 1..8 is the address of the next tree nodes
  unsigned index = 1;
  for (int i = 7; i > 0; --i) {
    __global unsigned *ptr = tc->octree + index;
    int child = ((x >> i) & 1) + ((y >> i) & 1) * 2 + ((z >> i) & 1) * 4;

    index = ptr[child];
    if (index == 0) {
      // in theory, only one thread will get this new address. Is this true?
      index = atom_add(tc->octree, 12);
      // attempt to replace address, but if we fail, use existing address
      //atom_cmpxchg(ptr + child, 0, index);
      ptr[child] = index;
      //unsigned old = atom_cmpxchg(ptr + child, 0, index);
      //if (old != 0) index = old;
    }
  }
  {
    __global unsigned *ptr = tc->octree + index;
    int child = (x & 1) + (y & 1) * 2 + (z & 1) * 4;

    index = ptr[child];
    if (index == 0) {
      // in theory, only one thread will get this new address. Is this true?
      index = atom_add(tc->octree, 4);
    }
  }
}

// generate a column of voxels
void gen_col(tri_context *tc, int x, int y, int z0, int z1) {
  for (int z = z0; z <= z1; ++z) {
    add_voxel(tc, x, y, z);
  }
}

// generate a line of columns
void gen_line(tri_context *tc, int x0, int x1, int y) {
  float k = (y * tc->yplane + tc->zplane);
  for (int x = x0; x <= x1; ++x) {
    float z00 = x * tc->xplane + k;
    float z10 = z00 + tc->xplane;
    float z01 = z00 + tc->yplane;
    float z11 = z10 + tc->yplane;
    int z0 = (int)min(min(z00, z01), min(z10, z11));
    int z1 = (int)max(max(z00, z01), max(z10, z11));
    gen_col(tc, x, y, z0, z1);
  }
}

// generate the top or bottom of a triangle in 3D voxels
void gen_half_triangle(tri_context *tc, int y0, int y1) {
  float fa = tc->pxa + ( (float)y0 - tc->pya ) * tc->dxdya;
  float fb = tc->pxb + ( (float)y0 - tc->pyb ) * tc->dxdyb;
  for (int y = y0; y < y1; ++y) {
    int x0n = min((int)fa, (int)fb);
    int x1n = max((int)fa, (int)fb);
    int x0 = min(tc->x0p, x0n);
    int x1 = max(tc->x1p, x1n);
    gen_line(tc, x0, x1, y-1);
    fa += tc->dxdya;
    fb += tc->dxdyb;
    tc->x0p = x0n;
    tc->x1p = x1n;
  }
}

// voxelise a 3D triangle
void gen_triangle(tri_context *tc, float x0f, float y0f, float x1f, float y1f, float x2f, float y2f) {
  if (y0f > y1f) { float tx = x0f, ty = y0f; x0f = x1f; y0f = y1f; x1f = tx; y1f = ty; }
  if (y1f > y2f) { float tx = x1f, ty = y1f; x1f = x2f; y1f = y2f; x2f = tx; y2f = ty; }
  if (y0f > y1f) { float tx = x0f, ty = y0f; x0f = x1f; y0f = y1f; x1f = tx; y1f = ty; }

  int y0 = (int)y0f;
  int y1 = (int)y1f;
  int y2 = (int)y2f;

  tc->x0 = (int)x0f;
  tc->x1 = (int)x1f;
  tc->x2 = (int)x2f;
  tc->xmin = min(tc->x0, min(tc->x1, tc->x2));
  tc->xmax = max(tc->x0, max(tc->x1, tc->x2));
        
  tc->pxb = x0f;
  tc->pyb = y0f;
  tc->dxdyb = (y2f - y0f) < 1e-6 ? 0 : (x2f - x0f) / (y2f - y0f); // experiment: remove ? guard

  tc->x0p = tc->x0;
  tc->x1p = tc->x0;

  if (y0 < y1) {
    tc->pxa = x0f;
    tc->pya = y0f;
    tc->dxdya = (y1f - y0f) < 1e-6 ? 0 : (x1f - x0f) / (y1f - y0f);
    gen_half_triangle(tc, y0+1, y1+1);
  }

  tc->x0p = min((int)x1f, tc->x0p);
  tc->x1p = max((int)x1f, tc->x1p);

  if (y1 < y2) {
    tc->pxa = x1f;
    tc->pya = y1f;
    tc->dxdya = (y2f - y1f) < 1e-6 ? 0 : (x2f - x1f) / (y2f - y1f);
    gen_half_triangle(tc, y1+1, y2+1);
  }

  tc->x0p = min((int)x2f, tc->x0p);
  tc->x1p = max((int)x2f, tc->x1p);

  gen_line(tc, tc->x0p, tc->x1p, y2);
}

__kernel void gen_voxels(
  __global unsigned char *image,
  int width,
  int height,
  __global const float *vertices,
  __global const unsigned short *indices,
  unsigned num_indices,
  unsigned stride,
  struct { float4 x, y, z, w; } modelToWorld,
  __global unsigned *octree
) {
  struct tri_context tc;
  tc.image = image;
  tc.width = width;
  tc.octree = octree;
  tc.octree_size = 0x100000;

  if (get_global_id(0) < 0 || get_global_id(0) > 0) return;

  //printf("gid=%d lid=%d\n", get_global_id(0), get_local_id(0));
  //printf("start\n");

  const int work_size = 32 * 3;
  unsigned imin = get_global_id(0) * work_size;
  unsigned imax = min(imin + work_size, num_indices);

  for (unsigned i = imin; i < imax; i += 3) {
    unsigned i0 = indices[i + 0];
    unsigned i1 = indices[i + 1];
    unsigned i2 = indices[i + 2];

    float3 p0 =
      vertices[i0*stride+0] * modelToWorld.x.xyz +
      vertices[i0*stride+1] * modelToWorld.y.xyz +
      vertices[i0*stride+2] * modelToWorld.z.xyz +
      modelToWorld.w.xyz
    ;

    float3 p1 =
      vertices[i1*stride+0] * modelToWorld.x.xyz +
      vertices[i1*stride+1] * modelToWorld.y.xyz +
      vertices[i1*stride+2] * modelToWorld.z.xyz +
      modelToWorld.w.xyz
    ;

    float3 p2 =
      vertices[i2*stride+0] * modelToWorld.x.xyz +
      vertices[i2*stride+1] * modelToWorld.y.xyz +
      vertices[i2*stride+2] * modelToWorld.z.xyz +
      modelToWorld.w.xyz
    ;

    //if (i0 == 96) printf("%3d %3d %3d [%f %f %f] [%f %f %f] [%f %f %f]\n", i0, i1, i2, p0.x, p0.y, p0.z, p1.x, p1.y, p1.z, p2.x, p2.y, p2.z);

    float3 normal = cross((p1 - p0), (p2 - p0));

    // do not draw degenerate triangles (zero or very small area)
    if (dot(normal, normal) > 1e-6) {
      normalize(normal);
      float nx = normal.x;
      float ny = normal.y;
      float nz = normal.z;
      float nw = -dot(normal, p0);
      float ax = fabs(nx), ay = fabs(ny), az = fabs(nz);
      tc.triangle = i; // todo: add mesh index
      tc.normal = (float4)(normal, nw);

      if (ax >= ay && ax >= az) {
        tc.dom_axis = 0;
        tc.xplane = nz * (-1.0f / nx);
        tc.yplane = ny * (-1.0f / nx);
        tc.zplane = nw * (-1.0f / nx);
        gen_triangle(&tc, p0.z, p0.y, p1.z, p1.y, p2.z, p2.y);
      } else if (ay >= ax && ay >= az) {
        tc.dom_axis = 1;
        tc.xplane = nx * (-1.0f / ny);
        tc.yplane = nz * (-1.0f / ny);
        tc.zplane = nw * (-1.0f / ny);
        gen_triangle(&tc, p0.x, p0.z, p1.x, p1.z, p2.x, p2.z);
      } else {
        tc.dom_axis = 2;
        tc.xplane = nx * (-1.0f / nz);
        tc.yplane = ny * (-1.0f / nz);
        tc.zplane = nw * (-1.0f / nz);
        gen_triangle(&tc, p0.x, p0.y, p1.x, p1.y, p2.x, p2.y);
      }
    }
  }
}

