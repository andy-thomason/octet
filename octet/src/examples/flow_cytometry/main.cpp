////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Test for Voxel obb test
//

#include "../../octet.h"

#include "flow_cytometry.h"

//
// a C++ program starts here with the command line arguments in argv[]
// argc is the number of arguments with argv[0] set to the command name.
//
int main(int argc, char **argv) {
  octet::app_utils::prefix("../../../");
  octet::app::init_all(argc, argv);
  octet::flow_cytometry app(argc, argv);
  app.init();
  octet::app::run_all_apps();
}

