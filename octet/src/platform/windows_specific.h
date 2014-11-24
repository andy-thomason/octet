////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Microsoft Windows specific information

// windows.h contains all the windows-specific definitions such as CreateWindow

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <mmsystem.h>

#include <ShellAPI.h> // for DragAcceptFiles etc.

// undo unwanted definitions that Microsoft make in windows.h
#undef min // Yes, Microsoft really define min!
#undef max // Yes, Microsoft really define max!

// avoid irritating security warnings
#pragma warning(disable : 4996)
#pragma warning(disable : 4345)
#pragma warning(disable : 4530)
#pragma warning(disable : 4799)

// basic windows audio
#pragma comment(lib, "winmm.lib")

// graphics - opengl
#pragma comment(lib, "OpenGL32.Lib")
#include <gl/GL.h>

/*
// audio - openal
#pragma comment(lib, "OpenAL32.lib")
#include "AL/alc.h"
#include "AL/AL.h"
*/

// compute - opencl
#if OCTET_OPENCL
  #ifdef WIN32
    #pragma comment(lib, "../../../lib/x86/OpenCL.lib")
  #else
    #pragma comment(lib, "../../../lib/x86_64/OpenCL.lib")
  #endif

  #include "CL/cl.h"
  #include "CL/cl_gl.h"
#endif

// windows sockets (a BSD socket clone)
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>

// some standard c++ definitions
#include <map>
#include <malloc.h>

// windows only supports OpenGL 1.2 natively
// so we need to extend this by getting the addresses of the extra functions
// ... enough of the OpenGL API to cover ES2 and ES3, the mobile variants
// do not use OpenGL 1.x functions *ever* they are obsolete.
#define GL_APIENTRY __stdcall
#include "gl_defs.h"
#include "al_defs.h"

// include cross platform app helpers, such as texture loaders
#include "app_common.h"

// Put this *only* on hot functions
// the less you use it the better as large functions pollute icache
// __forceinline causes functions to always inline, eliminating the call overhead.
// A limited number uses per program is recommended.
#define OCTET_HOT __forceinline

#include <xmmintrin.h>
#define snprintf sprintf_s

namespace octet {
  class HWND_cmp : public hash_map_cmp {
  public:
    static unsigned get_hash(HWND key) { return fuzz_hash((unsigned)(intptr_t)key); }

    static bool is_empty(HWND key) { return !key; }
  };

  // this is the class that all apps are derived from.
  class app : public app_common {
    HGLRC gl_context;
    HWND window_handle;

    void init_gl_context(HWND window_handle) {
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

      HDC hdc = GetDC(window_handle);

      int pixel_format = ChoosePixelFormat(hdc, &pfd);

      SetPixelFormat(hdc, pixel_format, &pfd);

      gl_context = wglCreateContext(hdc);

      wglMakeCurrent (hdc, gl_context);

      ReleaseDC(window_handle, hdc);

      init_wgl();

      //printf("%s\n", glGetString(GL_EXTENSIONS));
    }

    typedef hash_map<HWND, app*, HWND_cmp> map_t;
    static map_t &map() { static map_t instance; return instance; }

  public:
    app(int argc, char **argv) {
    }

    void init() {
      WSADATA wsa;
      WSAStartup(MAKEWORD(2,2), &wsa);

      HINSTANCE instance = (HINSTANCE)GetModuleHandle(0);
      HBRUSH brush = (HBRUSH) GetStockObject(NULL_BRUSH);
      HICON icon = LoadIcon(0, IDI_ASTERISK);
      HCURSOR cursor = LoadCursor(0, IDC_ARROW);

      static WNDCLASSW wndclass = {
        CS_HREDRAW | CS_VREDRAW, DefWindowProc, 0, 0, instance,
        icon, cursor, brush, 0, L"MyClass"
      };
      RegisterClassW (&wndclass);

      gl_context = 0;
     
      window_handle = CreateWindowW(L"MyClass", L"octet",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 768, 768,
        NULL, NULL, wndclass.hInstance, (LPVOID)this
      );

      map()[window_handle] = this;

      // enable drag and drop of files
      DragAcceptFiles(window_handle, TRUE);

      // register interest in USB devices (this may include game controllers)
      RAWINPUTDEVICE devices[1];
      // mouse input device (see http://www.usb.org/developers/hidpage/Hut1_12v2.pdf)
      devices[0].usUsagePage = 1; devices[0].usUsage = 2; devices[0].dwFlags = 0; devices[0].hwndTarget = 0;
      RegisterRawInputDevices(devices, 1, sizeof(RAWINPUTDEVICE));

      init_gl_context(window_handle);

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

      POINT mouse_pos;
      GetCursorPos(&mouse_pos);

      ScreenToClient(window_handle, &mouse_pos);
      set_mouse_pos(mouse_pos.x, mouse_pos.y);

      RECT rect;
      GetClientRect(window_handle, &rect);
      set_viewport_size(rect.right - rect.left, rect.bottom - rect.top);

      begin_frame();

      draw_world(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
      inc_frame_number();

      end_frame();

      SwapBuffers(hdc);

      wglMakeCurrent (hdc, NULL);
      ReleaseDC(window_handle, hdc);
    }

    void disable_cursor() const {
      ShowCursor(FALSE);
      SetCapture(window_handle);
    }

    void enable_cursor() const {
      ShowCursor(TRUE);
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

    static void handle_file_drop(app *app, HDROP drop) {
      dynarray<string> &queue = app->access_load_queue();
      unsigned num_files = DragQueryFileW(drop, 0xFFFFFFFF, 0, 0);
      for (unsigned i = 0; i != num_files; ++i) {
        TCHAR utf16_filename[MAX_PATH];
        DragQueryFile(drop, i, utf16_filename, sizeof(utf16_filename));
        queue.push_back(utf16_filename);
      }
      DragFinish(drop);
    }

    // usb 
    static void handle_usb_input(app *app, MSG &msg) {
      UINT size = 0;
      GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
      //printf("gri %d\n", size);
      if (size < 0x1000) {
        uint8_t buffer[0x1000];
        GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, buffer, &size, sizeof(RAWINPUTHEADER));
        RAWINPUT *ri = (RAWINPUT*)buffer;
        switch (ri->header.dwType) {
          case RIM_TYPEKEYBOARD: {
          } break;
          case RIM_TYPEMOUSE: {
            app->accumulate_absolute_mouse_movement(ri->data.mouse.lLastX, ri->data.mouse.lLastY);
          } break;
        }
      }
    }

    static void run_all_apps() {
      map_t &m = map();
      MSG msg;     
      for(;;) {
        // todo: get notification when the windows close.
        while (PeekMessage(&msg, 0, 0, 0, TRUE)) {
          //printf("msg=%04x %02x\n", msg.message, msg.wParam);
          app *app = m[msg.hwnd];
          if (app) {
            if (msg.message == WM_KEYDOWN || msg.message == WM_KEYUP) {
              app->set_key(app::translate((unsigned)msg.wParam), msg.message == WM_KEYDOWN);
            } else if (msg.message == WM_SYSKEYDOWN || msg.message == WM_SYSKEYUP) {
              app->set_key(app::translate((unsigned)msg.wParam), msg.message == WM_SYSKEYDOWN);
            //} else if (msg.message == WM_MOUSEMOVE) {
              //app->set_mouse_pos((unsigned)msg.lParam & 0xffff, (unsigned)msg.lParam >> 16);
            } else if (msg.message == WM_MOUSEWHEEL) {
              app->set_mouse_wheel(app->get_mouse_wheel() + (int)msg.wParam);
            } else if (msg.message == WM_LBUTTONDOWN || msg.message == WM_LBUTTONUP) {
              app->set_key(key_lmb, msg.message == WM_LBUTTONDOWN);
            } else if (msg.message == WM_MBUTTONDOWN || msg.message == WM_MBUTTONUP) {
              app->set_key(key_mmb, msg.message == WM_MBUTTONDOWN);
            } else if (msg.message == WM_RBUTTONDOWN || msg.message == WM_RBUTTONUP) {
              app->set_key(key_rmb, msg.message == WM_RBUTTONDOWN);
            } else if (msg.message == WM_DROPFILES) {
              handle_file_drop(app, (HDROP)msg.wParam);
            } else if (msg.message == WM_INPUT) {
              handle_usb_input(app, msg);
            }
          }
          DispatchMessage (&msg);
        }

        // waste some time. (do not do this in real games!)
        Sleep(1000/30);

        for (int i = 0; i != m.size(); ++i) {
          // note: because Win8 generates an invisible window, we need to check m.value(i)
          if (m.get_key(i) && m.get_value(i)) {
            m.get_value(i)->render();
          }
        }

        Fake_AL_context()->update();
      }
    }

    static void error(const char *msg) {
      MessageBoxA(0, msg, "error", MB_OK);
      exit(1);
    }

    static bool &sound_disabled() { static bool instance; return instance; }

  };

}
