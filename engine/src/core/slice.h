#ifndef SLICE_HEADER_INCLUDED
#define SLICE_HEADER_INCLUDED

#include "../defines.h"

namespace ns {

/**
 * A slice is a view of an array of T
 */
template <typename T> struct Slice {
  Slice(Slice const &) = default;
  static Slice from_parts(T const *data, usize count) {
    Slice slice;
    slice.m_data = data;
    slice.m_count = count;
    return slice;
  }

  Slice sub(usize start, usize end) {
    Slice slice;
    slice.m_data = m_data + start;
    slice.m_count = end - start;
    return slice;
  }

  Slice sub(usize start) {
    Slice slice;
    slice.m_data = m_data + start;
    slice.m_count = m_count - start;
    return slice;
  }

  Slice firsts(usize count) {
    Slice slice;
    slice.m_data = m_data;
    slice.m_count = count;
    return slice;
  }

  T const &operator[](usize index) const { return m_data[index]; }

  operator T const *() const { return m_data; }

  T const *begin() const { return m_data; }
  T const *end() const { return m_data + m_count; }

  bool operator==(Slice const &other) const {
    if (m_count != other.m_count) {
      return false;
    }
    if (m_data == other.m_data)
      return true;
    if (m_data == nullptr || other.m_data == nullptr)
      return false;
    for (usize i = 0; i < m_count; i++) {
      if (m_data[i] != other.m_data[i]) {
        return false;
      }
    }
    return true;
  }

  bool operator!=(Slice const &other) const { return !(*this == other); }

  usize len() const { return m_count; }

protected:
  T const *m_data;
  usize m_count;

private:
  Slice() {}
};

} // namespace ns

#endif // SLICE_HEADER_INCLUDED
