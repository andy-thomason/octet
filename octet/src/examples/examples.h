////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// include all examples

#include "../platform/platform.h"

#include "triangle/triangle_app.h"
#include "texture/texture_app.h"
#include "gif/gif_app.h"
#include "ping/ping_app.h"
#include "invaderers/invaderers_app.h"
#include "cube/cube_app.h"
#include "duck/duck_app.h"
#include "bump/bump_app.h"
#include "physics/physics_app.h"

namespace octet {
  static app *app_factory(const char *name, int argc, char **argv) {
    if (!strcmp(name, "triangle")) return new triangle_app(argc, argv);    //  triangle sample: start here to learn how to draw a triangle
    else if (!strcmp(name, "texture")) return new texture_app(argc, argv); //  texture sample: continue here to learn how to draw a textured triangle
    else if (!strcmp(name, "gif")) return new gif_app(argc, argv);         //  gif sample: continue here to learn how to load and draw a GIF texture
    else if (!strcmp(name, "ping")) return new ping_app(argc, argv);       //  ping sample: continue here to learn how to make a simple game
    else if (!strcmp(name, "invaderers")) return new invaderers_app(argc, argv);//  invaderers sample: continue here to learn how to make sounds
    else if (!strcmp(name, "cube")) return new cube_app(argc, argv);       //  cube sample: continue here to learn how to draw stock 3d geometry
    else if (!strcmp(name, "duck")) return new duck_app(argc, argv);       //  duck sample: continue here to learn how to load collada geometry
    else if (!strcmp(name, "bump")) return new bump_app(argc, argv);       //  bump map sample: continue here to learn how to draw bump mapped textures
    else if (!strcmp(name, "physics")) return new physics_app(argc, argv); //  physics sample: continue here to learn how to annimate using physics
    else return 0;
  }

  inline void run_examples(int argc, char **argv) {
    app::init_all(argc, argv);

    if (argc == 1) {
      printf("running triangle... Why not try texture, gif, ping, cube, duck, bump or physics?\n");
      app *myapp = app_factory("triangle", argc, argv);
      // if you can't edit the debug arguments,
      // change the string above to another of the examples.
      myapp->init();
    } else {
      for (int i = 1; i != argc; ++i) {
        if (argv[i][0] != '-') {
          app *myapp = app_factory(argv[i], argc, argv);
          if (myapp) {
            myapp->init();
          }
        }
      }
    }

    app::run_all_apps();
  }
}
