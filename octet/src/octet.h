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

// defines and configuration
#include "platform/configure.h"

// data storage in containers
#include "containers/containers.h"

// target specific support: Windows, Mac, Linux, PS Vita
#include "platform/machine_specific.h"

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
#include "physics/physics.h"

// scene (layer2)
#include "scene/scene.h"

// high level helpers
#include "helpers/mouse_ball.h"
#include "helpers/http_server.h"
#include "helpers/text_overlay.h"
#include "helpers/object_picker.h"

// asset loaders
#include "loaders/collada_builder.h"

// forward references
#include "resources/resources.inl"
#include "resources/mesh_builder.inl"

