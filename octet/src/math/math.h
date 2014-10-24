////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014, 2014 (MIT license)
//
// Math library for games and geometry based on glsl
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the 
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
// AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#ifndef OCTET_MATH_INCLUDED
#define OCTET_MATH_INCLUDED


namespace octet {
  namespace math {
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
    OCTET_HUNGARIANS(zcylinder)
  }

  using namespace math;
}

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
#include "polygon.h"
#include "zcylinder.h"
#include "voxel_grid.h"

#endif
