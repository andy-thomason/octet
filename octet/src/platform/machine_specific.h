////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014 (MIT license)
//
// Framework for OpenGLES2 rendering on multiple platforms.
//
// Machine specific includes
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the 
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
// AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//


static char *get_sprintf_buffer() {
  static int i;
  static char tmp[4][256];
  return tmp[i++ & 3];
}

#if defined(__GENERIC__)
  #include "generic.h"
#elif defined(WIN32)
  #include "direct_show.h"
  #include "windows_specific.h"
  //#include "glut_specific.h"
#elif defined(OCTET_VITA)
  #include "../../external/src/vita_specific.h"
#elif defined(__APPLE__) || defined(OCTET_LINUX)
  #include <unistd.h>
  #include <sys/socket.h>
  #include <sys/ioctl.h>
  #include <fcntl.h>
  #include <netinet/in.h>
  #define OCTET_HOT __attribute__( ( always_inline ) )
  #define ioctlsocket ioctl
  #define closesocket close
  #include "video_capture.h"
  #include "glut_specific.h"
#endif

