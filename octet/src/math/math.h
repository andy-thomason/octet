////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013 (MIT license)
//
// Math library for games and geometry based on glsl

namespace octet {
  OCTET_HUNGARIANS_NC(int)
  OCTET_HUNGARIANS_NC(unsigned)
  OCTET_HUNGARIANS_NC(float)
  OCTET_HUNGARIANS_NC(int8_t)
  OCTET_HUNGARIANS_NC(uint8_t)
  OCTET_HUNGARIANS_NC(int16_t)
  OCTET_HUNGARIANS_NC(uint16_t)
  OCTET_HUNGARIANS_NC(int32_t)
  OCTET_HUNGARIANS_NC(uint32_t)

  OCTET_HUNGARIANS(rational)
  OCTET_HUNGARIANS(vec2)
  OCTET_HUNGARIANS(vec3)
  OCTET_HUNGARIANS(vec3p)
  OCTET_HUNGARIANS(vec4)
  OCTET_HUNGARIANS(ivec3)
  OCTET_HUNGARIANS(ivec4)
  OCTET_HUNGARIANS(quat)
  OCTET_HUNGARIANS(mat4t)
  OCTET_HUNGARIANS(bvec2)
  OCTET_HUNGARIANS(bvec3)
  OCTET_HUNGARIANS(bvec4)
  OCTET_HUNGARIANS(aabb)
  OCTET_HUNGARIANS(obb)
  OCTET_HUNGARIANS(sphere)
  OCTET_HUNGARIANS(plane)
  OCTET_HUNGARIANS(half_space)
  OCTET_HUNGARIANS(ray)
  OCTET_HUNGARIANS(random)
};

// numbers
#include "scalar.h"
#include "random.h"
#include "rational.h"
#include "vec2.h"
#include "vec3.h"
#include "vec4.h"
#include "ivec3.h"
#include "ivec4.h"
#include "quat.h"
#include "mat4t.h"
#include "bvec2.h"
#include "bvec3.h"
#include "bvec4.h"

// geometry
#include "aabb.h"
#include "obb.h"
#include "sphere.h"
#include "plane.h"
#include "half_space.h"
#include "ray.h"
#include "csg.h"
