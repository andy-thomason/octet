////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// 3D mesh container
//

// mesh builder class for standard meshes.
// get a mesh mesh from the builder either as VBOs or allocated memory.
inline void octet::resources::mesh_builder::get_mesh(scene::mesh &s) {
  unsigned isize = indices.size() * sizeof(indices[0]);
  unsigned vsize = vertices.size() * sizeof(vertices[0]);
  s.init();
  s.allocate(vsize, isize);
  s.assign(vsize, isize, (unsigned char*)&vertices[0], (unsigned char*)&indices[0]);
  s.set_params(sizeof(vertex), indices.size(), vertices.size(), GL_TRIANGLES, GL_UNSIGNED_SHORT);

  s.add_attribute(attribute_pos, 3, GL_FLOAT, 0);
  s.add_attribute(attribute_normal, 3, GL_FLOAT, 12);
  s.add_attribute(attribute_uv, 2, GL_FLOAT, 24);
}
