////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet { namespace scene {
  /// General mesh class. This is a base class for all meshes such as mesh_box.
  /// Meshes may be dynamic or static.
  class mesh : public resource {
  public:
    // default vertex format
    struct vertex {
      vec3p pos;
      vec3p normal;
      vec2p uv;

      vertex() {
      }

      vertex(vec3_in pos, vec3_in normal, vec3_in uvw) {
        this->pos = pos;
        this->normal = normal;
        this->uv = uvw.xy();
      }
    };

    // sortable edge
    struct edge {
      int32_t idx0;
      int32_t idx1;
      int32_t tri0;
      int32_t tri1;
      bool operator<(const edge &rhs) const { return idx0 == rhs.idx0 ? idx1 < rhs.idx1 : idx0 < rhs.idx0; }
    };
  private:
    ref<gl_resource> vertices;
    ref<gl_resource> indices;

    // attribute formats
    enum { max_slots = 16 };
    uint32_t format[max_slots];

    uint32_t num_indices;
    uint32_t num_vertices;
    uint32_t first_index;
    uint16_t stride;
    uint16_t mode;
    uint16_t index_type;
    uint16_t normalized;

    uint8_t num_slots;

    // optional skin
    ref<skin> mesh_skin;
    
    // bounding box
    aabb mesh_aabb;

    struct general_vertex {
      const uint8_t *bytes;
      unsigned size;

      bool is_empty() const { return bytes == 0; }

      bool operator ==(const general_vertex &rhs) const {
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
      static unsigned get_hash(const general_vertex &key) { return fuzz_hash(key.get_hash()); }
      static bool is_empty(const general_vertex &key) { return key.is_empty(); }
    };

    // add a new edge to a hash map. (index, index) -> (triangle+1, triangle+1)
    static void add_edge(dynarray<edge> &edges, unsigned tri_idx, unsigned i0, unsigned i1) {
      edge e = { std::min(i0, i1), std::max(i0, i1), tri_idx, ~0 };
      edges.push_back(e);
    }

    // return true if the triangle tri is visible from the viewpoint (in model space)
    // the exact definition of "is visible" depends on winding order.
    static bool tri_is_visible(
      unsigned tri, unsigned pos_offset, unsigned stride,
      const uint32_t *ip, const uint8_t *vp, const vec3 &viewpoint,
      bool is_directional
    ) {
      const vec3p &pa = *(const vec3p*)(vp + ip[tri+0] * stride + pos_offset);
      const vec3p &pb = *(const vec3p*)(vp + ip[tri+1] * stride + pos_offset);
      const vec3p &pc = *(const vec3p*)(vp + ip[tri+2] * stride + pos_offset);
      vec3 normal = cross(((vec3)pb - pa), ((vec3)pc - pa));
      vec3 dir = is_directional ? viewpoint - pa : viewpoint;
      return dot(normal, dir) <= 0;
    }

    /// Get a vec4 value of an attribute.
    vec4 get_value(const uint8_t *bytes, unsigned slot, unsigned index) const {
      unsigned size = get_size(slot);
      vec4 result = vec4(0, 0, 0, 0);
      bytes += stride * index + get_offset(slot);
    
      switch (get_kind(slot)) {
        case GL_FLOAT: {
          const float *src = (const float*)(bytes);
          result = vec4(src[0], size > 1 ? src[1] : 0, size > 2 ? src[2] : 0, size > 3 ? src[3] : 1);
     	  } break;
        case GL_BYTE: {
          const int8_t *src = (const int8_t*)(bytes);
          result = vec4((float)src[0], size > 1 ? (float)src[1] : 0, size > 2 ? (float)src[2] : 0, size > 3 ? (float)src[3] : 255) * (1.0f/255);
     	  } break;
        case GL_UNSIGNED_BYTE: {
          const uint8_t *src = (const uint8_t*)(bytes);
          result = vec4((float)src[0], size > 1 ? (float)src[1] : 0, size > 2 ? (float)src[2] : 0, size > 3 ? (float)src[3] : 255) * (1.0f/255);
     	  } break;
        case GL_SHORT: {
          const int16_t *src = (const int16_t*)(bytes);
          result = vec4((float)src[0], size > 1 ? (float)src[1] : 0, size > 2 ? (float)src[2] : 0, size > 3 ? (float)src[3] : 0xffff) * (1.0f/0xffff);
     	  } break;
        case GL_UNSIGNED_SHORT: {
          const uint16_t *src = (const uint16_t*)(bytes);
          result = vec4((float)src[0], size > 1 ? (float)src[1] : 0, size > 2 ? (float)src[2] : 0, size > 3 ? (float)src[3] : 0xffff) * (1.0f/0xffff);
     	  } break;
        case GL_INT: {
          const int32_t *src = (const int32_t*)(bytes);
          result = vec4((float)src[0], size > 1 ? (float)src[1] : 0, size > 2 ? (float)src[2] : 0, size > 3 ? (float)src[3] : 0xffff) * (1.0f/0xffff);
     	  } break;
        case GL_UNSIGNED_INT: {
          const uint32_t *src = (const uint32_t*)(bytes);
          result = vec4((float)src[0], size > 1 ? (float)src[1] : 0, size > 2 ? (float)src[2] : 0, size > 3 ? (float)src[3] : 0xffff) * (1.0f/0xffff);
     	  } break;
      }
      return result;
    }

  public:
    RESOURCE_META(mesh)

    /// make a new, empty, mesh.
    mesh(skin *_skin=0) {
      init(_skin, 0, 0);
    }

    mesh(unsigned num_vertices, unsigned num_indices) {
      init(0, num_vertices, num_indices);
    }

    /// clone a mesh. Note that this does not also clone the vertices and indices.
    mesh(const mesh &rhs) {
      vertices = rhs.vertices;
      indices = rhs.indices;

      memcpy(format, rhs.format, sizeof(format));

      num_indices = rhs.num_indices;
      num_vertices = rhs.num_vertices;
      first_index = rhs.first_index;
      stride = rhs.stride;
      mode = rhs.mode;
      index_type = rhs.index_type;
      normalized = rhs.normalized;

      num_slots = rhs.num_slots;
      index_type = rhs.index_type;
      mode = rhs.mode;

      mesh_skin = rhs.mesh_skin;
    }

    /// Init function used for aggregated meshes.
    void init(skin *_skin=0, unsigned max_vertices=0, unsigned max_indices=0) {
      vertices = new gl_resource();
      indices = new gl_resource();

      memset(format, 0, sizeof(format));

      num_indices = 0;
      num_vertices = 0;
      first_index = 0;
      stride = 0;
      mode = 0;
      index_type = 0;
      normalized = 0;

      num_slots = 0;
      index_type = GL_UNSIGNED_SHORT;
      mode = GL_TRIANGLES;

      mesh_skin = _skin;

      if (max_vertices || max_indices) {
        set_default_attributes();
        allocate(max_vertices * sizeof(vertex), max_indices * sizeof(uint32_t));
      }
    }

    /// Serialize.
    void visit(visitor &v) {
      v.visit(vertices, atom_vertices);
      v.visit(indices, atom_indices);
      v.visit(format, atom_format);
      v.visit(num_indices, atom_num_indices);
      v.visit(num_vertices, atom_num_vertices);
      v.visit(first_index, atom_first_index);
      v.visit(stride, atom_stride);
      v.visit(mode, atom_mode);
      v.visit(index_type, atom_index_type);
      v.visit(normalized, atom_normalized);
      v.visit(num_slots, atom_num_slots);
      v.visit(mesh_skin, atom_mesh_skin);
      v.visit(mesh_aabb, atom_aabb);
    }

    // Destructor
    ~mesh() {
    }

    /// Set the defuault mesh parameters, used for boxes, spheres etc.
    void set_default_attributes() {
      add_attribute(attribute_pos, 3, GL_FLOAT, 0);
      add_attribute(attribute_normal, 3, GL_FLOAT, 12);
      add_attribute(attribute_uv, 2, GL_FLOAT, 24);
      set_params(32, 0, 0, GL_TRIANGLES, GL_UNSIGNED_INT);
    }

    /// reset the mesh to empty.
    void clear_attributes() {
      num_slots = 0;
    }

    /// Add an extra attribute to the mesh. eg. add_attribute(attribute_pos, 3, GL_FLOAT, 0)
    unsigned add_attribute(unsigned attr, unsigned size, unsigned kind, unsigned offset, unsigned norm=0) {
      assert(num_slots < max_slots);
      format[num_slots] = (offset << 9) + (attr << 5) + ((size-1) << 3) + (kind - GL_BYTE);
      if (norm) normalized |= 1 << num_slots;
      return num_slots++;
    }

    /// helper function: how many bytes does this GL_? type use?
    static unsigned kind_size(unsigned kind) {
      static const uint8_t bytes[] = { 1, 1, 2, 2, 4, 4, 4, 4 };
      return kind < GL_BYTE || kind > GL_FLOAT ? 0 : bytes[kind - GL_BYTE];
    }

    /// For a particular slot, get the offset in the vertex buffer of the first attribute.
    unsigned get_offset(unsigned slot) const {
      return ( format[slot] >> 9 ) & 0x3f;
    }

    /// For a particular slot, get the attribute used by the shader.
    unsigned get_attr(unsigned slot) const {
      return ( format[slot] >> 5 ) & 0x0f;
    }

    /// For a particular slot, get the number of lanes in the attribute.
    unsigned get_size(unsigned slot) const {
      return ( ( format[slot] >> 3 ) & 0x03 ) + 1;
    }

    /// For a particular slot, get the GL kind of the attribute (eg. GL_FLOAT)
    unsigned get_kind(unsigned slot) const {
      return ( ( format[slot] >> 0 ) & 0x07 ) + GL_BYTE;
    }

    /// Get the stride of attributes in this mesh.
    unsigned get_stride() const {
      return stride;
    }

    /// Get the number of vertices to be drawn. (actual number may be higher than this).
    unsigned get_num_vertices() const {
      return num_vertices;
    }

    /// Get the number of indices to be drawn and hence the number of primitives.
    unsigned get_num_indices() const {
      return num_indices;
    }

    /// Get the first index that we will render
    unsigned get_first_index() const {
      return first_index;
    }

    /// Get the kind of primitive we are drawing. (ie. GL_TRIANGLES etc.)
    unsigned get_mode() const {
      return mode;
    }

    /// get the type of the indices (ie. GL_UNSIGNED_SHORT/GL_UNSIGNED_INT)
    unsigned get_index_type() const {
      return index_type;
    }

    /// get the size of an index element in bytes.
    size_t get_index_size() const {
      return index_type == GL_UNSIGNED_SHORT ? 2 : 4;
    }

    /// Set the kind of index to use (0 means use glDrawArrays)
    void set_index_type(unsigned value) {
      assert(value == 0 || value == GL_UNSIGNED_SHORT || value == GL_UNSIGNED_INT);
      index_type = value;
    }

    /// Get the number of slots (attributes) we have.
    unsigned get_num_slots() const {
      return num_slots;
    }

    /// Get the optional skin data
    skin *get_skin() const {
      return (skin*)mesh_skin;
    }

    /// Set the number of vertices to draw. (may be smaller that the buffer size).
    void set_num_vertices(unsigned value) {
      num_vertices = value;
    }

    /// Set the number of indices to draw. (may be smaller that the buffer size).
    void set_num_indices(unsigned value) {
      num_indices = value;
    }

    /// Set the first index to draw.
    void set_first_index(unsigned value) {
      first_index = value;
    }

    /// Set the kind of primitive to draw. (ie. GL_TRIANGLES etc.)
    void set_mode(unsigned value) {
      assert(value >= GL_POINTS && value <= GL_POLYGON);
      mode = value;
    }

    /// set the optional skin
    /// note: the mesh state owns the skin.
    void set_skin(skin *value) {
      mesh_skin = value;
    }
    
    /// set the axis aligned bounding box of the untransformed mesh
    void set_aabb(const aabb &value) {
      mesh_aabb = value;
    }

    /// get the axis aligned bounding box of the untransformed mesh
    aabb get_aabb() {
      return mesh_aabb;
    }

    /// return true if this mesh has a particular attribute. eg. attribute_pos
    bool has_attribute(unsigned attr) {
      for (unsigned i = 0; i != num_slots; ++i) {
        if (get_attr(i) == attr) {
          return true;
        }
      }
      return false;
    }

    #ifdef OCTET_BULLET
      /// Get a bullet shape object for this mesh
      virtual btCollisionShape *get_bullet_shape() {
        return NULL;
      }

      /// Get a bullet shape object for this mesh for static use only!
      virtual btCollisionShape *get_static_bullet_shape() {
        // note that it is your responsibility to deallocate resources!
        btIndexedMesh mesh;
        mesh.m_numTriangles = get_num_indices() / 3;
        mesh.m_triangleIndexBase = (const unsigned char *)malloc(get_indices()->get_size());
        mesh.m_triangleIndexStride = sizeof(uint32_t) * 3;
        mesh.m_numVertices = get_num_vertices();
        mesh.m_vertexBase = (const unsigned char *)malloc(get_vertices()->get_size());
        mesh.m_vertexStride = get_stride();

        {
          gl_resource::rolock idx_lock(get_indices());
          gl_resource::rolock vtx_lock(get_vertices());
          memcpy((void*)mesh.m_triangleIndexBase, idx_lock.u8() + get_index_size() * first_index, get_indices()->get_size());
          memcpy((void*)mesh.m_vertexBase, vtx_lock.u8(), get_vertices()->get_size());
        }

        btTriangleIndexVertexArray *trimesh = new btTriangleIndexVertexArray();
        trimesh->addIndexedMesh(mesh);
        btBvhTriangleMeshShape *result = new btBvhTriangleMeshShape(trimesh, true);
        return result;
      }
    #endif

    /// get which slot a particular attribute is in. (does a search).
    unsigned get_slot(unsigned attr) const {
      for (unsigned i = 0; i != max_slots; ++i) {
        if (!format[i]) break;
        if (get_attr(i) == attr) {
          return i;
        }
      }
      return ~0;
    }

    /// Get an index value from the index buffer object.
    unsigned get_index(const uint8_t *bytes, unsigned index) const {
      unsigned result = 0;
      if (index_type == GL_UNSIGNED_SHORT) {
        uint16_t *src = (uint16_t*)((uint8_t*)bytes + (first_index + index)*2);
        result = *src;
      } else if (index_type == GL_UNSIGNED_INT) {
        unsigned int *src = (unsigned int*)((uint8_t*)bytes + (first_index + index)*4);
        result = *src;
      }
      return result;
    }

    /// Allocate VBO and IBO objects together.
    void allocate(size_t vsize, size_t isize) {
      vertices->allocate(GL_ARRAY_BUFFER, vsize);
      indices->allocate(GL_ELEMENT_ARRAY_BUFFER, isize);
    }

    /// allocate and assign data to IBO and VBO
    void assign(size_t vsize, size_t isize, uint8_t *vsrc, uint8_t *isrc) {
      vertices->assign(vsrc, 0, vsize);
      indices->assign(isrc, 0, isize);
    }

    /// set standard parameters of the mesh together.
    void set_params(size_t stride_, size_t num_indices_, size_t num_vertices_, unsigned mode_, unsigned index_type_) {
      stride = (uint16_t)stride_;
      num_indices = (uint32_t)num_indices_;
      num_vertices = (uint32_t)num_vertices_;
      mode = mode_;
      index_type = index_type_;
    }

    /// dump the mesh to a file in ASCII. Used to debug mesh transforms.
    void dump(FILE *file) {
      gl_resource::rolock vtx_lock(get_vertices());
      gl_resource::rolock idx_lock(get_indices());

      fprintf(file, "<model mode=%04x index_type=%04x stride=%d>\n", mode, index_type, stride);
      for (unsigned slot = 0; slot != num_slots; ++slot) {
        fprintf(file, "  <slot n=%d attr=%d kind=%04x size=%d offset=%d>\n", slot, get_attr(slot), get_kind(slot), get_size(slot), get_offset(slot)); 
        const char *fmt[] = { "", "    [%d %f]\n", "    [%d %f %f]\n", "    [%d %f %f %f]\n", "    [%d %f %f %f %f]\n" };
        for (unsigned i = 0; i != num_vertices; ++i) {
          vec4 value = get_value(vtx_lock.u8(), slot, i);
          fprintf(file, fmt[get_size(slot)], i, value[0], value[1], value[2], value[3]);
        }
        fprintf(file, "  </slot>\n");
      }
      fprintf(file, "  <indices>\n    ");
      unsigned ni = get_num_indices();
      for (unsigned i = 0; i != ni; ++i) {
        fprintf(file, "%d ", get_index(idx_lock.u8(), i));
      }
      fprintf(file, "\n  </indices>\n");
      fprintf(file, "</model>\n");
    }

    /// When rendering a mesh, call this first to enable the attributes.
    /// assume the shader, uniforms and render params are already set up.
    void enable_attributes() const {
      vertices->bind();

      unsigned n = normalized;
      for (unsigned slot = 0; slot != get_num_slots(); ++slot) {
        unsigned size = get_size(slot);
        unsigned kind = get_kind(slot);
        unsigned attr = get_attr(slot);
        unsigned offset = get_offset(slot);
        glVertexAttribPointer(attr, size, kind, n & 1, get_stride(), (void*)(offset));
        glEnableVertexAttribArray(attr);
        n >>= 1;
      }
    }

    /// When rendering a mesh, call this next to draw the primitives.
    void draw() {
      //printf("de %04x %d %d\n", get_mode(), get_num_vertices(), get_index_type());
      if (get_index_type()) {
        indices->bind();
        glDrawElements(get_mode(), get_num_indices(), get_index_type(), (GLvoid*)(get_index_size() * first_index));
      } else {
        glDrawArrays(get_mode(), 0, get_num_vertices());
      }
    }

    /// When rendering a mesh, call this last to disable attributes.
    void disable_attributes() {
      for (unsigned slot = 0; slot != get_num_slots(); ++slot) {
        unsigned attr = get_attr(slot);
        glDisableVertexAttribArray(attr);
      }
    }

    /// render in one pass.
    void render() {
      enable_attributes();
      draw();
      disable_attributes();
    }

    /// Compute the axis aligned bounding box for this mesh in model space and set it.
    void calc_aabb() {
      unsigned num_vertices = get_num_vertices();
      if (get_num_vertices() == 0) {
        mesh_aabb = aabb();
        return;
      }

      gl_resource::rolock vtx_lock(get_vertices());

      unsigned slot = get_slot(attribute_pos);
      vec3 vmin = get_value(vtx_lock.u8(), slot, 0).xyz();
      vec3 vmax = vmin;
      for (unsigned i = 1; i < num_vertices; ++i) {
        vec3 pos = get_value(vtx_lock.u8(), slot, i).xyz();
        vmin = min(pos, vmin);
        vmax = max(pos, vmin);
      }
      mesh_aabb = aabb((vmax + vmin) * 0.5f, (vmax - vmin) * 0.5f);
    }

    /// *very* slow ray cast.
    /// returns "barycentric" coordinates.
    /// eg. hit pos = bary[0] * pos0 + bary[1] * pos1 + bary[2] * pos2 (or ray.start + ray.distance * bary[3])
    /// eg. hit uv = bary[0] * uv0 + bary[1] * uv1 + bary[2] * uv2
    bool ray_cast(const ray &the_ray, int indices[], vec4 &bary_numer, float &bary_denom) {
      unsigned pos_slot = get_slot(attribute_pos);
      if (get_index_type() != GL_UNSIGNED_INT) return false;
      if (get_size(pos_slot) < 3) return false;
      if (get_kind(pos_slot) != GL_FLOAT) return false;

      vec3 org = the_ray.get_start();
      vec3 dir = the_ray.get_distance();
      //log("ray_cast: org=%s dir=%s\n", org.toString(), dir.toString());

      unsigned pos_offset = get_offset(pos_slot);
      gl_resource::rolock idx_lock(get_indices());
      gl_resource::rolock vtx_lock(get_vertices());
      const uint32_t *idx = idx_lock.u32() + first_index;
      const uint8_t *vtx = vtx_lock.u8();

      float best_denom = 0;
      vec4 best_numer(0, 0, 0, 0);
      for (unsigned i = 0; i != get_num_indices(); i += 3) {
        vec3 a = (vec3)*(const vec3p*)(vtx + pos_offset + stride * idx[i+0]) - org;
        vec3 b = (vec3)*(const vec3p*)(vtx + pos_offset + stride * idx[i+1]) - org;
        vec3 c = (vec3)*(const vec3p*)(vtx + pos_offset + stride * idx[i+2]) - org;
        vec3 d = dir;

        // solve [ba, bb, bc, bd] * [[ax, ay, az, 1], [bx, by, bz, 1], [cx, cy, cz, 1], [-dx, -dy, -dz, 0]] = [0, 0, 0, 1]
        //
        // ie. ba + bb + bc = 1  and  ba * a + bb * b + bc * c = bd * d
        //
        // [ba, bb, bc] are barycentric coordinates, bd is the distance along the vector

        // The last line of the inverse matrix is the solution (vector triple products)

        // numerator
        vec4 numer(
          dot(cross(b, c), d),
          dot(cross(c, a), d),
          dot(cross(a, b), d),
          dot(cross(a, b), c)
        );

        // denominator
        float denom = numer[0] + numer[1] + numer[2];

        //log("a=%s b=%s c=%s d=%s denom=%f\n", a.toString(), b.toString(), c.toString(), d.toString(), denom);
        //log("numer=%s\n", numer.toString());
        //log("res=%s\n", (numer / denom).toString());

        // using a multiply lets us check the sign without using a divide.
        vec4 bary2 = numer * denom;

        if (all(bary2 >= vec4(0, 0, 0, 0))) {
          rational best_distance(best_numer[3], best_denom);
          rational new_distance(numer[3], denom);
          /*printf(
            "t%d %9.3f %d %d %d  %s %s %s\n",
            i, numer[3]/denom, idx[i+0], idx[i+1], idx[i+2],
            (numer/denom).toString(), best_distance.toString(), new_distance.toString()
          );*/

          unsigned further = new_distance > best_distance;
          if (!further) {
            indices[0] = idx[i+0];
            indices[1] = idx[i+1];
            indices[2] = idx[i+2];
            best_numer = numer;
            best_denom = denom;
          }
        }
      }

      if (fabsf(best_denom) < 1e-6f) {
        bary_numer = vec4(0, 0, 0, 0);
        bary_denom = 0;
        return false;
      } else {
        bary_numer = best_numer;
        bary_denom = best_denom;
        return true;
      }
    }

    /// access the vertex buffer (VBO) or memory buffer
    gl_resource *get_vertices() const {
      return vertices;
    }

    /// access the index buffer (IBO) or memory buffer
    gl_resource *get_indices() const {
      return indices;
    }

    /// set a new VBO object
    void set_vertices(gl_resource *value) {
      vertices = value;
    }

    /// assign a vector to the vertex buffer and set params
    template <class elem_t> void set_vertices(const dynarray<elem_t> &rhs) {
      if (!vertices || vertices->get_size() != rhs.size() * sizeof(elem_t)) {
        vertices = new gl_resource();
        vertices->allocate(GL_ARRAY_BUFFER, rhs.size() * sizeof(elem_t));
      }
      vertices->assign(rhs.data(), 0, rhs.size() * sizeof(elem_t));
      stride = sizeof(elem_t);
      set_num_vertices(rhs.size());
    }

    /// set a new IBO object
    void set_indices(gl_resource *value) {
      indices = value;
    }

    /// assign a vector to the index buffer and set params
    template <class elem_t> void set_indices(const dynarray<elem_t> &rhs) {
      if (!indices || indices->get_size() != rhs.size() * sizeof(elem_t)) {
        indices = new gl_resource();
        indices->allocate(GL_ELEMENT_ARRAY_BUFFER, rhs.size() * sizeof(elem_t));
      }
      indices->assign(rhs.data(), 0, rhs.size() * sizeof(elem_t));
      set_index_type(sizeof(elem_t) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT);
      set_num_indices(rhs.size());
      set_first_index(0);
    }

    /// Get all the edges in a hash map to avoid duplicates.
    /// record the triangle indices that they came from.
    void get_edges(dynarray<edge> &edges) {
      if (get_index_type() != GL_UNSIGNED_INT) return;

      gl_resource::rolock idx_lock(get_indices());
      const uint32_t *ip = idx_lock.u32();

      edges.resize(0);
      for (unsigned i = 0; i < get_num_indices(); i += 3) {
        add_edge(edges, i, ip[i+0], ip[i+1]);
        add_edge(edges, i, ip[i+1], ip[i+2]);
        add_edge(edges, i, ip[i+2], ip[i+0]);
      }

      std::sort(edges.data(), edges.data() + edges.size());

      size_t dest = 0, src = 0;
      for (; src < edges.size()-1; ) {
        edge &e0 = edges[src];
        edge &e1 = edges[src+1];
        if (dest != src && e0.idx0 == e1.idx0 && e0.idx1 == e1.idx1) {
          // combine two similar edges.
          edge &ed = edges[dest++];
          ed = e0;
          ed.tri1 = e1.tri0;
          src += 2;
        } else {
          edges[dest++] = e0;
          src++;
        }
      }

      if (src < edges.size()-1) {
        edges[dest++] = edges[src++];
      }
      edges.resize(dest);
    }

    /// Generate silhouette edges.
    /// Silhouette edges are used for shadows, highlighting and volumetric effects such as shadows.
    /// the resulting indices could be used for a GL_LINES render, for example.
    ///
    /// An edge is a silhouette edge if:
    ///
    ///   There is only one triangle that uses the edge.
    ///   One triangle can be seen from the viewpoint, the other can't.
    void get_silhouette_edges(const vec3 &viewpoint, bool is_directional, dynarray<edge> &edges) {
      unsigned pos_slot = get_slot(attribute_pos);
      if (get_index_type() != GL_UNSIGNED_INT) return;
      if (get_size(pos_slot) < 3) return;
      if (get_kind(pos_slot) != GL_FLOAT) return;

      unsigned pos_offset = get_offset(pos_slot);

      get_edges(edges);

      gl_resource::rolock idx_lock(get_indices());
      gl_resource::rolock vtx_lock(get_vertices());
      const uint32_t *ip = idx_lock.u32() + first_index;
      const uint8_t *vp = vtx_lock.u8();
      unsigned stride = get_stride();
      
      size_t dest = 0;
      for (size_t i = 0; i != edges.size(); ++i) {
        const edge &edge = edges[i];
        if (
          tri_is_visible(
            edge.tri0, pos_offset, stride, ip, vp, viewpoint, is_directional
          ) != tri_is_visible(
            edge.tri1, pos_offset, stride, ip, vp, viewpoint, is_directional
          )
        ) {
          edges[dest++] = edge;
        }
      }
      edges.resize(dest);
    }

    /// Debugging: show the effect of the vertex shader on the vertices
    /// If the vertices are outside [[-1,1], [-1,1], [-1,1]] then something is wrong!
    void dump_transformed(mat4t_in modelToProjection) {
      unsigned pos_offset = get_offset(get_slot(attribute_pos));
      gl_resource::rolock idx_lock(get_indices());
      gl_resource::rolock vtx_lock(get_vertices());
      const uint32_t *ip = idx_lock.u32();
      const uint8_t *vp = vtx_lock.u8();
      unsigned stride = get_stride();
      for (unsigned i = 0; i != get_num_indices(); ++i) {
        vec4 pos_in = vec4((vec3)*(const vec3p*)(vp + ip[i] * stride + pos_offset), 1.0f );
        vec4 pos_out = pos_in * modelToProjection;
        vec3 res = pos_out.perspectiveDivide();
        //vec3 ares = abs(res);
        bool err = any(abs(res) > vec3(1));
        char tmp[2][256];
        log("%5d %s -> %s %s\n", i, pos_in.toString(tmp[0], sizeof(tmp[0])), res.toString(tmp[1], sizeof(tmp[1])), err ? "FAIL" : "");
      }
    }

    /// Convert from GL_TRIANGLES to GL_LINES.
    /// Double the number of indices.
    void make_wireframe() {
      if (mode != GL_TRIANGLES) return;
      if (index_type != GL_UNSIGNED_INT) return;

      gl_resource::rolock idx_lock(get_indices());
      const uint32_t *sip = idx_lock.u32();
      gl_resource *new_indices = new gl_resource();
      new_indices->allocate(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * get_num_indices() * 2);
      gl_resource::wolock new_idx_lock(new_indices);
      uint32_t *dip = new_idx_lock.u32();

      for (unsigned i = 0; i < num_indices; i += 3) {
        dip[0] = dip[5] = sip[0];
        dip[2] = dip[1] = sip[1];
        dip[4] = dip[3] = sip[2];
        sip += 3;
        dip += 6;
      }
      set_indices(new_indices);
      set_num_indices(get_num_indices() * 2);
      set_mode(GL_LINES);
    }

    /// re-index the mesh
    void reindex() {
      if (get_index_type() != GL_UNSIGNED_INT) return;

      hash_map<general_vertex, unsigned, vertex_cmp> vertex_to_index;

      dynarray<uint8_t> dest_vertices;
      dynarray<uint32_t> dest_indices;
      dest_indices.reserve(get_num_indices());

      //The code below is inside a new scope { ... } with the purpose of be sure that outside the scope idx_lock will be deleted
      //  why do we want to delete idx_lock? When the object is created it locks indices to read only, and we want to unlock it after using it
      //  the unlocking of indices at the end of this scope will help us while trying to write to indices again
      {
        // This is the begining of a scope, every instance declared inside will be deleted at the end of the scope
        gl_resource::rolock idx_lock(get_indices());
        gl_resource::rolock vtx_lock(get_vertices());
        const uint32_t *ip = idx_lock.u32();
        const uint8_t *vp = vtx_lock.u8();

        unsigned stride = get_stride();
        unsigned num_vertices = 0;
        for (unsigned i = 0; i != get_num_indices(); ++i) {
          uint32_t idx = ip[i];
          general_vertex v = { vp + idx * stride, stride };
          unsigned &e = vertex_to_index[v];
          if (e == 0) { // hash_map inits to zero
            // vertex is unique.
            e = ++num_vertices;
            unsigned old_size = dest_vertices.size();
            dest_vertices.resize(old_size + stride);
            memcpy(&dest_vertices[old_size], vp + idx * stride, stride);
          }
          dest_indices.push_back(e - 1);
        }
      }

      // This is the end of the scope, not the end of the reindex() function, hence idx_lock, vtx_lock... will be deleted at this point
      //    and in the case of idx_lock (check gl_resources.h), it will unlock indices, letting us to write in it

      // if we have fewer vertices now, update the index and vertices.
      if (num_vertices != get_num_vertices()) {
        unsigned isize = dest_indices.size() * sizeof(uint32_t);
        unsigned vsize = dest_vertices.size() * sizeof(uint8_t);
        gl_resource *indices = get_indices();
        gl_resource *vertices = new gl_resource(GL_ARRAY_BUFFER, vsize);
        indices->assign(&dest_indices[0], 0, isize);
        vertices->assign(&dest_vertices[0], 0, vsize);

        set_vertices(vertices);
        set_num_vertices(num_vertices);
      }
    }

    /// Add a polygon to the mesh, appending vertices until the buffer size is exceeded.
    /// returns false if no space is available.
    /// If we are in GL_TRIANGLES mode, fill the triangles.
    template <class uvgen> bool add_polygon(const polygon &poly) {
      bool is_triangles = get_mode() == GL_TRIANGLES;
      unsigned npv = poly.get_num_vertices();

      if (is_triangles && npv < 3) {
        return false;
      }

      if ((num_vertices + npv) * sizeof(vertex) > vertices->get_size()) {
        return false;
      }

      unsigned ni = is_triangles ? (npv - 2) * 3 : npv * 2;
      if ((num_indices + ni) * sizeof(uint32_t) > indices->get_size()) {
        return false;
      }

      gl_resource::wolock vlock(get_vertices());
      gl_resource::wolock ilock(get_indices());

      vertex *vtx = (vertex*)vlock.u8() + num_vertices;
      unsigned onv = num_vertices;
      for (unsigned i = 0; i != npv; ++i) {
        vec3 pos = poly.get_vertex(i);
        vtx->pos = uvgen::pos(pos);
        vtx->normal = uvgen::normal(pos);
        vtx->uv = uvgen::uv(pos);
        vtx++;
      }
      num_vertices += npv;

      uint32_t *idx = ilock.u32() + num_indices;
      if (is_triangles) {
        // assume polygon is convex and fill it
        for (unsigned i = 0; i + 2 < npv; ++i) {
          *idx++ = onv;
          *idx++ = onv + i + 1;
          *idx++ = onv + i + 2;
        }
        num_indices += (npv - 2) * 3;
      } else {
        // add a ring of lines.
        for (unsigned i = 0; i + 1 < npv; ++i) {
          *idx++ = onv + i;
          *idx++ = onv + i + 1;
        }
        *idx++ = onv + npv - 1;
        *idx++ = onv;
        num_indices += npv * 2;
      }

      return true;
    }

    /// Add a pair of polygons to the mesh, appending vertices until the buffer size is exceeded.
    /// returns false if no space is available.
    /// The polygons must have the same number of vertices.
    /// If we are in GL_TRIANGLES mode, fill the gap between the polygons.
    /// If we are in GL_LINES mode, add both polygons as lines.
    template <class uvgen> bool extrude(const polygon &poly1, const polygon &poly2) {
      bool is_triangles = get_mode() == GL_TRIANGLES;
      if (!is_triangles) {
        return add_polygon<uvgen>(poly1) && add_polygon<uvgen>(poly2);
      }

      unsigned npv = poly1.get_num_vertices();
      if (npv != poly2.get_num_vertices()) {
        return false;
      }

      if ((num_vertices + npv*2) * sizeof(vertex) > vertices->get_size()) {
        return false;
      }

      unsigned ni = npv * 6;
      if ((num_indices + ni) * sizeof(uint32_t) > indices->get_size()) {
        return false;
      }

      gl_resource::wolock vlock(get_vertices());
      gl_resource::wolock ilock(get_indices());

      vertex *vtx = (vertex*)vlock.u8() + num_vertices;
      unsigned onv = num_vertices;
      for (unsigned i = 0; i != npv; ++i) {
        vec3 pos = poly1.get_vertex(i);
        vtx->pos = uvgen::pos(pos);
        vtx->normal = uvgen::normal(pos);
        vtx->uv = uvgen::uv(pos);
        vtx++;
        pos = poly2.get_vertex(i);
        vtx->pos = uvgen::pos(pos);
        vtx->normal = uvgen::normal(pos);
        vtx->uv = uvgen::uv(pos);
        vtx++;
      }
      num_vertices += npv*2;

      uint32_t *idx = ilock.u32() + num_indices;
      for (unsigned i = 0; i < npv; ++i) {
        *idx++ = onv + i;
        *idx++ = onv + i + 1;
        *idx++ = onv + i + 2;
        *idx++ = onv + i + 1;
        *idx++ = onv + i + 3;
        *idx++ = onv + i + 2;
      }
      num_indices += npv * 6;

      return true;
    }

    template <class vertex_t> struct sink {
      mesh *mesh_;
      dynarray<vertex_t> vertices;
      dynarray<uint32_t> indices;
      mat4t transform;

      sink(mesh *mesh_, mat4t_in transform) :
        mesh_(mesh_), transform(transform)
      {
      }

      ~sink() {
        mesh_->allocate(sizeof(vertex_t) * vertices.size(), sizeof(uint32_t) * indices.size());
        mesh_->get_vertices()->assign(vertices.data(), 0, sizeof(vertex_t) * vertices.size());
        mesh_->get_indices()->assign(indices.data(), 0, sizeof(uint32_t) * indices.size());
        mesh_->set_num_vertices(vertices.size());
        mesh_->set_num_indices(indices.size());
      }

      void reserve(uint32_t num_vertices, uint32_t num_indices) {
        vertices.reserve(num_vertices);
        indices.reserve(num_indices);
      }

      size_t add_vertex(vec3_in pos, vec3_in normal, vec3_in uvw) {
        vec3 tpos = pos * transform;
        vec3 tnormal = normal.x() * transform.x().xyz() + normal.y() * transform.y().xyz() + normal.z() * transform.z().xyz();
        size_t index = vertices.size();
        vertices.push_back(vertex_t(tpos, tnormal, uvw));
        return index;
      }

      void add_triangle(uint32_t a, uint32_t b, uint32_t c) {
        indices.push_back(a);
        indices.push_back(b);
        indices.push_back(c);
      }

      const vertex_t &get_vertex(size_t index) const {
        return vertices[index];
      }
    };

    template <class shape_t, class vertex_t> void set_shape(shape_t &shape, mat4t_in transform, int steps) {
      sink<vertex_t> sink_(this, transform);
      shape.get_geometry(sink_, steps);
    }
  };
}}
