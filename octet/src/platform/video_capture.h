////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Microsoft Windows specific information

namespace octet {
  // dummy video capture class
  class video_capture {
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
  };
}
