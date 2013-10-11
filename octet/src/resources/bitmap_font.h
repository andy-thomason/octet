////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Container for bitmap font metrics
//

namespace octet {
  class bitmap_font : public resource {
    // Angelcode BMFont .fnt file format

    // font/fount metrics
    struct info {
      uint8_t fontSize[2];
      uint8_t bitField;
      uint8_t charSet;
      uint8_t stretchH[2];
      uint8_t aa;
      uint8_t paddingUp;
      uint8_t paddingRight;
      uint8_t paddingDown;
      uint8_t paddingLeft;
      uint8_t spacingHoriz;
      uint8_t spacingVert;
      uint8_t outline;
      uint8_t fontName[1];
    };

    // where to find resources
    struct common {
      uint8_t lineHeight[2];
      uint8_t base[2];
      uint8_t scaleW[2];
      uint8_t scaleH[2];
      uint8_t pages[2];
      uint8_t bitField;
      uint8_t alphaChnl;
      uint8_t redChnl;
      uint8_t greenChnl;
      uint8_t blueChnl;
    };

    // structure for each character
    struct char_info {
      uint8_t id[4];
      uint8_t x[2];
      uint8_t y[2];
      uint8_t width[2];
      uint8_t height[2];
      uint8_t xoffset[2]; // s2
      uint8_t yoffset[2]; // s2
      uint8_t xadvance[2]; // s2
      uint8_t page;
      uint8_t chnl;
    };

    // kerning pairs eg. in "To" we can tuck the "o" under the "T"
    struct kern {
      uint8_t first[4];
      uint8_t second[4];
      uint8_t amount[2];
    };

    // serialized info
    dynarray<uint8_t> font_info;

    // transient info, built by update()
    const info *finfo;
    const common *fcommon;
    const kern *fkern;
    hash_map<unsigned, const char_info *> char_map;

public:
    // output format
    struct vertex {
      float x, y, z;
      float u, v;
      unsigned color;
    };

private:
    // add a new vertex to the buffer
    static vertex *add_vertex(vertex *vtx, int xdraw, int ydraw, int x, int y, unsigned color) {
      vtx->x = (float)xdraw;
      vtx->y = (float)ydraw;
      vtx->z = 0;
      vtx->u = x * (1.0f/256);
      vtx->v = 1.0f - y * (1.0f/256);
      vtx->color = color;
      return vtx + 1;
    }

    // read little endian bytes on any machine
    static unsigned u4(const uint8_t *src) {
      return src[0] + src[1] * 256 + src[2] * 65536 + src[3] * 0x1000000;
    }

    static unsigned u2(const uint8_t *src) {
      return src[0] + src[1] * 256;
    }

    static short s2(const uint8_t *src) {
      return (short)(src[0] + src[1] * 256);
    }

    // http://en.wikipedia.org/wiki/UTF-8
    static unsigned decode_utf8(const char *&src) {
      unsigned chr = *src++;
      if ((chr & 0x80) != 0x00) {
        unsigned mask = 0x7f;
        while ((*src & 0xc0) == 0x80) {
          chr = (chr << 6) | (*src++ & 0x3f);
          mask = (mask << 5) | mask;
        }
        chr &= mask;
      }
      return chr;
    }

    // create other data items from font_info
    void update() {
      if (font_info.is_empty()) return;

      finfo = 0;
      fcommon = 0;
      fkern = 0;

      const uint8_t *ptr = &font_info[0];
      const uint8_t *ptr_max = ptr + font_info.size();
      if(ptr[0] != 'B' || ptr[1] != 'M' || ptr[2] != 'F' || ptr[3] != 3) return;

      ptr += 4;

      char_map.clear();

      while (ptr < ptr_max) {
        unsigned size = u4(ptr+1);
        const uint8_t *next = (ptr + 5) + size;
        //printf("%d %d\n", *ptr, size);
        if (*ptr == 1) {
          finfo = (const info*)(ptr + 5);
        } else if (*ptr == 2) {
          fcommon = (const common*)(ptr + 5);
        } else if (*ptr == 3) {
          // pages
        } else if (*ptr == 4) {
          const char_info *chars = (const char_info*)(ptr + 5);
          for (unsigned i = 0; i < size; i += sizeof(char_info)) {
            char_map[u4(chars->id)] = chars++;
          }
          /*for (unsigned i = 0; i != char_map.size(); ++i) {
            app_utils::log("%d %08x %p %d\n", i, char_map.key(i), char_map.value(i), char_map.get_index(char_map.key(i)));
          }*/
        } else if (*ptr == 5) {
          fkern = (const kern*)(ptr + 5);
        }
        ptr = next;
      }
    }

  public:
    RESOURCE_META(bitmap_font)

    bitmap_font(const char *fnt_file = 0) {
      if (fnt_file) {
        app_utils::get_url(font_info, fnt_file);
        update();
      }
    }

    // serialize this object.
    void visit(visitor &v) {
      v.visit(font_info, atom_font_info);
      update();
    }

    unsigned build_mesh(int &xdraw, int &ydraw, vertex *vtx, uint32_t *idx, unsigned max_quads, const char *text, const char *max_text) {
      unsigned num_quads = 0;

      //char arabic_text[] = "أخبار الوطن العربي";

      unsigned color = 0xffffffff;
      bool left_to_right = true;

      for (const char *src = text; src < max_text && *src && num_quads < max_quads; ) {
        unsigned chr = decode_utf8(src);

        int index = char_map.get_index(chr);
        if (index >= 0) {
          const char_info *ci = char_map.value(index);
          if (ci) {
            unsigned x = u2(ci->x);
            unsigned y = u2(ci->y);
            unsigned width = u2(ci->width);
            unsigned height = u2(ci->height);
            int xoffset = s2(ci->xoffset);
            int yoffset = s2(ci->yoffset);
            int xadvance = s2(ci->xadvance);

            if (!left_to_right) xdraw -= xadvance;

            if (idx && vtx) {
              // 0 1
              // 2 3
              idx[0] = num_quads * 4 + 0;
              idx[1] = num_quads * 4 + 1;
              idx[2] = num_quads * 4 + 2;
              idx[3] = num_quads * 4 + 2;
              idx[4] = num_quads * 4 + 1;
              idx[5] = num_quads * 4 + 3;
              idx += 6;

              vtx = add_vertex(vtx, xdraw + xoffset, ydraw - yoffset, x, y, color);
              vtx = add_vertex(vtx, xdraw + xoffset + width, ydraw - yoffset, x + width, y, color);
              vtx = add_vertex(vtx, xdraw + xoffset, ydraw - yoffset - height, x, y + height, color);
              vtx = add_vertex(vtx, xdraw + xoffset + width, ydraw - yoffset - height, x + width, y + height, color);
            }

            num_quads++;

            if (left_to_right) xdraw += xadvance;
          }
        }
      }
      return num_quads;
    }
  };
}
