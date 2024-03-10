#ifndef FILESYSTEM_HEADER_INCLUDED
#define FILESYSTEM_HEADER_INCLUDED

#include "../defines.h"

namespace ns::fs {

/**
 * The handle of a file
 * @struct File
 * @field handle The handle of the file
 * @field is_valid Whether the file is valid
 */
struct File {
  ptr handle;
  bool is_valid;
};

/**
 * The mode of a file
 * @enum Mode
 * @field READ The file should be opened for reading
 * @field WRITE The file should be opened for writing
 */
enum class Mode {
  READ = 0x1,
  WRITE = 0x2,
};

/**
 * Checks if a file exists
 * @param path The path of the file
 * @returns True if the file exists
 */
NS_API bool exists(cstr path);

/**
 * Opens a file
 * @param path The path of the file
 * @param mode The mode of the file
 * @param binary Whether the file should be opened in binary mode
 * @param out_handle The handle of the file
 * @returns True if the file was opened successfully
 */
NS_API bool open(cstr path, Mode mode, bool binary, File *out_handle);

/**
 * Closes a file
 * @param handle The handle of the file
 */
NS_API void close(File *handle);

/**
 * Gets the size of a file
 * @param handle The handle of the file
 * @param out_size The size of the file
 * @returns True if the size was retrieved successfully
 */
NS_API bool fsize(File *handle, usize *out_size);

/**
 * Reads a line from a file
 * @param handle The handle of the file
 * @param max_length The maximum length of the line
 * @param line_buf The buffer to store the line in
 * @param out_line_length The length of the line
 * @returns True if the line was read successfully
 */
NS_API bool read_line(File *handle, usize max_length, str *line_buf,
                      u64 *out_line_length);

/**
 * Writes a line to a file
 * @param handle The handle of the file
 * @param text The text to write
 * @returns True if the line was written successfully
 */
NS_API bool write_line(File *handle, cstr text);

/**
 * Reads data from a file
 * @param handle The handle of the file
 * @param data_size The size of the data to read
 * @param out_data The buffer to store the data in
 * @param out_bytes_read The number of bytes read
 * @returns True if the data was read successfully
 */
NS_API bool read(File *handle, usize data_size, ptr out_data,
                 usize *out_bytes_read);

/**
 * Reads all data from a file
 * @param handle The handle of the file
 * @param out_bytes The buffer to store the data in
 * @param out_bytes_read The number of bytes read
 * @returns True if the data was read successfully
 */
NS_API bool read_all_bytes(File *handle, bytes out_bytes,
                           usize *out_bytes_read);

/**
 * Reads all text from a file
 * @param handle The handle of the file
 * @param out_text The buffer to store the text in
 * @param out_bytes_read The number of bytes read
 * @returns True if the text was read successfully
 */
NS_API bool read_all_text(File *handle, str out_text, usize *out_bytes_read);

/**
 * Writes data to a file
 * @param handle The handle of the file
 * @param data_size The size of the data to write
 * @param data The data to write
 * @param out_bytes_written The number of bytes written
 * @returns True if the data was written successfully
 */
NS_API bool write(File *handle, usize data_size, roptr data,
                  usize *out_bytes_written);

} // namespace ns::fs

#endif // FILESYSTEM_HEADER_INCLUDED
