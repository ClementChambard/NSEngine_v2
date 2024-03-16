#include "./string.h"

#include "../logger.h"

// #ifndef _MSC_VER
// #include <strings.h>
// #endif

namespace ns {

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
  if (length == 0)
    return;
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
  if (length == 0)
    return;
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
  if (length == 0)
    return;
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

} // namespace ns
