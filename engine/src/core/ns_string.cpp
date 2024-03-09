#include "./ns_string.h"
#include <cstdio>
#include <cstring>

// #ifndef _MSC_VER
// #include <strings.h>
// #endif

namespace ns {

bool string_eq(cstr s1, cstr s2) { return std::strcmp(s1, s2) == 0; }

bool string_EQ(cstr s1, cstr s2) {
#if defined(__GNUC__)
  return strcasecmp(s1, s2) == 0;
#elif defined(_MSC_VER)
  return _strcmpi(s1, s2) == 0;
#endif
}

usize string_length(cstr s) { return std::strlen(s); }

str string_dup(cstr s) {
  if (!s)
    return nullptr;
  usize length = string_length(s);
  str out = new char[length + 1];
  std::memcpy(out, s, length);
  out[length] = 0;
  return out;
}

i32 string_fmt_v(str out, usize n, cstr format, __builtin_va_list va_list) {
  if (!out)
    return -1;
  return std::vsnprintf(out, n, format, va_list);
}

i32 string_fmt(str out, usize n, cstr format, ...) {
  if (!out)
    return -1;
  __builtin_va_list va;
  __builtin_va_start(va, format);
  int ret = string_fmt_v(out, n, format, va);
  __builtin_va_end(va);
  return ret;
}

} // namespace ns
