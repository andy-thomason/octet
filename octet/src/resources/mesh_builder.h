////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// 3D mesh container
//
// mesh builder class for standard meshes.
//

namespace octet { namespace resources {
  /// Now deprecated mesh builder for spheres, cones etc. Being replaced by mesh_* classes
  class mesh_builder {
    struct vertex { float pos[3]; float normal[3]; float uv[2]; };
    dynarray<vertex, allocator> vertices;
    dynarray<unsigned short, allocator> indices;

    struct sphere {
      vec4 center;
      union {
        int children[2];
        int indices[3];
      };
      bool is_leaf;
    };

    dynarray<sphere> spheres;

    // current orientation and position of components
    mat4t matrix;

    // For a cube, add the front face. Matrix transforms are used to add the others.
    void add_front_face(float size) {
      unsigned short cur_vertex = (unsigned short)vertices.size();
      add_vertex(vec4(-size, -size, size, 1.0f), vec4(0.0f, 0.0f, 1.0f, 0.0f), 0.0f, 0.0f);
      add_vertex(vec4(-size,  size, size, 1.0f), vec4(0.0f, 0.0f, 1.0f, 0.0f), 0.0f, 1.0f);
      add_vertex(vec4( size,  size, size, 1.0f), vec4(0.0f, 0.0f, 1.0f, 0.0f), 1.0f, 1.0f);
      add_vertex(vec4( size, -size, size, 1.0f), vec4(0.0f, 0.0f, 1.0f, 0.0f), 1.0f, 0.0f);
      indices.push_back(cur_vertex+0);
      indices.push_back(cur_vertex+1);
      indices.push_back(cur_vertex+2);
      indices.push_back(cur_vertex+0);
      indices.push_back(cur_vertex+2);
      indices.push_back(cur_vertex+3);
    }

    // add a ring in the x-y plane. Return index of first index
    unsigned add_ring(float radius, const vec4 &normal, unsigned num_vertices, float v, float uvscale) {
      float rnv = 1.0f / num_vertices;
      float angle = 3.1415926536f * 2 * rnv;
      float delta_c = cosf(angle), delta_s = sinf(angle);
      mat4t save_matrix = matrix;
      unsigned first_index = (unsigned)vertices.size();
      float u = 0;
      for (unsigned i = 0; i <= num_vertices; ++i) {
        add_vertex(vec4(radius, 0.0f, 0.0f, 1.0f), normal, u, v);
        matrix.rotateSpecial(delta_c, delta_s, 0, 1);
        u += rnv * uvscale;
      }
      matrix = save_matrix;
      return first_index;
    }

    void add_cone_or_sphere(float radius, float height, unsigned slices, unsigned stacks, float uvscale, bool is_sphere) {
      float rstacks = 1.0f / stacks;

      mat4t save_matrix = matrix;

      // start at the south pole and work up
      matrix.translate(0, 0, -radius);
      float v = 0;
      unsigned prev_ring = 0;
      vec4 cone_normal = vec4(height, 0, radius, 0).normalize();

      if (!is_sphere) {
        // end cap for cone
        unsigned center = add_vertex(vec4(0, 0, 0, 1), vec4(0, 0, -1, 0), 0, 1);
        unsigned cur_ring = add_ring(radius, vec4(0, 0, -1, 0), slices, 0, uvscale);
        for (unsigned j = 0; j != slices; ++j) {
          indices.push_back(center);
          indices.push_back(cur_ring + j);
          indices.push_back(cur_ring + j + 1);
        }
      }

      for (unsigned i = 0; i <= stacks; ++i) {
        float c = cosf(i * rstacks * 3.1415926536f);
        float s = sinf(i * rstacks * 3.1415926536f);
        float ring_radius = is_sphere ? radius * s : radius * (stacks - i) * rstacks;
        float z = is_sphere ? (-radius) * c : height * (stacks - i) * rstacks;
        vec4 normal = is_sphere ? vec4(s, 0, -c, 0) : cone_normal;
        matrix = save_matrix;
        matrix.translate(0, 0, z);
        unsigned cur_ring = add_ring(ring_radius, normal, slices, v, uvscale);
        //printf("%d/%d z=%f r=%f\n", i, stacks, z, ring_radius);
        v += rstacks * radius * uvscale;
        if (i != 0) {
          for (unsigned j = 0; j != slices; ++j) {
            indices.push_back(prev_ring + j);
            indices.push_back(cur_ring + j);
            indices.push_back(cur_ring + j + 1);
            indices.push_back(prev_ring + j);
            indices.push_back(cur_ring + j + 1);
            indices.push_back(prev_ring + j + 1);
          }
        }
        prev_ring = cur_ring;
      }
      matrix = save_matrix;
    }

  public:
    mesh_builder() {
      init();
    }

    void init(int num_vertices=0, int num_indices=0) {
      vertices.resize(0);
      indices.resize(0);
      vertices.reserve(num_vertices);
      indices.reserve(num_indices);
      matrix.loadIdentity();
    }

    // add one vertex to the model
    unsigned add_vertex(const vec4 &pos, const vec4 &normal, float u, float v) {
      vec4 tpos = pos * matrix;
      vec4 tnormal = normal * matrix;
      vertex vtx = { tpos[0], tpos[1], tpos[2], tnormal[0], tnormal[1], tnormal[2], u, v };
      unsigned result = (unsigned)vertices.size();
      vertices.push_back(vtx);
      return result;
    }

    // add one index to the model
    void add_index(unsigned index) {
      indices.push_back(index);
    }

    // add a cube to the model at the current matrix location
    // as in glutSolidCube
    void add_cube(float size) {
      add_front_face(size);
      matrix.rotateY90();
      add_front_face(size);
      matrix.rotateY90();
      add_front_face(size);
      matrix.rotateY90();
      add_front_face(size);
      matrix.rotateY90();

      matrix.rotateX90();
      add_front_face(size);
      matrix.rotateX180();
      add_front_face(size);
      matrix.rotateX90();
    }

    // add a subdivided size*size plane with nx*ny squares
    void add_plane(float size, unsigned nx, unsigned ny) {
      float xsize = size / nx;
      float ysize = size / ny;
      float sizeBy2 = size * 0.5f;
      for (unsigned i = 0; i != nx; ++i) {
        for (unsigned j = 0; j != ny; ++j) {
          unsigned short cur_vertex = (unsigned short)vertices.size();
          add_vertex(vec4( i*xsize+sizeBy2, j*ysize+sizeBy2, 0, 1), vec4(0, 0, 1, 0), 0, 0);
          add_vertex(vec4( i*xsize+sizeBy2, (j+1)*ysize+sizeBy2, 0, 1), vec4(0, 0, 1, 0), 0, 1);
          add_vertex(vec4( (i+1)*xsize+sizeBy2, (j+1)*ysize+sizeBy2, 0, 1), vec4(0, 0, 1, 0), 1, 1);
          add_vertex(vec4( (i+1)*xsize+sizeBy2, j*ysize+sizeBy2, 0, 1), vec4(0, 0, 1, 0), 1, 0);
          indices.push_back(cur_vertex+0);
          indices.push_back(cur_vertex+1);
          indices.push_back(cur_vertex+2);
          indices.push_back(cur_vertex+0);
          indices.push_back(cur_vertex+2);
          indices.push_back(cur_vertex+3);
        }
      }
    }

    // add a sphere to the model at the current matrix location
    // as in glutSolidSphere.
    // This is not a very nice sphere so later I must add a geosphere.
    void add_sphere(float radius, unsigned slices, unsigned stacks, float uvscale=1) {
      add_cone_or_sphere(radius, radius*2.0f, slices, stacks, uvscale, true);
    }

    // add a cone to the model at the current matrix location
    // as in glutSolidCone.
    void add_cone(float radius, float height, unsigned slices, unsigned stacks, float uvscale=1) {
      add_cone_or_sphere(radius, height, slices, stacks, uvscale, false);
    }

    // get a mesh mesh from the builder either as VBOs or allocated memory.
    void get_mesh(scene::mesh &s);

    void scale(float x, float y, float z) {
      matrix.scale(x, y, z);
    }

    void translate(float x, float y, float z) {
      matrix.translate(x, y, z);
    }
  };
} }

