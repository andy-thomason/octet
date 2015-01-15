////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014 (MIT license)
//
// Framework for OpenGLES2 rendering on multiple platforms.
//
// Platform specific includes
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

#ifndef OCTET_OPENCL
  #define OCTET_OPENCL 0
#endif

#if defined(WIN32)
  #define OCTET_SSE 1
  #pragma warning(disable : 4996)
#endif

#if OCTET_MAC
  #define OCTET_SSE 1
  #define GL_UNIFORM_BUFFER 0
#endif

// use <> to include from standard directories
// use "" to include from our own project
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>
#include <string>
#include <vector>
#include <array>
#include <deque>
#include <queue>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <fstream>

#if defined(WIN32)
  #include <direct.h>
#endif

namespace octet {
  /// write some text to log.txt
  inline static FILE * log(const char *fmt, ...) {
    static FILE *file;
    va_list list;
    va_start(list, fmt);
    if (!file) file = fopen("log.txt", "w");
    vfprintf(file, fmt, list);
    va_end(list);
    //fflush(file);
    return file;
  }
}

