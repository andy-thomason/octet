////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
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
    bitset<256> keys;
    bitset<256> prev_keys;
    int mouse_x;
    int mouse_y;
    int mouse_wheel;
    int mouse_abs_x;
    int mouse_abs_y;
    int viewport_x;
    int viewport_y;
    int frame_number;
    bool is_gles3;
    video_capture video_capture_;

    // queue of files to load
    dynarray<string> load_queue;

  public:
    app_common() {
      keys.clear();
      prev_keys.clear();
      // this memset writes 0 to every byte of keys[]
      mouse_x = mouse_y = 0;
      mouse_abs_x = mouse_abs_y = 0;
      is_gles3 = false;
      frame_number = 0;
    }

    virtual ~app_common() {
    }

    void begin_frame() {
      //char buf[256+5];
      //printf("p %s\n", prev_keys.toString(buf, sizeof(buf)));
      //printf("k %s\n\n", keys.toString(buf, sizeof(buf)));
    }

    void end_frame() {
      prev_keys = keys;
    }

    virtual void draw_world(int x, int y, int w, int h) = 0;
    virtual void app_init() = 0;

    /// returns true if a key is down
    bool is_key_down(unsigned key) {
      return keys[key & 0xff] != 0;
    }

    /// returns true if a key has gone down this frame
    bool is_key_going_down(unsigned key) {
      return keys[key & 0xff] != 0 && prev_keys[key & 0xff] == 0;
    }

    /// returns true if a key has gone down this frame
    bool is_key_going_up(unsigned key) {
      return keys[key & 0xff] != 0 && prev_keys[key & 0xff] == 0;
    }

    /// return the current set of keys down.
    bitset<256> get_keys() const {
      return keys;
    }

    /// return the previous set of keys down
    bitset<256> get_prev_keys() const {
      return prev_keys;
    }

    /// return the previous set of keys down
    bitset<256> get_keys_going_down() const {
      return keys & ~prev_keys;
    }

    /// return the previous set of keys down
    bitset<256> get_keys_going_up() const {
      return ~keys & prev_keys;
    }

    void get_mouse_pos(int &x, int &y) {
      x = mouse_x;
      y = mouse_y;
    }

    int get_mouse_wheel() {
      return mouse_wheel;
    }

    void get_viewport_size(int &x, int &y) {
      x = viewport_x;
      y = viewport_y;
    }

    int get_frame_number() {
      return frame_number;
    }

    void inc_frame_number() {
      frame_number++;
    }

    dynarray<string> &access_load_queue() {
      return load_queue;
    }

    video_capture *get_video_capture() {
      return &video_capture_;
    }

    // used by the platform to set a key
    void set_key(unsigned key, bool is_down) {
      if (is_down) {
        keys.setbit(key & 0xff);
      } else {
        keys.clearbit(key & 0xff);
      }
    }

    // used by the platform to set mouse positions
    void set_mouse_pos(int x, int y) {
      mouse_x = x;
      mouse_y = y;
    }

    // we may recieve several WM_INPUT messages during the frame,
    // so accumulate.
    void accumulate_absolute_mouse_movement(int x, int y) {
      mouse_abs_x += x;
      mouse_abs_y += y;
    }

    void get_absolute_mouse_movement(int &x, int &y) {
      x = mouse_abs_x;
      y = mouse_abs_y;
    }

    // used by the platform to set mouse wheel clicks
    void set_mouse_wheel(int z) {
      mouse_wheel = z;
    }

    void set_viewport_size(int x, int y) {
      // make the viewport size even, so the centre is always
      // at the centre of a pixel.
      viewport_x = x & ~1; // ie, clear the bottom bit.
      viewport_y = y & ~1;
      //printf("set_viewport_size: %03x %03x\n", viewport_x, viewport_y);
    }

    static bool can_use_vbos() {
      return false;
    }

    bool get_is_gles3() {
      return is_gles3;
    }

    void set_is_gles3(bool value) {
      is_gles3 = value;
    }

    // use the allocator to allocate this resource and its child classes
    void *operator new (size_t size) {
      return allocator::malloc(size);
    }

    // use the allocator to free this resource and its child classes
    void operator delete (void *ptr, size_t size) {
      return allocator::free(ptr, size);
    }

  };
}
