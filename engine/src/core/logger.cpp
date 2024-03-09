#include "./logger.h"
#include "../platform/filesystem.h"
#include "../platform/platform.h"
#include "./asserts.h"
#include "./ns_memory.h"
#include "./ns_string.h"

namespace ns {

struct logger_system_state {
  fs::File log_file_handle;
};

static logger_system_state *state_ptr;

void append_to_log_file(cstr message) {
  if (state_ptr && state_ptr->log_file_handle.is_valid) {
    usize length = string_length(message);
    usize written = 0;
    if (!fs::write(&state_ptr->log_file_handle, length, message, &written)) {
      platform_console_write_error("ERROR writing to console.log.",
                                   static_cast<u8>(log_level::ERROR));
    }
  }
}

bool initialize_logging(usize *memory_requirement, ptr state) {
  *memory_requirement = sizeof(logger_system_state);
  if (state == nullptr) {
    return true;
  }

  state_ptr = reinterpret_cast<logger_system_state *>(state);

  if (!fs::open("console.log", fs::Mode::WRITE, false,
                &state_ptr->log_file_handle)) {
    platform_console_write_error(
        "ERROR: Unable to open console.log for writing.",
        static_cast<u8>(log_level::ERROR));
    return false;
  }

  return true;
}

void shutdown_logging(ptr /*state*/) {
  // TODO(ClementChambard): cleanup logging/write queued entries
}

void log_output(log_level level, cstr message, ...) {
  // TODO(ClementChambard): move to another thread
  cstr level_strings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN] : ",
                           "[INFO] : ", "[DEBUG]: ", "[TRACE]: "};
  bool is_error = level < log_level::WARN;

  char _message[32000];
  mem_zero(_message, sizeof(_message));
  __builtin_va_list arg_ptr;
  __builtin_va_start(arg_ptr, message);
  string_fmt_v(_message, sizeof(_message), message, arg_ptr);
  __builtin_va_end(arg_ptr);

  char out_message[32000];
  string_fmt(out_message, sizeof(out_message), "%s%s\n",
             level_strings[static_cast<usize>(level)], _message);

  if (is_error) {
    platform_console_write_error(out_message, static_cast<u8>(level));
  } else {
    platform_console_write(out_message, static_cast<u8>(level));
  }

  append_to_log_file(out_message);
}

void report_assertion_failure(cstr expression, cstr message, cstr file,
                              isize line) {
  log_output(log_level::FATAL,
             "Assertion Failure: %s, message: '%s', in file: %s, line: %d",
             expression, message, file, line);
}

} // namespace ns
