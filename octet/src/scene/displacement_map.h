////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Displacement map modifier.
// use with smooth modifier for terrain.
//

namespace octet {
  class displacement_map : public mesh {
    // source mesh. Provides underlying geometry.
    ref<mesh> src;

  public:
    RESOURCE_META(displacement_map)

    displacement_map(mesh *src=0) {
      this->src = src;
      update();
    }

    void update() {
      if (!src) return;

      *(mesh*)this = *(mesh*)src;

    }

    void visit(visitor &v) {
      mesh::visit(v);
      v.visit(src, atom_src);
    }
  };
}
