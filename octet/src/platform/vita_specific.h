////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Microsoft Windows specific information

// windows.h contains all the windows-specific definitions such as CreateWindow

// some standard c++ definitions
#include <stdlib.h>

#include "gl_skeleton.h"
#include "al_defs.h"

// include cross platform app helpers, such as texture loaders
#include "app_common.h"

#include <net.h>
#define FIONBIO 1

inline void ioctlsocket(int socket, unsigned kind, unsigned long *up) {
}

#define sockaddr SceNetInAddr
#define sockaddr_in SceNetSockaddrIn

#define AF_INET SCE_NET_AF_INET
#define SOCK_STREAM SCE_NET_SOCK_STREAM
#define INADDR_ANY SCE_NET_INADDR_ANY

inline int socket(int domain, int type, int protocol) {
  return sceNetSocket("sock", domain, type, protocol);
}

inline int accept(int s, SceNetSockaddrArg addr, SceNetSocklen_t *addrlen) {
  return sceNetAccept(s, addr, addrlen);
}

inline int bind(int s, sockaddr *addr, size_t addrlen) {
  return sceNetBind(s, (SceNetSockaddrConstArg)addr, addrlen);
}

#define htonl sceNetHtonl
#define htons sceNetHtons
#define send sceNetSend
#define recv sceNetRecv
#define listen sceNetListen
#define closesocket sceNetSocketClose

// Put this *only* on hot functions
// the less you use it the better as large functions pollute icache
// __forceinline causes functions to always inline, eliminating the call overhead.
// A limited number uses per program is recommended.
#define OCTET_HOT __attribute__( ( always_inline ) )

namespace octet {
  // this is the class that all apps are derived from.
  class app : public app_common {

  public:
    app(int argc, char **argv) {
    }

    void init() {

      app_init();
    }

    void render() {

      draw_world(0, 0, 1024, 768);

      //SwapBuffers(hdc);

    }

    /*static unsigned translate(unsigned key) {
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
    }*/

    ~app() {
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
      for(;;) {

        Fake_AL_context()->update();
      }
    }

    static void error(const char *msg) {
      printf("%s\n", msg);
      exit(1);
    }

    static bool &sound_disabled() { static bool instance; return instance; }

  };
}
