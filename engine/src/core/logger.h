#ifndef LOGGER_HEADER_INCLUDED
#define LOGGER_HEADER_INCLUDED

#include "../defines.h"

#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

#if NS_RELEASE == 1
#define LOG_DEBUG_ENABLED 0
#define LOG_TRACE_ENABLED 0
#endif

namespace ns {

enum class LogLevel {
  FATAL = 0,
  ERROR = 1,
  WARN = 2,
  INFO = 3,
  DEBUG = 4,
  TRACE = 5,
};

bool initialize_logging(usize *memory_requirement, ptr state);
void shutdown_logging(ptr state);

NS_API void log_output(LogLevel level, cstr message, ...);

} // namespace ns

#define NS_FATAL(message, ...)                                                 \
  ::ns::log_output(::ns::LogLevel::FATAL, message, ##__VA_ARGS__);

#define NS_ERROR(message, ...)                                                 \
  ::ns::log_output(::ns::LogLevel::ERROR, message, ##__VA_ARGS__);

#if LOG_WARN_ENABLED == 1
#define NS_WARN(message, ...)                                                  \
  ::ns::log_output(::ns::LogLevel::WARN, message, ##__VA_ARGS__);
#else
#define NS_WARN(message, ...)
#endif

#if LOG_INFO_ENABLED == 1
#define NS_INFO(message, ...)                                                  \
  ::ns::log_output(::ns::LogLevel::INFO, message, ##__VA_ARGS__);
#else
#define NS_INFO(message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
#define NS_DEBUG(message, ...)                                                 \
  ::ns::log_output(::ns::LogLevel::DEBUG, message, ##__VA_ARGS__);
#else
#define NS_DEBUG(message, ...)
#endif

#if LOG_TRACE_ENABLED == 1
#define NS_TRACE(message, ...)                                                 \
  ::ns::log_output(::ns::LogLevel::TRACE, message, ##__VA_ARGS__);
#else
#define NS_TRACE(message, ...)
#endif

#endif // LOGGER_HEADER_INCLUDED
