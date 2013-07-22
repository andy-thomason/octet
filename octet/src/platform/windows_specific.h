////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Microsoft Windows specific information

// windows.h contains all the windows-specific definitions such as CreateWindow
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
//#include <mmsystem.h>

// undo unwanted definitions that Microsoft make in windows.h
#undef min // Yes, Microsoft really define min!
#undef max // Yes, Microsoft really define max!

// avoid irritating security warnings
#pragma warning(disable : 4996)
#pragma warning(disable : 4345)

// graphics - opengl
#pragma comment(lib, "OpenGL32.Lib")
#include <gl/GL.h>

// audio - openal
#pragma comment(lib, "../lib/OpenAL32.lib")
#include "AL/alc.h"
#include "AL/AL.h"

// compute - opencl
#if OCTET_OPENCL
  #pragma comment(lib, "../lib/OpenCL.lib")
  #include "CL/cl.h"
  #include "CL/cl_gl.h"
#endif


// some standard c++ definitions
#include <map>
#include <malloc.h>

// windows only supports OpenGL 1.2 natively
// so we need to extend this by getting the addresses of the extra functions
// ... enough of the OpenGL API to cover ES2 and ES3, the mobile variants
// do not use OpenGL 1.x functions *ever* they are obsolete.
#define GL_APIENTRY __stdcall
#include "gl_defs.h"

// include cross platform app helpers, such as texture loaders
#include "app_common.h"

namespace octet {
  // this is the class that all apps are derived from.
  class app : public app_common {
    HGLRC gl_context;
    HWND window_handle;

    void init_gl_context(HDC hdc) {
      static const PIXELFORMATDESCRIPTOR pfd = { 
        sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd  
        1,                     // version number  
        PFD_DRAW_TO_WINDOW |   // support window  
        PFD_SUPPORT_OPENGL |   // support OpenGL  
        PFD_DOUBLEBUFFER,      // double buffered  
        PFD_TYPE_RGBA,         // RGBA type  
        24,                    // 24-bit color depth  
        0, 0, 0, 0, 0, 0,      // color bits ignored  
        0,                     // no alpha buffer  
        0,                     // shift bit ignored  
        0,                     // no accumulation buffer  
        0, 0, 0, 0,            // accum bits ignored  
        32,                    // 32-bit z-buffer      
        0,                     // no stencil buffer  
        0,                     // no auxiliary buffer  
        PFD_MAIN_PLANE,        // main layer  
        0,                     // reserved  
        0, 0, 0                // layer masks ignored  
      };
 
      int pixel_format = ChoosePixelFormat(hdc, &pfd);

      SetPixelFormat(hdc, pixel_format, &pfd);

      gl_context = wglCreateContext(hdc);

      wglMakeCurrent (hdc, gl_context);

      init_wgl();
    }

    typedef std::map<HWND, app*> map_t;
    //typedef dynarray<dynarray<unsigned char>> audio_t;

    static map_t &map() { static map_t instance; return instance; }
    //static audio_t &audio() { static audio_t instance; return instance; }

  public:
    app(int argc, char **argv) {
    }

    void init() {
      HINSTANCE instance = (HINSTANCE)GetModuleHandle(0);
      HBRUSH brush = (HBRUSH) GetStockObject(NULL_BRUSH);
      HICON icon = LoadIcon(0, IDI_ASTERISK);
      HCURSOR cursor = LoadCursor(0, IDC_ARROW);

      static WNDCLASS wndclass = {
        CS_HREDRAW | CS_VREDRAW, DefWindowProc, 0, 0, instance,
        icon, cursor, brush, 0, L"MyClass"
      };
      RegisterClass (&wndclass);

      gl_context = 0;
     
      window_handle = CreateWindow(L"MyClass", L"framework",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 512, 512,
        NULL, NULL, wndclass.hInstance, (LPVOID)this
      );

      map()[window_handle] = this;

      init_gl_context(GetDC(window_handle));

      RECT rect;
      GetClientRect(window_handle, &rect);
      set_viewport_size(rect.right - rect.left, rect.bottom - rect.top);

      app_init();

      ShowWindow (window_handle, SW_SHOW);
      UpdateWindow (window_handle);
    }

    void render() {
      HDC hdc = GetDC(window_handle);
      wglMakeCurrent (hdc, gl_context);

      RECT rect;
      GetClientRect(window_handle, &rect);
      set_viewport_size(rect.right - rect.left, rect.bottom - rect.top);

      draw_world(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);

      SwapBuffers(hdc);

      wglMakeCurrent (hdc, NULL);
    }

    static unsigned translate(unsigned key) {
      switch (key) {
        case VK_SHIFT: return key_shift;
        case VK_CONTROL: return key_ctrl;
        case VK_MENU: return key_alt;

        case VK_END: return key_end;
        case VK_HOME: return key_home;
        case VK_LEFT: return key_left;
        case VK_UP: return key_up;
        case VK_RIGHT: return key_right;
        case VK_DOWN: return key_down;
        case VK_INSERT: return key_insert;
        case VK_DELETE: return key_delete;

        case VK_F1: return key_f1;
        case VK_F2: return key_f2;
        case VK_F3: return key_f3;
        case VK_F4: return key_f4;
        case VK_F5: return key_f5;
        case VK_F6: return key_f6;
        case VK_F7: return key_f7;
        case VK_F8: return key_f8;
        case VK_F9: return key_f9;
        case VK_F10: return key_f10;
        case VK_F11: return key_f11;
        case VK_F12: return key_f12;
      }
      return key;
    }

    ~app() {
      // disable the gl context
      wglMakeCurrent (NULL, NULL); 
 
      // delete the rendering context  
      wglDeleteContext (gl_context);
    }

    static void init_all(int argc, char **argv) {
      sound_disabled() = true;
      ALCdevice *dev = alcOpenDevice(NULL);
      if (dev == NULL) {
        printf("OpenAL not found, disabling sound");
      } else {
        ALCcontext *ctx = alcCreateContext(dev, NULL);
        if (ctx == NULL) {
          printf("OpenAL not found, disabling sound");
        } else {
          alcMakeContextCurrent(ctx);
          sound_disabled() = false;
        }
      }
    }

    static void run_all_apps() {
      MSG msg;     
      for(;;) {
        // todo: get notification when the windows close.
        while (PeekMessage(&msg, 0, 0, 0, TRUE)) {
          //printf("msg=%04x %02x\n", msg.message, msg.wParam);
          app *app = map()[msg.hwnd];
          if (app) {
            if (msg.message == WM_KEYDOWN || msg.message == WM_KEYUP) {
              app->set_key(app::translate(msg.wParam), msg.message == WM_KEYDOWN);
            } else if (msg.message == WM_MOUSEMOVE) {
              app->set_mouse_pos(msg.lParam & 0xffff, msg.lParam >> 16);
            } else if (msg.message == WM_LBUTTONDOWN || msg.message == WM_LBUTTONUP) {
              app->set_key(key_lmb, msg.message == WM_LBUTTONDOWN);
            } else if (msg.message == WM_MBUTTONDOWN || msg.message == WM_MBUTTONUP) {
              app->set_key(key_mmb, msg.message == WM_MBUTTONDOWN);
            } else if (msg.message == WM_RBUTTONDOWN || msg.message == WM_RBUTTONUP) {
              app->set_key(key_mmb, msg.message == WM_RBUTTONDOWN);
            }
          }
          DispatchMessage (&msg);
        }

        // waste some time. (do not do this in real games!)
        Sleep(16);

        // render each window in turn
        for (map_t::iterator i = map().begin(); i != map().end(); ++i) {
          i->second->render();
        }
      }
    }

    static void error(const char *msg) {
      MessageBoxA(0, msg, "error", MB_OK);
      exit(1);
    }

    static bool &sound_disabled() { static bool instance; return instance; }
  };
}
