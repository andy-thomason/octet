////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Index modifier. Reduce vertices to the minimum set
//

namespace octet {
  class indexer : public mesh {

  public:
    struct vertex {
      const uint8_t *bytes;
      unsigned size;

      bool is_empty() const { return bytes == 0; }

      bool operator ==(const vertex &rhs) const {
        return size == rhs.size && memcmp(bytes, rhs.bytes, size) == 0;
      }

      unsigned get_hash() const {
        unsigned hash = 0;
        for (unsigned i = 0; i != size; ++i) {
          hash = ( hash >> 3 ) ^ ( hash >> 5 ) ^ bytes[i];
        }
        return hash;
      }
    };

    class vertex_cmp {
    public:
      static unsigned fuzz_hash(unsigned hash) { return hash ^ (hash >> 3) ^ (hash >> 5); }

      static unsigned get_hash(const vertex &key) { return fuzz_hash(key.get_hash()); }

      static bool is_empty(const vertex &key) { return key.is_empty(); }
    };
  private:
    // source mesh. Provides underlying geometry.
    ref<mesh> src;

  public:
    RESOURCE_META(indexer)

    indexer(mesh *src=0) {
      this->src = src;
      update();
    }

    void update() {
      if (!src) return;

      *(mesh*)this = *(mesh*)src;

      if (get_index_type() != GL_UNSIGNED_INT) return;

      hash_map<vertex, unsigned, vertex_cmp> vertex_to_index;

      dynarray<uint8_t> dest_vertices;
      dynarray<uint32_t> dest_indices;
      dest_indices.reserve(get_num_indices());

      //const uint32_t *ip = (const uint32_t*)get_indices()->lock_read_only();
      const uint8_t *vp = (const uint8_t*)get_vertices()->lock_read_only();
      unsigned stride = get_stride();
      unsigned num_vertices = 0;
      for (unsigned i = 0; i != get_num_indices(); ++i) {
        //uint32_t idx = ip[i];
        vertex v = { vp + i * stride, stride };
        unsigned &e = vertex_to_index[v];
        if (e == 0) { // hash_map inits to zero
          e = ++num_vertices;
          unsigned old_size = dest_vertices.size();
          dest_vertices.resize(old_size + stride);
          memcpy(&dest_vertices[old_size], vp + i * stride, stride);
        }
        dest_indices.push_back(e - 1);
      }
      get_vertices()->unlock_read_only();
      get_indices()->unlock_read_only();
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
    }

    void visit(visitor &v) {
      mesh::visit(v);
      v.visit(src, atom_src);
    }
  };

  inline unsigned get_hash(indexer::vertex vtx) { return vtx.get_hash(); }
}
