////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
//

namespace octet {
  // standard attribute names
  enum attribute {
    attribute_position = 0,
    attribute_pos = 0,
    attribute_blendweight = 1,
    attribute_normal = 2,
    attribute_diffuse = 3,
    attribute_color = 3,
    attribute_specular = 4,
    attribute_tessfactor = 5,
    attribute_fogcoord = 5,
    attribute_psize = 6,
    attribute_blendindices = 7,
    attribute_texcoord = 8,
    attribute_uv = 8,
    attribute_tangent = 14,
    attribute_bitangent = 15,
    attribute_binormal = 15,
  };

  enum key {
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

    // mouse buttons
    key_lmb,
    key_mmb,
    key_rmb,
  };

  class app_common {
    unsigned char keys[256];
    int mouse_x;
    int mouse_y;
    int viewport_x;
    int viewport_y;

  public:
    app_common() {
      // this memset writes 0 to every byte of keys[]
      memset(keys, 0, sizeof(keys));
      mouse_x = mouse_y = 0;
    }

    virtual void draw_world(int x, int y, int w, int h) = 0;
    virtual void app_init() = 0;

    // returns true if a key is down
    bool is_key_down(unsigned key) {
      return keys[key & 0xff] == 1;
    }

    void get_mouse_pos(int &x, int &y) {
      x = mouse_x;
      y = mouse_y;
    }

    void get_viewport_size(int &x, int &y) {
      x = viewport_x;
      y = viewport_y;
    }

    // used by the platform to set a key
    void set_key(unsigned key, bool is_down) {
      keys[key & 0xff] = is_down ? 1 : 0;
    }

    // used by the platform to set a key
    void set_mouse_pos(int x, int y) {
      mouse_x = x;
      mouse_y = y;
    }

    void set_viewport_size(int x, int y) {
      viewport_x = x;
      viewport_y = y;
    }

    static bool can_use_vbos() {
      return false;
    }
  };
}
