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
  struct stat buffer;
  return stat(path, &buffer) == 0;
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

  FILE *file = fopen(path, mode_str);
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
    fclose(HANDLE(handle));
    handle->handle = nullptr;
    handle->is_valid = false;
  }
}

bool read_line(File *handle, str *line_buf) {
  if (!handle->handle)
    return false;
  char buffer[32000];
  if (fgets(buffer, sizeof(buffer), HANDLE(handle)) != 0) {
    usize length = strlen(buffer);
    *line_buf = reinterpret_cast<str>(
        alloc((sizeof(char) * length) + 1, mem_tag::STRING));
    strcpy(*line_buf, buffer);
    return true;
  }
  return false;
}

bool write_line(File *handle, cstr text) {
  if (!handle->handle)
    return false;
  i32 result = fputs(text, HANDLE(handle));
  if (result != EOF) {
    result = fputc('\n', HANDLE(handle));
  }

  fflush(HANDLE(handle));
  return result != EOF;
}

bool read(File *handle, usize data_size, ptr out_data, usize *out_bytes_read) {
  if (!handle->handle)
    return false;
  *out_bytes_read = fread(out_data, 1, data_size, HANDLE(handle));
  return *out_bytes_read == data_size;
}

bool read_all_bytes(File *handle, bytes *out_bytes, usize *out_bytes_read) {
  if (!handle->handle)
    return false;
  fseek(HANDLE(handle), 0, SEEK_END);
  usize size = ftell(HANDLE(handle));
  rewind(HANDLE(handle));

  *out_bytes =
      reinterpret_cast<bytes>(alloc(sizeof(u8) * size, mem_tag::STRING));
  *out_bytes_read = fread(*out_bytes, 1, size, HANDLE(handle));
  return *out_bytes_read == size;
}

bool write(File *handle, usize data_size, roptr data,
           usize *out_bytes_written) {
  if (!handle->handle)
    return false;
  *out_bytes_written = fwrite(data, 1, data_size, HANDLE(handle));
  if (*out_bytes_written != data_size)
    return false;
  fflush(HANDLE(handle));
  return true;
}

} // namespace ns::fs
