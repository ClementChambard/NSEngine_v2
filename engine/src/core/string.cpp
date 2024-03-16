#include "./string.h"
#include <cstdio>
#include <cstring>
#include <ctype.h>

#include "./logger.h"
#include "./memory.h"

// #ifndef _MSC_VER
// #include <strings.h>
// #endif

namespace ns {

Str::Str(cstr s) : Slice<char>(Slice<char>::from_parts(s, string_length(s))) {}

String::String(cstr s) {
  if (!s)
    return;
  usize length = string_length(s);
  m_data = ns::alloc_n<char>(length + 1, MemTag::STRING);
  mem_copy(m_data, s, length);
  m_data[length] = 0;
}

void String::resize(usize, char) {
  NS_WARN("called resize on String. Does nothing");
}

void String::push(char c) {
  if (m_size >= m_capacity - 1) {
    reserve(m_capacity == 0 ? FIRST_CAPACITY : m_capacity * CAPACITY_MUL);
  }
  m_data[m_size++] = c;
  m_data[m_size] = 0;
}

void String::push(cstr s) {
  usize length = string_length(s);
  if (m_size + length >= m_capacity - 1) {
    usize new_capacity =
        m_capacity == 0 ? FIRST_CAPACITY : m_capacity * CAPACITY_MUL;
    usize actual_new = length > new_capacity ? length + 1 : new_capacity;
    reserve(actual_new);
  }
  mem_copy(m_data + m_size, s, length);
  m_size += length;
  m_data[m_size] = 0;
}

void String::push(Str s) {
  usize length = s.len();
  if (m_size + length >= m_capacity - 1) {
    usize new_capacity =
        m_capacity == 0 ? FIRST_CAPACITY : m_capacity * CAPACITY_MUL;
    usize actual_new = length > new_capacity ? length + 1 : new_capacity;
    reserve(actual_new);
  }
  mem_copy(m_data + m_size, s, length);
  m_size += length;
  m_data[m_size] = 0;
}

void String::push(String const &s) {
  usize length = s.m_size;
  if (m_size + length >= m_capacity - 1) {
    usize new_capacity =
        m_capacity == 0 ? FIRST_CAPACITY : m_capacity * CAPACITY_MUL;
    usize actual_new = length > new_capacity ? length + 1 : new_capacity;
    reserve(actual_new);
  }
  mem_copy(m_data + m_size, s.m_data, length);
  m_size += length;
  m_data[m_size] = 0;
}

char String::pop() {
  char c = m_data[m_size - 1];
  m_data[m_size - 1] = 0;
  m_size--;
  return c;
}

void String::erase(usize index) {
  mem_copy(m_data + index, m_data + index + 1,
           (m_size - index - 1) * sizeof(char));
  m_size--;
  m_data[m_size] = 0;
}

bool string_eq(cstr s1, cstr s2) { return std::strcmp(s1, s2) == 0; }

bool string_EQ(cstr s1, cstr s2) {
#if defined(__GNUC__)
  return strcasecmp(s1, s2) == 0;
#elif defined(_MSC_VER)
  return _strcmpi(s1, s2) == 0;
#endif
}

usize string_length(cstr s) { return std::strlen(s); }

pstr string_dup(cstr s) {
  if (!s)
    return nullptr;
  usize length = string_length(s);
  pstr out = reinterpret_cast<pstr>(
      ns::alloc((length + 1) * sizeof(char), MemTag::STRING));
  mem_copy(out, s, length);
  out[length] = 0;
  return out;
}

i32 string_fmt_v(pstr out, usize n, cstr format, __builtin_va_list va_list) {
  if (!out)
    return -1;
  return std::vsnprintf(out, n, format, va_list);
}

i32 string_fmt(pstr out, usize n, cstr format, ...) {
  if (!out)
    return -1;
  __builtin_va_list va;
  __builtin_va_start(va, format);
  int ret = string_fmt_v(out, n, format, va);
  __builtin_va_end(va);
  return ret;
}

pstr string_cpy(pstr dest, cstr src) { return std::strcpy(dest, src); }

pstr string_ncpy(pstr dest, cstr src, usize n) {
  return std::strncpy(dest, src, n);
}

pstr string_trim(pstr s) {
  while (isspace(static_cast<u8>(*s))) {
    s++;
  }
  if (*s) {
    pstr p = s;
    while (*p) {
      p++;
    }
    while (isspace(static_cast<u8>(*(--p)))) {
    }
    p[1] = '\0';
  }
  return s;
}

void string_sub(pstr dest, cstr src, usize start, isize length) {
  if (length == 0) {
    return;
  }
  usize src_len = string_length(src);
  if (start >= src_len) {
    dest[0] = '\0';
    return;
  }
  if (length > 0) {
    isize j = 0;
    for (usize i = start; j < length && src[i]; i++, j++) {
      dest[j] = src[i];
    }
    dest[start + length] = '\0';
  } else {
    usize j = 0;
    for (usize i = start; src[i]; i++, j++) {
      dest[j] = src[i];
    }
    dest[start + j] = '\0';
  }
}

isize string_indexof(cstr s, char c) {
  if (!s)
    return -1;
  for (usize i = 0; s[i]; i++) {
    if (s[i] == c) {
      return i;
    }
  }
  return -1;
}

pstr string_clear(pstr s) {
  if (s)
    s[0] = '\0';
  return s;
}

i32 string_scanf(cstr s, cstr format, ...) {
  if (!s)
    return -1;
  __builtin_va_list va;
  __builtin_va_start(va, format);
  i32 ret = std::vsscanf(s, format, va);
  __builtin_va_end(va);
  return ret;
}

bool string_parse(cstr s, f32 *out) {
  if (!s)
    return false;
  *out = 0.0f;
  return string_scanf(s, "%f", out) != -1;
}

bool string_parse(cstr s, f64 *out) {
  if (!s)
    return false;
  *out = 0.0;
  return string_scanf(s, "%lf", out) != -1;
}

bool string_parse(cstr s, i8 *out) {
  if (!s)
    return false;
  *out = 0;
  return string_scanf(s, "%hhi", out) != -1;
}

bool string_parse(cstr s, i16 *out) {
  if (!s)
    return false;
  *out = 0;
  return string_scanf(s, "%hi", out) != -1;
}

bool string_parse(cstr s, i32 *out) {
  if (!s)
    return false;
  *out = 0;
  return string_scanf(s, "%i", out) != -1;
}

bool string_parse(cstr s, i64 *out) {
  if (!s)
    return false;
  *out = 0;
  return string_scanf(s, "%lli", out) != -1;
}

bool string_parse(cstr s, u8 *out) {
  if (!s)
    return false;
  *out = 0;
  return string_scanf(s, "%hhu", out) != -1;
}

bool string_parse(cstr s, u16 *out) {
  if (!s)
    return false;
  *out = 0;
  return string_scanf(s, "%hu", out) != -1;
}

bool string_parse(cstr s, u32 *out) {
  if (!s)
    return false;
  *out = 0;
  return string_scanf(s, "%u", out) != -1;
}

bool string_parse(cstr s, u64 *out) {
  if (!s)
    return false;
  *out = 0;
  return string_scanf(s, "%llu", out) != -1;
}

bool string_parse(cstr s, bool *out) {
  if (!s)
    return false;
  if (string_eq(s, "1") || string_eq(s, "true")) {
    *out = true;
    return true;
  }
  if (string_eq(s, "0") || string_eq(s, "false")) {
    *out = false;
    return true;
  }
  return false;
}

} // namespace ns
