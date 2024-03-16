#ifndef VEC_HEADER_INCLUDED
#define VEC_HEADER_INCLUDED

#include "../core/memory.h"
#include "../core/slice.h"

#include <initializer_list>

namespace ns {

usize string_length(cstr s);

template <typename T, MemTag tag = MemTag::VECTOR> struct Vec {
  static constexpr usize FIRST_CAPACITY = 4;
  static constexpr usize CAPACITY_MUL = 2;
  Vec() = default;

  explicit Vec(usize size, T c = T()) {
    m_size = size;
    m_capacity = size;
    m_data = ns::alloc_n<T>(m_capacity, tag);
    for (usize i = 0; i < size; i++) {
      m_data[i] = c;
    }
  }

  explicit Vec(Slice<T> s) {
    m_size = s.m_count;
    m_capacity = s.m_count;
    m_data = ns::alloc_n<T>(m_capacity, tag);
    mem_copy(m_data, s.m_data, m_size * sizeof(T));
  }

  static Vec from_parts(T const *data, usize count) {
    Vec vec;
    vec.m_data = data;
    vec.m_size = count;
    vec.m_capacity = count;
    return vec;
  }

  Vec(std::initializer_list<T> list) {
    m_size = list.size();
    m_capacity = m_size;
    m_data = ns::alloc_n<T>(m_capacity, tag);
    mem_copy(m_data, list.begin(), m_size * sizeof(T));
  }

  Vec(Vec const &other) {
    m_size = other.m_size;
    m_capacity = other.m_capacity;
    if (m_capacity) {
      m_data = ns::alloc_n<T>(m_capacity, tag);
      mem_copy(m_data, other.m_data, m_size * sizeof(T));
    }
  }

  Vec(Vec &&other) {
    m_size = other.m_size;
    m_capacity = other.m_capacity;
    m_data = other.m_data;
    other.m_data = nullptr;
    other.m_size = 0;
    other.m_capacity = 0;
  }

  ~Vec() { free(); }

  Vec operator=(Vec const &other) {
    if (this == &other)
      return *this;
    if (m_capacity < other.m_size) {
      if (m_data) {
        ns::free_n<T>(m_data, m_capacity, tag);
        m_data = nullptr;
      }
      m_capacity = other.m_size;
      m_data = ns::alloc_n<T>(m_capacity, tag);
    }
    m_size = other.m_size;
    mem_copy(m_data, other.m_data, m_size * sizeof(T));
    return *this;
  }

  Vec operator=(Vec &&other) {
    if (this == &other)
      return *this;
    if (m_data) {
      ns::free_n<T>(m_data, m_capacity, tag);
    }
    m_data = other.m_data;
    m_size = other.m_size;
    m_capacity = other.m_capacity;
    other.m_data = nullptr;
    other.m_size = 0;
    other.m_capacity = 0;
    return *this;
  }

  void free() {
    if (m_data) {
      ns::free_n<T>(m_data, m_capacity, tag);
      m_data = nullptr;
    }
    m_size = 0;
    m_capacity = 0;
  }

  operator T const *() const { return m_data; }
  operator T *() { return m_data; }

  operator Slice<T>() const { return Slice<T>::from_parts(m_data, m_size); }

  usize len() const { return m_size; }
  usize capacity() const { return m_capacity; }

  void clear() { m_size = 0; }
  void push(T c) {
    if (m_size == m_capacity) {
      reserve(m_capacity == 0 ? FIRST_CAPACITY : m_capacity * CAPACITY_MUL);
    }
    m_data[m_size++] = c;
  }
  T &&pop() { return m_data[--m_size]; }
  void reserve(usize n) {
    if (n == 0)
      return;
    if (m_capacity == 0) {
      m_capacity = n;
      m_data = ns::alloc_n<T>(m_capacity, tag);
    } else if (n > m_capacity) {
      m_data = ns::realloc_n<T>(m_data, m_capacity, n, tag);
      m_capacity = n;
    }
  }

  void resize(usize n, T c = T()) {
    reserve(n);
    m_size = n;
    for (usize i = m_size; i < n; i++) {
      m_data[i] = c;
    }
  }

  Slice<T> sub(usize start, usize end) const {
    return Slice<T>::from_parts(m_data + start, end - start);
  }
  Slice<T> sub(usize start) const {
    return Slice<T>::from_parts(m_data + start, m_size - start);
  }
  Slice<T> firsts(usize count) const {
    return Slice<T>::from_parts(m_data, count);
  }

  T const &operator[](usize index) const { return m_data[index]; }
  T &operator[](usize index) { return m_data[index]; }

  T *data() { return m_data; }
  T const *data() const { return m_data; }

  T *begin() { return m_data; }
  T const *begin() const { return m_data; }

  T *end() { return m_data + m_size; }
  T const *end() const { return m_data + m_size; }

  void erase(T const *it) {
    usize index = it - m_data;
    mem_copy(m_data + index, m_data + index + 1,
             (m_size - index - 1) * sizeof(T));
    m_size--;
  }
  void erase(usize index) {
    mem_copy(m_data + index, m_data + index + 1,
             (m_size - index - 1) * sizeof(T));
    m_size--;
  }

protected:
  T *m_data;
  usize m_size;
  usize m_capacity;
};

} // namespace ns

#endif // VEC_HEADER_INCLUDED
