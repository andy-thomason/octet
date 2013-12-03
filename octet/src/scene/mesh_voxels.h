////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Mesh smooth modifier. Work in progress.
//

namespace octet {
  class mesh_voxels : public mesh {
    ivec3 size;
    float voxel_size;

    enum { subcube_dim = 32 };

    dynarray<ref<mesh_voxel_subcube>> subcubes;

    void update_lod() {
      for (unsigned i = 0; i != subcubes.size(); ++i) {
        mesh_voxel_subcube *p = subcubes[i];
        if (p) {
          p->update_lod();
        }
      }
    }

    void update_mesh() {
      mesh_iterate_faces<face_counter, subcube_dim> count;
      for (unsigned i = 0; i != subcubes.size(); ++i) {
        mesh_voxel_subcube *p = subcubes[i];
        if (p) {
          p->count_faces(count);
        }
      }

      allocate(sizeof(vertex)*count.num_faces*4, sizeof(uint32_t)*count.num_faces*6);
      set_num_indices(count.num_faces*6);
      set_num_vertices(count.num_faces*4);

      mesh_iterate_faces<face_adder, subcube_dim> add;
      add.vtx = (vertex *)get_vertices()->lock();
      add.idx = (uint32_t *)get_indices()->lock();
      add.dx = vec3(voxel_size, 0.0f, 0.0f);
      add.dy = vec3(0.0f, voxel_size, 0.0f);
      add.dz = vec3(0.0f, 0.0f, voxel_size);
      add.voxel_size = voxel_size;

      vec3 offset = vec3(size) * (-0.5f * subcube_dim * voxel_size);
      vec3 scale(subcube_dim * voxel_size);
      int idx = 0;
      for (int z = 0; z != size.z(); ++z) {
        for (int y = 0; y != size.y(); ++y) {
          for (int x = 0; x != size.x(); ++x) {
            mesh_voxel_subcube *p = subcubes[idx++];
            add.origin = vec3(x, y, z) * scale + offset;
            if (p) {
              p->add_faces(add);
            }
          }
        }
      }

      assert(count.num_faces == add.num_faces);

      get_vertices()->unlock();
      get_indices()->unlock();
      //dump(app_utils::log("voxels\n"));
    }

  public:
    RESOURCE_META(mesh_voxels)

    mesh_voxels(float voxel_size_in=1.0f/32, const ivec3 &size_in = ivec3(2, 2, 2)) {
      set_default_attributes();
      voxel_size = voxel_size_in;
      size = size_in;
      //set_aabb(aabb(vec3(0, 0, 0), size));

      subcubes.resize(size.x() * size.y() * size.z());
      int idx = 0;
      for (int z = 0; z != size.z(); ++z) {
        for (int y = 0; y != size.y(); ++y) {
          for (int x = 0; x != size.x(); ++x) {
            ivec3 pos(x, y, z);
            subcubes[idx++] = new mesh_voxel_subcube();
          }
        }
      }

      //box(aabb(vec3(8, 8, 8), vec3(8, 8, 8)));
      update_lod();
    }

    void update() {
      update_lod();
      update_mesh();
    }

    void visit(visitor &v) {
      mesh::visit(v);
    }

    template <class set> void add_voxels(mat4t_in voxelToWorld, const set &set_in) {
      int idx = 0;
      vec3 offset = vec3(size) * (-0.5f * subcube_dim) + vec3(0.5f);
      vec3 scale = vec3(subcube_dim);
      for (int z = 0; z != size.z(); ++z) {
        for (int y = 0; y != size.y(); ++y) {
          for (int x = 0; x != size.x(); ++x) {
            mat4t localVoxelToWorld = voxelToWorld;
            vec3 pos = vec3(x, y, z) * scale + offset;
            localVoxelToWorld.translate(pos.x(), pos.y(), pos.z());
            //localVoxelToWorld.w() += vec4(0.5f, 0.5f, 0.5f, 0.0f);
            subcubes[idx++]->add_voxels(localVoxelToWorld, set_in);
          }
        }
      }
    }

    /*mesh_voxels &cylinder(vec3_in centre, vec3_in axis, float radius, float half_length) {
      float r2 = radius * radius;
      for (int z = 0; z != dim; ++z) {
        for (int y = 0; y != dim; ++y) {
          for (int x = 0; x != dim; ++x) {
            vec3 pos = vec3(x-dim/2, y-dim/2, z-dim/2) - centre;
            float adotp = dot(pos, axis);
            vec3 nearest = axis * adotp;
            float d2 = squared(nearest - pos);
            if (d2 <= r2 && abs(adotp) <= half_length) {
              opaque[z*dim+y] |= 1<<x;
            }
          }
        }
      }
      return *this;
    }

    mesh_voxels &sphere(vec3_in centre, float radius) {
      float r2 = radius * radius;
      for (int z = 0; z != dim; ++z) {
        for (int y = 0; y != dim; ++y) {
          for (int x = 0; x != dim; ++x) {
            vec3 pos = vec3(x-dim/2, y-dim/2, z-dim/2) - centre;
            float d2 = squared(pos);
            if (d2 <= r2) {
              opaque[z*dim+y] |= 1<<x;
            }
          }
        }
      }
      return *this;
    }*/


    mesh_voxels &box(mat4t_in voxelToWorld, aabb_in bounds) {
      add_voxels(voxelToWorld, bounds);
      return *this;
    }

    void dump(FILE *fp) {
      int idx = 0;
      for (int z = 0; z != size.z(); ++z) {
        for (int y = 0; y != size.y(); ++y) {
          for (int x = 0; x != size.x(); ++x, idx++) {
            fprintf(fp, "\n%d %d %d\n", x, y, z);
            if (subcubes[idx]) {
              subcubes[idx]->dump(fp);
            }
          }
        }
      }
      mesh::dump(fp);
    }
  };
}
