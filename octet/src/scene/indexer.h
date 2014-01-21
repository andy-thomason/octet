////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//

namespace octet { namespace scene {
  /// Mesh modifier to index a mesh. The meshes from Collada may not be correctly indexed
  /// and vertices may be duplicated. This modifier de-duplicates vertices.
  class indexer : public mesh {
    struct vertex {
      const uint8_t *bytes;
      unsigned size;

      bool is_empty() const { return bytes == 0; }

      bool operator ==(const vertex &rhs) const {
        //printf("%p %p %d\n", this, &rhs, size == rhs.size && memcmp(bytes, rhs.bytes, size) == 0);
        return size == rhs.size && memcmp(bytes, rhs.bytes, size) == 0;
      }

      unsigned get_hash() const {
        unsigned hash = 0;
        for (unsigned i = 0; i != size; ++i) {
          hash = ( hash * 7 ) + ( hash >> 13 ) + bytes[i];
          //printf("%02x ", bytes[i]);
        }
        //printf("%d hash=%08x\n", size, hash_map_cmp::fuzz_hash(hash));
        return hash;
      }
    };

    class vertex_cmp : public hash_map_cmp {
    public:
      static unsigned get_hash(const vertex &key) { return fuzz_hash(key.get_hash()); }
      static bool is_empty(const vertex &key) { return key.is_empty(); }
    };

    // source mesh. Provides underlying geometry.
    ref<mesh> src;

  public:
    RESOURCE_META(indexer)

    /// Construct a mesh indexer from a mesh.
    indexer(mesh *src=0) {
      this->src = src;
      update();
    }

    /// standard update function, called if input changes.
    void update() {
      if (!src) return;

      *(mesh*)this = *(mesh*)src;

      if (get_index_type() != GL_UNSIGNED_INT) return;

      hash_map<vertex, unsigned, vertex_cmp> vertex_to_index;

      dynarray<uint8_t> dest_vertices;
      dynarray<uint32_t> dest_indices;
      dest_indices.reserve(get_num_indices());

      gl_resource::rolock idx_lock(get_indices());
      gl_resource::rolock vtx_lock(get_vertices());
      const uint32_t *ip = idx_lock.u32();
      const uint8_t *vp = vtx_lock.u8();

      unsigned stride = get_stride();
      unsigned num_vertices = 0;
      for (unsigned i = 0; i != get_num_indices(); ++i) {
        uint32_t idx = ip[i];
        vertex v = { vp + idx * stride, stride };
        unsigned &e = vertex_to_index[v];
        //printf("i=%d idx=%d e=%d\n", i, idx, e);
        if (e == 0) { // hash_map inits to zero
          e = ++num_vertices;
          unsigned old_size = dest_vertices.size();
          dest_vertices.resize(old_size + stride);
          memcpy(&dest_vertices[old_size], vp + idx * stride, stride);
        }
        dest_indices.push_back(e - 1);
        //printf("di=%d\n", e-1);
      }
      //printf("%d/%d\n", get_num_vertices(), num_vertices);

      unsigned isize = dest_indices.size() * sizeof(dest_indices[0]);
      unsigned vsize = dest_vertices.size() * sizeof(dest_vertices[0]);
      gl_resource *indices = new gl_resource(GL_ELEMENT_ARRAY_BUFFER, isize);
      gl_resource *vertices = new gl_resource(GL_ARRAY_BUFFER, vsize);
      indices->assign(&dest_indices[0], 0, isize);
      vertices->assign(&dest_vertices[0], 0, vsize);

      set_indices(indices);
      set_vertices(vertices);
      set_num_vertices(num_vertices);

      //this->dump(log("dump\n"));
    }

    /// Serialization, scripts, web access
    void visit(visitor &v) {
      mesh::visit(v);
      v.visit(src, atom_src);
    }
  };
}}
