////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// map a file to memory

class file_map {
  #ifdef WIN32
    HANDLE file_handle;
    HANDLE mapping_handle;
  #else
    int file_handle;
  #endif
  uint64_t size;
  const uint8_t *data;
  const char *error;
public:
  file_map(const char *file_name) {
    error = 0;
    data = 0;
    size = 0;

    if (file_name == NULL) {
      error = "no file name";
      return;
    }

    #ifdef WIN32
      mapping_handle = INVALID_HANDLE_VALUE;

      file_handle = CreateFileA(
        file_name, GENERIC_READ, FILE_SHARE_READ, 0,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0
      );

      if (file_handle == INVALID_HANDLE_VALUE) {
        error = "could not open file";
        return;
      }

      DWORD sizehi = 0, sizelo = GetFileSize(file_handle, &sizehi);
      size = ((uint64_t)sizehi << 32) | sizelo;
      mapping_handle = CreateFileMappingA(file_handle, 0, PAGE_READONLY, 0, 0, 0);

      if (mapping_handle == INVALID_HANDLE_VALUE) {
        error = "could not map file";
        return;
      }

      data = (const uint8_t *)MapViewOfFile(mapping_handle, FILE_MAP_READ, 0, 0, 0);
    #else
      //file_handle = open(file_name, O_RDONLY);
      //mmap
    #endif
  }

  ~file_map() {
    #ifdef WIN32
      UnmapViewOfFile(data);
      CloseHandle(file_handle);
      CloseHandle(mapping_handle);
    #else
    #endif
  }

  const char *get_error() const {
    return error;
  }

  const uint8_t *get_data() const {
    return data;
  }

  uint64_t get_size() const {
    return size;
  }
};

