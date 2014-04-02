////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// a container for named resources
//

#ifndef OCTET_RESOURCES_INCLUDED
#define OCTET_RESOURCES_INCLUDED
  namespace octet {
    namespace scene { class visual_scene; }
    #define OCTET_CLASS(N, X) namespace N { class X; }
    #include "classes.h"
    #undef OCTET_CLASS
  }

  // resources
  #include "../resources/file_map.h"
  #include "../resources/zip_file.h"
  #include "../resources/app_utils.h"
  #include "../resources/visitor.h"
  #include "../resources/binary_writer.h"
  #include "../resources/binary_reader.h"
  #include "../resources/xml_writer.h"
  #include "../resources/http_writer.h"
  #include "../resources/resource.h"
  #include "../resources/resource_dict.h"
  #include "../resources/gl_resource.h"
  #include "../resources/bitmap_font.h"
  #include "../resources/mesh_builder.h"

#endif
