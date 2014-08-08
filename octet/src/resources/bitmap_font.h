////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Container for bitmap font metrics
//

namespace octet { namespace resources {
  /// Bitmap font class. Uses the Angelcode BMFont .fnt file format
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
    float uscale;
    float vscale;

    // transient info, built by update()
    const info *finfo;
    const common *fcommon;
    const kern *fkern;
    hash_map<unsigned, const char_info *> char_map;

public:
    /// Output vertex format used by the mesh generator.
    struct vertex {
      float x, y, z;
      float u, v;
      unsigned color;
    };

private:
    // add a new vertex to the buffer
    vertex *add_vertex(vertex *vtx, int xdraw, int ydraw, int x, int y, unsigned color) {
      vtx->x = (float)xdraw;
      vtx->y = (float)ydraw;
      vtx->z = 0;
      vtx->u = x * uscale;
      vtx->v = 1.0f - y * vscale;
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

    // find the next space or hyphen
    const char *find_word_end(const char *src, const char *src_max) {
      // a space or hyphen on its own is a word
      if (src < src_max && (*src == ' ' || *src == '-')) {
        src++;
      } else {
        // otherwise skip to a space or hyphen
        while (src < src_max && *src != ' ' && *src != '\n' && *src != '-') ++src;
        if (src < src_max && *src == '-') ++src;
      }
      return src;
    }

    // find how many pixels we move if we render this string
    int find_escapement(const char *src, const char *src_max) {
      int result = 0;
      while (src < src_max) {
        unsigned chr = decode_utf8(src);
        int index = char_map.get_index(chr);
        if (index >= 0) {
          const char_info *ci = char_map.get_value(index);
          if (ci) {
            result += s2(ci->xadvance);
          }
        }
      }
      return result;
    }

    // create other data items from font_info
    void update() {
      finfo = 0;
      fcommon = 0;
      fkern = 0;

      if (font_info.empty()) return;

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
            log("%d %08x %p %d\n", i, char_map.key(i), char_map.value(i), char_map.get_index(char_map.key(i)));
          }*/
        } else if (*ptr == 5) {
          fkern = (const kern*)(ptr + 5);
        }
        ptr = next;
      }
    }

    // draw a word by creating a serues of quads.
    unsigned render_chars(unsigned num_quads, int xdraw, int ydraw, vertex *vtx, uint32_t *idx, const char *src, const char *src_max, bool left_to_right, unsigned &color) {
      vtx += num_quads * 4;
      idx += num_quads * 6;
      while (src < src_max) {
        unsigned chr = decode_utf8(src);

        int index = char_map.get_index(chr);
        if (index >= 0) {
          const char_info *ci = char_map.get_value(index);
          if (ci) {
            unsigned x = u2(ci->x);
            unsigned y = u2(ci->y);
            unsigned width = u2(ci->width);
            unsigned height = u2(ci->height);
            int xoffset = s2(ci->xoffset);
            int yoffset = s2(ci->yoffset);
            int xadvance = s2(ci->xadvance);

            if (!left_to_right) xdraw -= xadvance;

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

            num_quads++;

            if (left_to_right) xdraw += xadvance;
          }
        }
      }
      return num_quads;
    }


  public:
    RESOURCE_META(bitmap_font)

    /// Create new bitmap font info object given the URL of a .fnt file
    bitmap_font(int page_width=1, int page_height=1, const char *fnt_file = 0) {
      finfo = 0;
      fcommon = 0;
      fkern = 0;

      uscale = 1.0f / page_width;
      vscale = 1.0f / page_height;
      if (fnt_file) {
        app_utils::get_url(font_info, fnt_file);
        update();
      }
    }

    /// Serialize this object.
    void visit(visitor &v) {
      v.visit(font_info, atom_font_info);
      v.visit(uscale, atom_uscale);
      v.visit(vscale, atom_vscale);
      if (v.is_reader()) update();
    }

    /// Build a mesh by combining the string with the bitmap font info.
    unsigned build_mesh(const aabb &bb, vertex *vtx, uint32_t *idx, unsigned max_quads, const char *text, const char *max_text) {
      // defensive coding
      if (!idx || !vtx) return 0;
      if (!text) return 0;
      if (!fcommon) return 0;

      if (!max_text) max_text = text + strlen(text);

      int xmin = (int)bb.get_min().x();
      int ymin = (int)bb.get_min().y();
      int xmax = (int)bb.get_max().x();
      int ymax = (int)bb.get_max().y();

      int line_height = u2(fcommon->lineHeight);

      unsigned num_quads = 0;

      //char arabic_text[] = "أخبار الوطن العربي";

      unsigned color = 0xffffffff;
      bool left_to_right = true;
      int base = s2(fcommon->base);

      int xdraw = left_to_right ? xmin : xmax;
      int ydraw = ymax - line_height + base;

      int index = char_map.get_index(' ');
      const char_info *ci = index >= 0 ? char_map.get_value(index) : 0;
      int space_size = ci ? s2(ci->xadvance) : line_height / 2;

      for (const char *src = text; src < max_text && *src && num_quads < max_quads; ) {
        const char *word_end = find_word_end(src, max_text);
        if (word_end == src) break; // avoid infinite loops

        int escapement = find_escapement(src, word_end);

        // handle word wrap
        if (left_to_right) {
          // latin/asian text
          if (xdraw + escapement > xmax) {
            if (xmin + escapement <= xmax) {
              // move to next line
              xdraw = xmin;
              ydraw -= line_height;
            } else {
              // for now: overrun line
            }
          }
        } else {
          // arabic text
          if (xdraw - escapement < xmin) {
            if (xmax - escapement >= xmin) {
              // move to next line
              xdraw = xmax;
              ydraw -= line_height;
            } else {
              // for now: overrun line
            }
          }
        }

        // end of column
        if (ydraw - base < ymin) {
          break;
        }

        num_quads = render_chars(num_quads, xdraw, ydraw, vtx, idx, src, word_end, left_to_right, color);

        src = word_end;

        xdraw += left_to_right ? escapement : -escapement;

        if (src < max_text && *src == ' ') {
          xdraw += left_to_right ? space_size : -space_size;
          src++;
        }
        if (src < max_text && *src == '\n') {
          src++;
          if (left_to_right) {
            xdraw = xmin;
            ydraw -= line_height;
          } else {
            xdraw = xmax;
            ydraw -= line_height;
          }
        }
      }
      return num_quads;
    }
  };
} }
