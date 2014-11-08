////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// raw 3D mesh container
//

namespace octet { namespace scene {
  /// Instance of a mesh in a game world; node, mesh, material and skin.
  class mesh_instance : public resource {
  public:
    enum { flag_selected = 1 << 0, flag_enabled = 1 << 1, flag_lod = 1 << 2 };

  private:
    // which scene_node (model to world matrix) to use in the scene
    ref<scene_node> node;

    // which mesh to render
    ref<mesh> msh;

    // what material to use
    ref<material> mat;

    // for characters, which skeleton to use
    ref<skeleton> skel;

    // assorted mesh instance booleans (see flag_*)
    unsigned flags;

    // if the object is closer than this from the camera, do not draw.
    float min_draw_distance;

    // if the object is further than this from the camera, do not draw.
    float max_draw_distance;

  public:
    RESOURCE_META(mesh_instance)

    /// Create a new mesh instance. If you add this instance to a scene, it will render it.
    mesh_instance(scene_node *node=0, mesh *msh=0, material *mat=0, skeleton *skel=0) {
      this->node = node;
      this->msh = msh;
      this->mat = mat;
      this->skel = skel;
      flags = flag_enabled;
      min_draw_distance = -8.507059e37f;
      max_draw_distance = 8.507059e37f;
    }

    /// metadata visitor. Used for serialisation and script interface.
    void visit(visitor &v) {
      v.visit(node, atom_node);
      v.visit(msh, atom_msh);
      v.visit(mat, atom_mat);
      v.visit(skel, atom_skel);
      v.visit(flags, atom_flags);
    }

    //////////////////////////////
    //
    // animation_target interface
    //

    /// the virtual add_ref on animation_target gets passed to here and we pass iton (delegate it) to the resource
    void add_ref() {
      resource::add_ref();
    }

    /// the virtual release on animation_target gets passed to here and we pass iton (delegate it) to the resource
    void release() {
      resource::release();
    }

    /// animation input: for now, we only support skeleton animation
    void set_value(atom_t sid, atom_t sub_target, atom_t component, float *value) {
      if (skel) {
        // hack for 
        static float euler[3];
        static float translate[3];
        static float scale[3];

        // todo: cache the index
        int index = skel->get_bone_index(sid);
        if (index != -1) {
          switch (sub_target) {
            case atom_transform: {
              mat4t m;
              m.init_transpose(value);
              skel->set_bone(index, m);
            } break;
            case atom_rotateX: euler[0] = *value; break;
            case atom_rotateY: euler[1] = *value; break;
            case atom_rotateZ: euler[2] = *value; break;
            case atom_translate: translate[0] = value[0]; translate[1] = value[1]; translate[2] = value[2]; break;
            case atom_scale: scale[0] = value[0]; scale[1] = value[1]; scale[2] = value[2]; break;
            default: break;
          }
        }
      }
    }

    void update(float delta_time) {
    }

    //////////////////////////////
    //
    // accessor methods
    //

    /// Get the transformation for this instance.
    scene_node *get_node() const { return node; }

    /// Get the mesh for this instance.
    mesh *get_mesh() const { return msh; }

    /// Get the material for this instance.
    material *get_material() const { return mat; }

    /// Get the skeleton for this instance.
    skeleton *get_skeleton() const { return skel; }

    /// Get the flags for this instance.
    unsigned get_flags() const { return flags; }

    /// Get the LOD min distance
    float get_min_draw_distance() const { return min_draw_distance; }

    /// Get the LOD max distance
    float get_max_draw_distance() const { return max_draw_distance; }

    /// Set the transformation for this instance.
    void set_node(scene_node *value) { node = value; }

    /// Set the mesh for this instance.
    void set_mesh(mesh *value) { msh = value; }

    /// Set the mesh for this instance.
    void set_material(material *value) { mat = value; }

    /// Set the skeleton for this instance.
    void set_skeleton(skeleton *value) { skel = value; }

    /// Set the flags for this instance.
    void set_flags(unsigned value) { flags = value; }

    /// Set the flags for this instance.
    void set_min_draw_distance(float value) { min_draw_distance = value; }

    /// Set the flags for this instance.
    void set_max_draw_distance(float value) { max_draw_distance = value; }
  };
}}

