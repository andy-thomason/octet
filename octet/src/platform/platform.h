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
// use "" to include from our own project
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

static char *get_sprintf_buffer() {
  static int i;
  static char tmp[4][256];
  return tmp[i++ & 3];
}

#ifdef WIN32
  #include "windows_specific.h"
  //#include "glut_specific.h"
#elif defined(__APPLE__)
  #include <unistd.h>
  #include <sys/socket.h>
  #include <sys/ioctl.h>
  #include <fcntl.h>
  #include <netinet/in.h>
  #define OCTET_HOT __attribute__( ( always_inline ) )
  #define ioctlsocket ioctl
  #define closesocket close
  #include "glut_specific.h"
#endif

#include "../math/scalar.h"
#include "../math/rational.h"
#include "../math/vec2.h"
#include "../math/vec3.h"
#include "../math/vec4.h"
#include "../math/quat.h"
#include "../math/mat4t.h"
#include "../math/ivec4.h"
#include "../math/bvec2.h"
#include "../math/bvec3.h"
#include "../math/bvec4.h"
#include "../math/aabb.h"
#include "../math/ray.h"
#include "../math/random.h"

// loaders (low dependency, so you can use them in other projects)
#include "../loaders/gif_decoder.h"
#include "../loaders/jpeg_decoder.h"
#include "../loaders/jpeg_encoder.h"
#include "../loaders/tga_decoder.h"
#include "../loaders/dds_decoder.h"

// resources
#include "../resources/app_utils.h"
#include "../resources/visitor.h"
#include "../resources/binary_writer.h"
#include "../resources/binary_reader.h"
#include "../resources/xml_writer.h"
#include "../resources/http_writer.h"
#include "../resources/resource.h"
#include "../resources/resources.h"
#include "../resources/gl_resource.h"
#include "../resources/mesh_builder.h"

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
#include "../scene/image.h"
#include "../scene/param.h"
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
#include "../helpers/object_picker.h"

// asset loaders
#include "../loaders/collada_builder.h"

// forward references
#include "../resources/resources.inl"
#include "../resources/mesh_builder.inl"

namespace octet {
  inline resource *resource::new_type(atom_t type) {
    switch (type) {
      case atom_skin: return new skin();
      case atom_skeleton: return new skeleton();
      case atom_mesh: return new mesh();
      case atom_material: return new material();
      case atom_image: return new image();
      case atom_animation: return new animation();
      case atom_camera_instance: return new camera_instance();
      case atom_light_instance: return new light_instance();
      case atom_mesh_instance: return new mesh_instance();
      case atom_animation_instance: return new animation_instance();
      case atom_scene: return new scene();
      case atom_scene_node: return new scene_node();
      case atom_param: return new param();
    }
    return NULL;
  }
}

#include "../physics/physics.h"
