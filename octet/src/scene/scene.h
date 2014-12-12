////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Scene Node heirachy
//

#ifndef OCTET_SCENE_INCLUDED
#define OCTET_SCENE_INCLUDED

#include "../scene/scene_node.h"
#include "../scene/skin.h"
#include "../scene/skeleton.h"
#include "../scene/animation.h"
#include "../scene/mesh.h"
#include "../scene/image.h"
#include "../scene/sampler.h"
#include "../scene/param.h"
#include "../scene/material.h"
#include "../scene/light.h"
#include "../scene/camera_instance.h"
#include "../scene/light_instance.h"
#include "../scene/mesh_instance.h"
#include "../scene/animation_instance.h"
#include "../scene/visual_scene.h"
#include "../scene/displacement_map.h"
#include "../scene/indexer.h"
#include "../scene/smooth.h"
#include "../scene/mesh_text.h"
#include "../scene/mesh_box.h"
#include "../scene/mesh_cylinder.h"
#include "../scene/mesh_sphere.h"
#include "../scene/mesh_particle_system.h"
#include "../scene/mesh_terrain.h"
#ifdef OCTET_VOXEL_TEST
  #include "../scene/mesh_voxel_subcube.h"
  #include "../scene/mesh_voxels.h"
#endif
#include "../scene/mesh_points.h"
#include "../scene/wireframe.h"
#include "../scene/mesh_voxel_grid.h"

namespace octet {
  using namespace scene;
}

#endif
