////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2013 (MIT license)
//
// Framework for OpenGLES2 rendering on multiple platforms.
//
// Platform specific includes
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

#define OCTET_OPENCL 1

// use <> to include from standard directories
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>

// some standard c++ definitions
#include <map>

#include "../containers/allocator.h"
#include "../containers/chars.h"
#include "../containers/dictionary.h"
#include "../containers/hash_map.h"
#include "../containers/double_list.h"
#include "../containers/dynarray.h"
#include "../containers/ptr.h"
#include "../containers/ref.h"

#ifdef WIN32
  #include "windows_specific.h"
  //#include "glut_specific.h"
#elif defined(__APPLE__)
  #include "glut_specific.h"
#endif

// use "" to include from our own project
#include "../math/vector.h"
#include "../math/matrix.h"
#include "../math/random.h"

#include "../resources/tinyxml/tinystr.cpp"
#include "../resources/tinyxml/tinyxml.cpp"
#include "../resources/tinyxml/tinyxmlerror.cpp"
#include "../resources/tinyxml/tinyxmlparser.cpp"

// standard attribute names
enum attribute {
  attribute_position = 0,
  attribute_pos = 0,
  attribute_blendweight = 1,
  attribute_normal = 2,
  attribute_diffuse = 3,
  attribute_color = 3,
  attribute_specular = 4,
  attribute_tessfactor = 5,
  attribute_fogcoord = 5,
  attribute_psize = 6,
  attribute_blendindices = 7,
  attribute_texcoord = 8,
  attribute_uv = 8,
  attribute_tangent = 14,
  attribute_bitangent = 15,
  attribute_binormal = 15,
};

#include "../resources/app_utils.h"
#include "../resources/resource.h"
#include "../resources/resources.h"
#include "../resources/animation_target.h"
#include "../resources/animation.h"
#include "../shaders/shader.h"
#include "../shaders/color_shader.h"
#include "../shaders/texture_shader.h"
#include "../shaders/phong_shader.h"
#include "../shaders/bump_shader.h"
#include "../resources/scene_node.h"
#include "../resources/skin.h"
#include "../resources/skeleton.h"
#include "../resources/mesh_state.h"
#include "../resources/material.h"
#include "../resources/bump_material.h"
#include "../resources/lighting.h"
#include "../resources/camera_instance.h"
#include "../resources/light_instance.h"
#include "../resources/mesh_instance.h"
#include "../resources/animation_instance.h"
#include "../resources/scene.h"

#include "../resources/gif_decoder.h"
#include "../resources/tga_decoder.h"
#include "../resources/mesh_builder.h"
#include "../resources/collada_builder.h"
#include "../resources/mesh.h"

// forward references
#include "../resources/material.inl"
#include "../resources/resources.inl"

#include "../physics/physics.h"

#include "../raytracer/raytracer.h"
