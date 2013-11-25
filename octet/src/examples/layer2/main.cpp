////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Animation example.
//
// Demonstrates a basic scene graph with import from Collada.
//

#include "../../platform/platform.h"

#if defined(OCTET_OBB)
  #include "obb_test.h"
#else
  #include "engine.h"
#endif

//
// a C++ program starts here with the command line arguments in argv[]
// argc is the number of arguments with argv[0] set to the command name.
//
int main(int argc, char **argv) {
  //octet::unit_test_ray();

  octet::app_utils::prefix("../../");
  octet::app::init_all(argc, argv);
  #if defined(OCTET_OBB)
    octet::obb_test app(argc, argv);
  #else
    octet::engine app(argc, argv);
  #endif
  app.init();
  octet::app::run_all_apps();
}

