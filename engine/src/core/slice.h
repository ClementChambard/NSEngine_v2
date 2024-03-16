/**
 * @file slice.h
 * @brief Contains the Slice class.
 * @author Clement Chambard
 * @date 2024
 */

#ifndef SLICE_HEADER_INCLUDED
#define SLICE_HEADER_INCLUDED

#include "../defines.h"

namespace ns {

/** @class Slice
 * @brief A slice is a view for array data.
 * @tparam T the type of the array.
 */
template <typename T> struct Slice {
  NS_API Slice(Slice const &) = default;

  /**
   * @brief Creates a slice from a C array.
   * @param data the C array.
   * @param count the number of elements.
   */
  NS_API static Slice from_parts(T const *data, usize count) {
    Slice slice;
    slice.m_data = data;
    slice.m_count = count;
    return slice;
  }

  /**
   * @brief Get a sub slice from inside the current slice.
   * @param start the start index of the sub slice.
   *        There is no bound checking on this value.
   * @param end the end index of the sub slice
   *        There is no bound checking on this value.
   * @return the slice.
   */
  NS_API Slice sub(usize start, usize end) {
    Slice slice;
    slice.m_data = m_data + start;
    slice.m_count = end - start;
    return slice;
  }

  /**
   * @brief Get a sub slice until the end of the current slice.
   * @param start the start index of the sub slice.
   *        There is no bound checking on this value.
   * @return the slice.
   */
  NS_API Slice sub(usize start) {
    Slice slice;
    slice.m_data = m_data + start;
    slice.m_count = m_count - start;
    return slice;
  }

  /**
   * @brief Get a sub slice from the start of the current slice.
   * @param count the number of elements in the slice.
   *        There is no bound checking on this value.
   * @return the slice.
   */
  NS_API Slice firsts(usize count) {
    Slice slice;
    slice.m_data = m_data;
    slice.m_count = count;
    return slice;
  }

  /**
   * @brief Get the element at the given index.
   * @param index the index of the element.
   *        There is no bound checking on this value.
   */
  NS_API T const &operator[](usize index) const { return m_data[index]; }

  /**
   * @brief Convertion to a C array.
   */
  NS_API operator T const *() const { return m_data; }

  /**
   * @brief Get the begin iterator of the slice.
   * @return the begin iterator of the slice.
   */
  NS_API T const *begin() const { return m_data; }

  /**
   * @brief Get the end iterator of the slice.
   * @return the end iterator of the slice.
   */
  NS_API T const *end() const { return m_data + m_count; }

  /**
   * @brief Comparison operator for slices.
   * @param other the other slice.
   *
   * @return true if the slices are equal:
   *   - the count is the same
   *   - the values are the same
   */
  NS_API bool operator==(Slice const &other) const {
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

  /**
   * @brief Comparison operator for slices.
   * @param other the other slice.
   *
   * @return true if the slices are not equal:
   *   - the count is the same
   *   - the values are the same
   */
  NS_API bool operator!=(Slice const &other) const { return !(*this == other); }

  /**
   * @brief Get the number of elements in the slice.
   * @return the number of elements in the slice.
   */
  NS_API usize len() const { return m_count; }

protected:
  T const *m_data;
  usize m_count;

private:
  Slice() {}
};

} // namespace ns

#endif // SLICE_HEADER_INCLUDED
