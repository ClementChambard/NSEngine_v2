/** @file vec.h
 * @brief This file contains the Vec class which is an implementation of a
 * dynamic array type.
 * @author Clement Chambard
 * @date 2024
 */

#ifndef VEC_HEADER_INCLUDED
#define VEC_HEADER_INCLUDED

#include "../core/memory.h"
#include "../core/slice.h"

#include <initializer_list>

namespace ns {

usize string_length(cstr s);

/** @class Vec
 * @brief A dynamic array implementation.
 * @tparam T the type of the array.
 * @tparam tag the memory tag to use for the array allocations.
 */
template <typename T, MemTag tag = MemTag::VECTOR> class Vec {
public:
  /**
   * @brief Default constructor.
   */
  NS_API Vec() = default;

  /**
   * @brief Creates a vector with the specified size and fill it with the
   * specified value.
   * @param size the size of the vector.
   * @param c the value to fill the vector with.
   */
  NS_API explicit Vec(usize size, T c = T()) {
    m_size = size;
    m_capacity = size;
    m_data = ns::alloc_n<T>(m_capacity, tag);
    for (usize i = 0; i < size; i++) {
      m_data[i] = c;
    }
  }

  /**
   * @brief Creates a vector from a slice.
   * @param s the slice.
   *
   * The contents of the slice is copied into the vector.
   */
  NS_API explicit Vec(Slice<T> s) {
    m_size = s.m_count;
    m_capacity = s.m_count;
    m_data = ns::alloc_n<T>(m_capacity, tag);
    mem_copy(m_data, s.m_data, m_size * sizeof(T));
  }

  /**
   * @brief Creates a vector from a C array.
   * @param data the C array.
   * @param count the number of elements.
   */
  NS_API static Vec from_parts(T const *data, usize count) {
    Vec vec;
    vec.m_data = data;
    vec.m_size = count;
    vec.m_capacity = count;
    return vec;
  }

  /**
   * @brief Creates a vector from an initializer list.
   * @param list the initializer list.
   */
  NS_API Vec(std::initializer_list<T> list) {
    m_size = list.size();
    m_capacity = m_size;
    m_data = ns::alloc_n<T>(m_capacity, tag);
    mem_copy(m_data, list.begin(), m_size * sizeof(T));
  }

  /**
   * @brief Copy constructor.
   * @param other the vector to copy.
   */
  NS_API Vec(Vec const &other) {
    m_size = other.m_size;
    m_capacity = other.m_capacity;
    if (m_capacity) {
      m_data = ns::alloc_n<T>(m_capacity, tag);
      mem_copy(m_data, other.m_data, m_size * sizeof(T));
    }
  }

  /**
   * @brief Move constructor.
   * @param other the vector to move.
   */
  NS_API Vec(Vec &&other) {
    m_size = other.m_size;
    m_capacity = other.m_capacity;
    m_data = other.m_data;
    other.m_data = nullptr;
    other.m_size = 0;
    other.m_capacity = 0;
  }

  /**
   * @brief Destructor.
   */
  NS_API ~Vec() { free(); }

  /**
   * @brief Copy assignment.
   * @param other the vector to copy.
   */
  NS_API Vec operator=(Vec const &other) {
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

  /**
   * @brief Move assignment.
   * @param other the vector to move.
   */
  NS_API Vec operator=(Vec &&other) {
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

  /**
   * @brief Frees the memory used by the vector.
   */
  NS_API void free() {
    if (m_data) {
      ns::free_n<T>(m_data, m_capacity, tag);
      m_data = nullptr;
    }
    m_size = 0;
    m_capacity = 0;
  }

  /**
   * @brief conversion operator for a C array.
   * @return the C array.
   *         (the pointer is only valid as long as the vector is not modified)
   */
  NS_API operator T const *() const { return m_data; }

  /**
   * @brief conversion operator for a C array.
   * @return the C array.
   *         (the pointer is only valid as long as the vector is not modified)
   */
  NS_API operator T *() { return m_data; }

  /**
   * @brief conversion operator for a slice.
   * @return the slice.
   *         (the slice is only valid as long as the vector is not modified)
   */
  NS_API operator Slice<T>() const {
    return Slice<T>::from_parts(m_data, m_size);
  }

  /**
   * @brief Get the number of elements in the vector.
   * @return the number of elements in the vector.
   */
  NS_API usize len() const { return m_size; }

  /**
   * @brief Check if the vector is empty.
   * @return true if the vector is empty, false otherwise.
   */
  NS_API bool is_empty() const { return m_size == 0; }

  /**
   * @brief Get the capacity of the vector.
   * @return the capacity of the vector.
   */
  NS_API usize capacity() const { return m_capacity; }

  /**
   * @brief Clear the vector.
   */
  NS_API void clear() { m_size = 0; }

  /**
   * @brief Pushes an element to the vector.
   * @param c the element to push.
   */
  NS_API void push(T c) {
    if (m_size == m_capacity) {
      reserve(m_capacity == 0 ? FIRST_CAPACITY : m_capacity * CAPACITY_MUL);
    }
    m_data[m_size++] = c;
  }

  /**
   * @brief Pops an element from the vector.
   * @return the element that was popped.
   */
  NS_API T &&pop() { return m_data[--m_size]; }

  /**
   * @brief Reserves memory for the vector.
   * @param n the number of elements to reserve.
   */
  NS_API void reserve(usize n) {
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

  /**
   * @brief Resizes the vector.
   * @param n the new size of the vector.
   * @param c the value to fill the new elements with.
   */
  NS_API void resize(usize n, T c = T()) {
    reserve(n);
    m_size = n;
    for (usize i = m_size; i < n; i++) {
      m_data[i] = c;
    }
  }

  /**
   * @brief Get a slice from inside the current vector.
   * @param start the start index of the slice.
   *        There is no bound checking on this value.
   * @param end the end index of the slice
   *        There is no bound checking on this value.
   * @return the slice.
   */
  NS_API Slice<T> sub(usize start, usize end) const {
    return Slice<T>::from_parts(m_data + start, end - start);
  }

  /**
   * @brief Get a slice until the end of the current vector.
   * @param start the start index of the slice.
   *        There is no bound checking on this value.
   * @return the slice.
   */
  NS_API Slice<T> sub(usize start) const {
    return Slice<T>::from_parts(m_data + start, m_size - start);
  }

  /**
   * @brief Get a slice from the start of the current vector.
   * @param count the number of elements in the slice.
   *        There is no bound checking on this value.
   * @return the slice.
   */
  NS_API Slice<T> firsts(usize count) const {
    return Slice<T>::from_parts(m_data, count);
  }

  /**
   * @brief get an element from the vector.
   * @param index the index of the element.
   *        There is no bound checking on this value.
   * @return the element.
   */
  NS_API T const &operator[](usize index) const { return m_data[index]; }

  /**
   * @brief get an element from the vector.
   * @param index the index of the element.
   *        There is no bound checking on this value.
   * @return the element.
   */
  NS_API T &operator[](usize index) { return m_data[index]; }

  /**
   * @brief Get the begin iterator of the vector.
   * @return the begin iterator of the vector.
   */
  NS_API T *begin() { return m_data; }

  /**
   * @brief Get the const begin iterator of the vector.
   * @return the const begin iterator of the vector.
   */
  NS_API T const *begin() const { return m_data; }

  /**
   * @brief Get the end iterator of the vector.
   * @return the end iterator of the vector.
   */
  NS_API T *end() { return m_data + m_size; }

  /**
   * @brief Get the const end iterator of the vector.
   * @return the const end iterator of the vector.
   */
  NS_API T const *end() const { return m_data + m_size; }

  /**
   * @brief Erase an element from the vector.
   * @param it the iterator of the element to erase.
   */
  NS_API void erase(T const *it) {
    usize index = it - m_data;
    mem_copy(m_data + index, m_data + index + 1,
             (m_size - index - 1) * sizeof(T));
    m_size--;
  }

  /**
   * @brief Erase an element from the vector.
   * @param index the index of the element to erase.
   */
  NS_API void erase(usize index) {
    mem_copy(m_data + index, m_data + index + 1,
             (m_size - index - 1) * sizeof(T));
    m_size--;
  }

protected:
  T *m_data;
  usize m_size;
  usize m_capacity;

  static constexpr usize FIRST_CAPACITY = 4;
  static constexpr usize CAPACITY_MUL = 2;
};

} // namespace ns

#endif // VEC_HEADER_INCLUDED
