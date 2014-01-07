////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Test for Voxel obb test
//

#include "../../platform/platform.h"

#include "voxel_obb_test.h"

//
// a C++ program starts here with the command line arguments in argv[]
// argc is the number of arguments with argv[0] set to the command name.
//
int main(int argc, char **argv) {
  octet::app_utils::prefix("../../");
  octet::app::init_all(argc, argv);
  octet::voxel_obb_test app(argc, argv);
  app.init();
  octet::app::run_all_apps();
}

