////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//

namespace octet { namespace resources {
  /// Zip file reader, uses zip_decoder to inflate compressed files.
  /// Zip files are smaller and faster than regular files.
  /// They make updates easier and work will over the internet.
  class zip_file {
    int ref_cnt;
    FILE *the_file;

    struct dir_entry {
      uint32_t offset;
      uint32_t csize;
      uint32_t usize;
      uint32_t compression;
    };

    dictionary<dir_entry> directory;

    zip_decoder decoder;

    // read little endian bytes on any machine
    static unsigned u4(const uint8_t *src) {
      return src[0] + src[1] * 256 + src[2] * 65536 + src[3] * 0x1000000;
    }

    static int s4(const uint8_t *src) {
      return (int32_t)(src[0] + src[1] * 256 + src[2] * 65536 + src[3] * 0x1000000);
    }

    static unsigned u2(const uint8_t *src) {
      return src[0] + src[1] * 256;
    }

    static int s2(const uint8_t *src) {
      return (int16_t)(src[0] + src[1] * 256);
    }

  public:
    /// Open a zip file for reading
    zip_file(const char *filename) {
      ref_cnt = 0;
      the_file = fopen(filename, "rb");
      if (!the_file) {
        printf("file %s not found\n", filename);
      } else {
        uint8_t tmp[256];
        unsigned tmp_size = (unsigned)sizeof(tmp);
        fseek(the_file, 0, SEEK_END);
        long file_size = ftell(the_file);
        long search_offset = file_size - (long)tmp_size;
        search_offset = search_offset < 0 ? 0 : search_offset;
        fseek(the_file, search_offset, SEEK_SET);
        fread(&tmp, 1, tmp_size, the_file);
        for( unsigned i = 0; i != tmp_size; ++i) {
          if (u4(tmp + i) == 0x06054b50) {
            dynarray<uint8_t> dir;
            dir.resize(u4(tmp + i + 12));
            fseek(the_file, (long)u4(tmp + i + 16), SEEK_SET);
            fread(&dir[0], 1, dir.size(), the_file);
            for (unsigned i = 0; i + 4 < dir.size();) {
              uint8_t *p = &dir[i];
              if (u4(p) != 0x02014b50) break;
              struct dir_entry d;
              d.compression = u2(p + 10);
              d.csize = u4(p + 20);
              d.usize = u4(p + 24);
              unsigned file_name_len = u2(p + 28);
              unsigned extra_len = u2(p + 30);
              string file;
              file.set((const char*)(p + 46), file_name_len);
              i += 46 + file_name_len + extra_len;
              d.offset = u4(p + 42);// + (46 + file_name_len + extra_len);
              for (unsigned i = 0; file[i]; ++i) {
                if (file[i] == '\\') file[i] = '/';
              }
              //printf("%s\n", file.c_str());
              directory[file] = d;
            }
            break;
          }
        }
      }
    }

    /// close the zip file
    ~zip_file() {
      if (the_file) fclose(the_file);
    }

    /// allow ref<zip_file>
    void add_ref() {
      ref_cnt++;
    }

    /// allow ref<zip_file>
    void release() {
      if (--ref_cnt) {
        delete this;
      }
    }

    /// get a file from a zip file, this is called from get_url with a zip:// prefix.
    void get_file(dynarray<uint8_t> &buffer, const char *file) {
      int index = directory.get_index(file);
      if (index < 0) return;
      if (!the_file) return;
      const dir_entry &d = directory.get_value(index);
      buffer.resize(d.usize);
      fseek(the_file, d.offset, SEEK_SET);
      /*local file header signature     4 bytes  (0x04034b50) 0
      version needed to extract       2 bytes 4
      general purpose bit flag        2 bytes 6
      compression method              2 bytes 8
      last mod file time              2 bytes 10
      last mod file date              2 bytes 12
      crc-32                          4 bytes 14
      compressed size                 4 bytes 18
      uncompressed size               4 bytes 22
      file name length                2 bytes 26
      extra field length              2 bytes 28 / 30*/

      uint8_t tmp[30];
      fread(tmp, 1, sizeof(tmp), the_file);
      if (u4(tmp) != 0x04034b50) return;
      unsigned extra = u2(tmp + 26) + u2(tmp + 28);
      fseek(the_file, d.offset + 30 + extra, SEEK_SET);
      if (d.compression == 0) {
        fread(buffer.data(), 1, d.usize, the_file);
      } else if (d.compression == 8) {
        dynarray<uint8_t> uncomp(d.csize + 4); // note: + 4 bytes prevents decode() overflowing
        fread(uncomp.data(), 1, d.csize, the_file);
        decoder.decode(buffer.data(), buffer.data() + d.usize, uncomp.data(), uncomp.data() + d.csize);
      }
    }
  };
} }
