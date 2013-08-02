////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013 (MIT license)
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

#ifndef OCTET_OPENCL
  #define OCTET_OPENCL 0
#endif

// use <> to include from standard directories
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>

// xml library
#include "../tinyxml/tinystr.cpp"
#include "../tinyxml/tinyxml.cpp"
#include "../tinyxml/tinyxmlerror.cpp"
#include "../tinyxml/tinyxmlparser.cpp"

// this is a dummy class used to customise the placement new and delete
struct dynarray_dummy_t {};

// placement new operator, allows construction in-place at "place"
void *operator new(size_t size, void *place, dynarray_dummy_t x) { return place; }

// dummy placement delete operator, allows destruction at "place"
void operator delete(void *ptr, void *place, dynarray_dummy_t x) {}

#include "../containers/allocator.h"
#include "../containers/dictionary.h"
#include "../containers/hash_map.h"
#include "../containers/double_list.h"
#include "../containers/dynarray.h"
#include "../containers/string.h"
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

// resources
#include "../resources/app_utils.h"
#include "../resources/visitor.h"
#include "../resources/xml_writer.h"
#include "../resources/http_writer.h"
#include "../resources/resource.h"
#include "../resources/resources.h"
#include "../resources/gl_resource.h"
#include "../resources/mesh_builder.h"
#include "../resources/animation_target.h"

// shaders
#include "../shaders/shader.h"
#include "../shaders/color_shader.h"
#include "../shaders/texture_shader.h"
#include "../shaders/phong_shader.h"
#include "../shaders/bump_shader.h"

// scene
#include "../scene/scene_node.h"
#include "../scene/skin.h"
#include "../scene/skeleton.h"
#include "../scene/animation.h"
#include "../scene/mesh.h"
#include "../scene/material.h"
#include "../scene/camera_instance.h"
#include "../scene/light_instance.h"
#include "../scene/mesh_instance.h"
#include "../scene/animation_instance.h"
#include "../scene/scene.h"

// high level helpers
#include "../helpers/mouse_ball.h"
#include "../helpers/http_server.h"
#include "../helpers/text_overlay.h"

// asset loaders
#include "../loaders/gif_decoder.h"
#include "../loaders/jpeg_decoder.h"
#include "../loaders/tga_decoder.h"
#include "../loaders/collada_builder.h"

// forward references
#include "../resources/resources.inl"
#include "../resources/mesh_builder.inl"

#include "../physics/physics.h"
