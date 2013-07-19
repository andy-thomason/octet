////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// OSX specific information

//#include <OpenGL/glx.h>
#include <OpenGL/gl.h>

typedef int HWND;

#include "window_map.h"

#include "app_common.h"

class app : public app_common {
public:
  enum {
    key_left = 0,
    key_right = 0,
    key_up = 0,
    key_down = 0,
    key_space = 0,
  };

  app(int argc, char **argv) {
  }

  void init() {
    printf("hello from init\n");
  }
};

// we have a circular dependency: window_map -> app -> window_map
// and so must separate the declaration (in the class) and definition (here, after app) 
inline void window_map::run() {
}
