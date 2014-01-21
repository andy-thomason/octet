////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Scene with one box in octet
//

#include "../../octet.h"

#include "example_box.h"

/// Create a box with octet
int main(int argc, char **argv) {
  octet::app_utils::prefix("../../");
  octet::app::init_all(argc, argv);
  octet::example_box app(argc, argv);
  app.init();
  octet::app::run_all_apps();
}

