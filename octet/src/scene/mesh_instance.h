////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// raw 3D mesh container
//

namespace octet {
  class mesh_instance : public resource {
    // which scene_node (model to world matrix) to use in the scene
    ref<scene_node> node;

    // which mesh to render
    ref<mesh> msh;

    // what material to use
    ref<material> mat;

    // for characters, which skeleton to use
    ref<skeleton> skel;
    

  public:
    RESOURCE_META(mesh_instance)

    mesh_instance(scene_node *node=0, mesh *msh=0, material *mat=0, skeleton *skel=0) {
      this->node = node;
      this->msh = msh;
      this->mat = mat;
      this->skel = skel;
    }

    // metadata visitor. Used for serialisation and script interface.
    void visit(visitor &v) {
      v.visit(node, atom_node);
      v.visit(msh, atom_msh);
      v.visit(mat, atom_mat);
      v.visit(skel, atom_skel);
    }

    //////////////////////////////
    //
    // animation_target interface
    //

    // the virtual add_ref on animation_target gets passed to here and we pass iton (delegate it) to the resource
    void add_ref() {
      resource::add_ref();
    }

    // the virtual release on animation_target gets passed to here and we pass iton (delegate it) to the resource
    void release() {
      resource::release();
    }

    // animation input: for now, we only support skeleton animation
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

    //////////////////////////////
    //
    // accessor methods
    //

    void update(float delta_time) {
    }

    //////////////////////////////
    //
    // accessor methods
    //

    scene_node *get_node() const { return node; }
    mesh *get_mesh() const { return msh; }
    material *get_material() const { return mat; }
    skeleton *get_skeleton() const { return skel; }

    void set_node(scene_node *value) { node = value; }
    void set_mesh(mesh *value) { msh = value; }
    void set_material(material *value) { mat = value; }
    void set_skeleton(skeleton *value) { skel = value; }
  };
}

