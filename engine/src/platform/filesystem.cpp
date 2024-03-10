#include "./filesystem.h"

#include "../core/logger.h"
#include "../core/ns_memory.h"

#include <cstdio>
#include <cstring>
#include <sys/stat.h>

namespace ns::fs {

#define HANDLE(handle) reinterpret_cast<FILE *>(handle->handle)
#define FILE_MODE_READ static_cast<int>(Mode::READ)
#define FILE_MODE_WRITE static_cast<int>(Mode::WRITE)

bool exists(cstr path) {
#ifdef _MSC_VER
  struct _stat buffer;
  return _stat(path, &buffer);
#else
  struct stat buffer;
  return stat(path, &buffer) == 0;
#endif
}

bool open(cstr path, Mode mode, bool binary, File *out_handle) {
  out_handle->is_valid = false;
  out_handle->handle = 0;
  cstr mode_str;
  int _mode = static_cast<int>(mode);
  if ((_mode & FILE_MODE_READ) != 0 && (_mode & FILE_MODE_WRITE) != 0) {
    mode_str = binary ? "w+b" : "w+";
  } else if ((_mode & FILE_MODE_READ) != 0 && (_mode & FILE_MODE_WRITE) == 0) {
    mode_str = binary ? "rb" : "r";
  } else if ((_mode & FILE_MODE_READ) == 0 && (_mode & FILE_MODE_WRITE) != 0) {
    mode_str = binary ? "wb" : "w";
  } else {
    NS_ERROR("Invalid mode passed while trying to open file: '%s'", path);
    return false;
  }

  FILE *file = std::fopen(path, mode_str);
  if (!file) {
    NS_ERROR("Error opening file: '%s'", path);
    return false;
  }

  out_handle->handle = file;
  out_handle->is_valid = true;

  return true;
}

void close(File *handle) {
  if (handle->handle) {
    std::fclose(HANDLE(handle));
    handle->handle = nullptr;
    handle->is_valid = false;
  }
}

bool fsize(File *handle, usize *out_size) {
  if (handle->handle) {
    std::fseek(HANDLE(handle), 0, SEEK_END);
    *out_size = std::ftell(HANDLE(handle));
    std::rewind(HANDLE(handle));
    return true;
  }
  return false;
}

bool read_line(File *handle, usize max_length, str *line_buf,
               u64 *out_line_length) {
  if (!handle->handle || !line_buf || !out_line_length || max_length == 0)
    return false;
  str buf = *line_buf;
  if (std::fgets(buf, max_length, HANDLE(handle)) != nullptr) {
    *out_line_length = std::strlen(buf);
    return true;
  }
  return false;
}

bool write_line(File *handle, cstr text) {
  if (!handle->handle)
    return false;
  i32 result = std::fputs(text, HANDLE(handle));
  if (result != EOF) {
    result = std::fputc('\n', HANDLE(handle));
  }

  std::fflush(HANDLE(handle));
  return result != EOF;
}

bool read(File *handle, usize data_size, ptr out_data, usize *out_bytes_read) {
  if (!handle->handle)
    return false;
  *out_bytes_read = std::fread(out_data, 1, data_size, HANDLE(handle));
  return *out_bytes_read == data_size;
}

bool read_all_bytes(File *handle, bytes out_bytes, usize *out_bytes_read) {
  if (!handle->handle || !out_bytes)
    return false;
  usize s = 0;
  if (!fsize(handle, &s))
    return false;

  usize nread = std::fread(out_bytes, 1, s, HANDLE(handle));
  if (out_bytes_read) {
    *out_bytes_read = nread;
  }
  return nread == s;
}

bool read_all_text(File *handle, str out_text, usize *out_bytes_read) {
  if (!handle->handle || !out_text)
    return false;
  usize s = 0;
  if (!fsize(handle, &s))
    return false;

  usize nread = std::fread(out_text, 1, s, HANDLE(handle));
  if (out_bytes_read) {
    *out_bytes_read = nread;
  }
  return nread == s;
}

bool write(File *handle, usize data_size, roptr data,
           usize *out_bytes_written) {
  if (!handle->handle)
    return false;
  *out_bytes_written = std::fwrite(data, 1, data_size, HANDLE(handle));
  if (*out_bytes_written != data_size)
    return false;
  std::fflush(HANDLE(handle));
  return true;
}

} // namespace ns::fs
