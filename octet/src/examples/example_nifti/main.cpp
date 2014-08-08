////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Text overlay
//

#include "../../octet.h"

#include "example_nifti.h"

/// Create a box with a 3d animated texture
int main(int argc, char **argv) {
  // set up the platform.
  octet::app::init_all(argc, argv);

  // our application.
  octet::example_nifti app(argc, argv);
  app.init();

  // open windows
  octet::app::run_all_apps();
}


