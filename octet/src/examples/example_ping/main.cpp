////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Ping, a game of two pongs.
//

#include "../../octet.h"

#include "ping_app.h"

/// Create a box with octet
int main(int argc, char **argv) {
  // set up the platform.
  octet::app::init_all(argc, argv);

  // our application.
  octet::ping_app app(argc, argv);
  app.init();

  // open windows
  octet::app::run_all_apps();
}
