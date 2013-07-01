////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
//

class app_common {
  unsigned char keys[256];

public:
  app_common() {
    // this memset writes 0 to every byte of keys[]
    memset(keys, 0, sizeof(keys));
  }

  enum {
    // keys with ascii equivalent, eg. space, esc, enter have their ascii code.
    key_backspace = 8,
    key_tab = 9,
    key_esc = 27,
    key_space = 32,

    // other keys have the following codes:
    key_f1 = 0x80,
    key_f2,
    key_f3,
    key_f4,
    key_f5,
    key_f6,
    key_f7,
    key_f8,
    key_f9,
    key_f10,
    key_f11,
    key_f12,
    key_left,
    key_up,
    key_right,
    key_down,
    key_page_up,
    key_page_down,
    key_home,
    key_end,
    key_insert,
    key_delete,
    key_shift,
    key_ctrl,
    key_alt,
  };

  virtual void draw_world(int x, int y, int w, int h) = 0;
  virtual void app_init() = 0;

  // returns true if a key is down
  bool is_key_down(unsigned key) {
    return keys[key & 0xff] == 1;
  }

  // used by the platform to set a key
  void set_key(unsigned key, bool is_down) {
    keys[key & 0xff] = is_down ? 1 : 0;
  }

  static bool can_use_vbos() {
    return false;
  }
};
