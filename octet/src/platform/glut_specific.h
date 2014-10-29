////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Microsoft Windows specific information

#ifdef WIN32
  // avoid irritating security warnings
  #define _CRT_SECURE_NO_WARNINGS 1
  #pragma warning(disable : 4996)
  #pragma warning(disable : 4345)

  // This is the "GL utilities" framework for drawing with OpenGL
  #define FREEGLUT_STATIC
  #define FREEGLUT_LIB_PRAGMAS 0
  #include "GL/glut.h"

  #pragma comment(lib, "OpenGL32.Lib")
  #pragma comment(lib, "../src/platform/freeglut_static.lib")

  #include <malloc.h>

  // get enough of the OpenGL API to cover ES2 and ES3, the mobile variants
  #define GL_APIENTRY __stdcall
  #include "gl_defs.h"
  #if OCTET_OPENCL
    #include <CL/cl.h>
  #endif
#elif defined(__APPLE__)
  //#define OCTET_GLES2 1
  #include <OpenAL/alc.h>
  #include <OpenAL/al.h>
  #include <OpenCL/cl.h>
  #include <OpenGL/gl.h>
  #include <OpenGL/glext.h>
  #include <GLUT/glut.h>
  #if OCTET_OPENCL
    #include <OpenCL/opencl.h>
  #endif
#elif defined(OCTET_LINUX)
  #define GL_GLEXT_PROTOTYPES
  #include <GL/glut.h>
  #include <GL/glext.h>
  #include <AL/alc.h>
  #include <AL/al.h>
#endif

// include cross platform app helpers, such as texture loaders
#include "app_common.h"

namespace octet {
  // this is the class that all apps are derived from.
  class app : public app_common {
    int window_handle;

    typedef hash_map<int, app*> map_t;

    static map_t &map() {
      static map_t instance;
      return instance;
    }

  public:
    // constructor
    app(int argc, char **argv) {
    }

    // initialiser (it is nice to keep the two separate for aggregate memory allocation)
    void init() {
      glutInitDisplayMode(GLUT_RGBA|GLUT_DEPTH|GLUT_DOUBLE);
      glutInitWindowSize(500, 500);
      window_handle = glutCreateWindow("glut window");
      map()[window_handle] = this;
      #ifdef WIN32
        init_wgl();
      #endif
      app_init();
    }

    void render() {
      begin_frame();

      //printf("render %d\n", glutGetWindow());
      int vx, vy;
      get_viewport_size(vx, vy);
      draw_world(0, 0, vx, vy);
      inc_frame_number();
      end_frame();
      glutSwapBuffers();
    }

    void disable_cursor() const {
    }

    void enable_cursor() const {
    }

    static unsigned translate_special(unsigned key) {
      switch (key) {
        case GLUT_KEY_END: return key_end;
        case GLUT_KEY_HOME: return key_home;
        case GLUT_KEY_LEFT: return key_left;
        case GLUT_KEY_UP: return key_up;
        case GLUT_KEY_RIGHT: return key_right;
        case GLUT_KEY_DOWN: return key_down;
        case GLUT_KEY_INSERT: return key_insert;
        //case GLUT_KEY_DELETE: return key_delete;

        case GLUT_KEY_F1: return key_f1;
        case GLUT_KEY_F2: return key_f2;
        case GLUT_KEY_F3: return key_f3;
        case GLUT_KEY_F4: return key_f4;
        case GLUT_KEY_F5: return key_f5;
        case GLUT_KEY_F6: return key_f6;
        case GLUT_KEY_F7: return key_f7;
        case GLUT_KEY_F8: return key_f8;
        case GLUT_KEY_F9: return key_f9;
        case GLUT_KEY_F10: return key_f10;
        case GLUT_KEY_F11: return key_f11;
        case GLUT_KEY_F12: return key_f12;
      }
      return key;
    }

    static unsigned translate_key(unsigned key) {
      if (key >= 'a' && key <= 'z') return key - 32;
      return key;
    }

    ~app() {
    }

    // interface from GLUT
    static void reshape(int w, int h) { map()[glutGetWindow()]->set_viewport_size(w, h); }
    static void display() { map()[glutGetWindow()]->render(); }
    static void do_key_down( unsigned char key, int x, int y) { map()[glutGetWindow()]->set_key(app::translate_key(key), 1); }
    static void do_key_up( unsigned char key, int x, int y) { map()[glutGetWindow()]->set_key(app::translate_key(key), 0); }
    static void do_special_down( int key, int x, int y) { map()[glutGetWindow()]->set_key(app::translate_special(key), 1); }
    static void do_special_up( int key, int x, int y) { map()[glutGetWindow()]->set_key(app::translate_special(key), 0); }
    static void do_mouse_button(int button, int state, int x, int y) {
      //printf("%d %d %d %d\n", button, state, x, y);
      map()[glutGetWindow()]->set_key(key_lmb + (button - GLUT_LEFT_BUTTON), state == GLUT_DOWN);
      map()[glutGetWindow()]->set_mouse_pos(x, y);
    }
    static void do_mouse(int x, int y) {
      //printf("%d %d\n", x, y);
      map()[glutGetWindow()]->set_mouse_pos(x, y);
    }

    static void timer(int value) {
      glutTimerFunc(16, timer, 1);
      map_t &m = map();
      for (int i = 0; i != m.size(); ++i) {
        if (m.get_key(i)) {
          glutSetWindow(m.get_key(i));
          glutPostRedisplay();
        }
      }
    }
  
    static void init_all(int &argc, char **argv) {
      glutInit(&argc, argv);
      ALCdevice *dev = alcOpenDevice(NULL);
      if (dev == NULL) {
        printf("OpenAL not found, disabling sound");
      } else {
        ALCcontext *ctx = alcCreateContext(dev, NULL);
        if (ctx == NULL) {
          printf("OpenAL not found, disabling sound");
        } else {
          alcMakeContextCurrent(ctx);
        }
      }
    }

    static void run_all_apps() {
      map_t &m = map();
      for (int i = 0; i != m.size(); ++i) {
        if (m.get_key(i)) {
          glutSetWindow(m.get_key(i));
          glutDisplayFunc(display);
          glutReshapeFunc(reshape);
          glutKeyboardFunc(do_key_down);
          glutKeyboardUpFunc(do_key_up);
          glutSpecialFunc(do_special_down);
          glutSpecialUpFunc(do_special_up);
          glutMouseFunc(do_mouse_button);
          glutMotionFunc(do_mouse);
          glutPassiveMotionFunc(do_mouse);
        }
      }
      glutTimerFunc(16, timer, 1);
      glutMainLoop();
    }

    static void error(const char *msg) {
      printf("%s - exiting\n", msg);
      exit(1);
    }
  };

  // dummy video capture class
  /*class video_capture {
  public:
    video_capture() {
    }

    int open() {
      return -1;
    }

    int read(void *buffer, unsigned max_size) {
      return 0;
    }

    int close() {
      return 0;
    }

    unsigned width() { return 0; }
    unsigned height() { return 0; }
    unsigned bits_per_pixel() { return 0; }
    unsigned image_size() { return 0; }
  };*/

  // on ARM we can do this faster with the "rev" instruction
  inline static unsigned rev16(unsigned value) {
    value = ( ( value >> 1 ) & 0x5555 ) | ( ( value & 0x5555 ) << 1 );
    value = ( ( value >> 2 ) & 0x3333 ) | ( ( value & 0x3333 ) << 2 );
    value = ( ( value >> 4 ) & 0x0f0f ) | ( ( value & 0x0f0f ) << 4 );
    value = ( ( value >> 8 ) & 0x00ff ) | ( ( value & 0x00ff ) << 8 );
    return value;
  }
}
