////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014 (MIT license)
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

#ifndef OCTET_INCLUDED
#define OCTET_INCLUDED

  ///
  /// octet is the top-level namespace.
  /// All classes that are part of the octet framework are part of this namespace.
  /// 
  namespace octet {
    /// The containers namespace holds classes that own data.
    ///
    /// The data will be freed when the objects go out of scope.
    ///
    /// Examples
    ///
    ///     dynarray<float> my_float_array; // a variable length array of floating point numbers
    ///     ref<visual_scene> my_scene;     // a smart pointer to a visual scene object.
    ///     dictionary<int> my_dict;        // text-to-object map, can be accessed like my_dict["twenty"]
    namespace containers {}


    /// The resources namespace holds classes that manage game data.
    ///
    /// All classes that are derived from the class resouce will work with the ref<> class.
    ///
    /// Examples
    ///
    ///     resource_dict my_resource;
    ///     visual_scene *scene = my_resource.get_visual_scene("loading_scene");
    namespace resources {}
    
    /// The scene namespace holds classes that represent parts of a game scene.
    ///
    /// All classes are derived from resource so that the ref<> class can hold a pointer to them.
    ///
    /// Example
    ///
    ///     visual_scene *scene = new visual_scene();
    ///     
    namespace scene {}
    
    /// The math namespace contains classes that deal with numbers and vectors.
    ///
    /// The classes are designed to be similar to vectors and matrices in GLSL and OpenCL
    namespace math {}
    
    /// The helpers namespace contains classe that provide user interface services.
    namespace helpers {}
    
    /// The loaders namespace contains classes that decode and encode a variety of formats.
    namespace loaders {}
    
    /// The shaders namespace contains a number of stock shaders.
    namespace shaders {}

    /// Functions and classes used to interact with physics systems
    namespace physics {}

    /// System utilities and hardware
    namespace platform {}

    using namespace containers;
    using namespace resources;
    using namespace scene;
    using namespace math;
    using namespace helpers;
    using namespace loaders;
    using namespace shaders;
    using namespace physics;
    using namespace platform;
  }

  // defines and configuration
  #include "platform/configure.h"

  // data storage in containers
  #include "containers/containers.h"

  // target specific support: Windows, Mac, Linux, PS Vita
  #include "platform/machine_specific.h"
  #include "platform/args_parser.h"

  // math library
  #include "math/math.h"

  // CG, GLSL, C++ compiler
  #include "compiler/compiler.h"

  // loaders (low dependency, so you can use them in other projects)
  #include "loaders/loaders.h"

  // xml library
  #include "tinyxml/tinystr.cpp"
  #include "tinyxml/tinyxml.cpp"
  #include "tinyxml/tinyxmlerror.cpp"
  #include "tinyxml/tinyxmlparser.cpp"

  // resource management
  #include "resources/resources.h"

  // shaders
  #include "shaders/shaders.h"

  // physics
  #ifdef OCTET_BULLET
    #pragma warning(disable : 4267)
    #include "../open_source/bullet/bullet.h"
  #endif

  // scene
  #include "scene/scene.h"

  #ifdef OCTET_OPENCL
    #include "platform/CL/cl.h"
    #include "platform/CL/cl_gl.h"
    #include "platform/opencl.h"
  #endif

  // high level helpers
  #include "helpers/mouse_ball.h"
  #include "helpers/mouse_look.h"
  #include "helpers/http_server.h"
  #include "helpers/text_overlay.h"
  #include "helpers/object_picker.h"
  #include "helpers/helper_fps_controller.h"

  // asset loaders
  #include "loaders/collada_builder.h"

  // forward references
  #include "resources/resources.inl"
  #include "resources/mesh_builder.inl"
#endif
