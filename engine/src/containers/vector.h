#ifndef VEC_HEADER_INCLUDED
#define VEC_HEADER_INCLUDED

#include "../core/logger.h"
#include "../core/ns_memory.h"
#include <vector>

// #define VECTOR_TRACE

namespace ns {

template <typename T> class VectorAllocator {
public:
  using value_type = T;
  using size_type = usize;

  VectorAllocator() noexcept {}

  template <class U> VectorAllocator(VectorAllocator<U> const &) noexcept {}

  T *allocate(usize n) {
#ifdef VECTOR_TRACE
    NS_TRACE("Allocating vector: %d", n * sizeof(T));
#endif
    return static_cast<T *>(ns::alloc(n * sizeof(T), mem_tag::VECTOR));
  }

  void deallocate(T *p, usize n) noexcept {
#ifdef VECTOR_TRACE
    NS_TRACE("Deallocating vector: %d", n * sizeof(T));
#endif
    ns::free(p, n * sizeof(T), mem_tag::VECTOR);
  }

  template <class U> bool operator==(VectorAllocator<U> const &) {
    return false;
  }

  bool operator==(VectorAllocator<T> const &) { return true; }
};

template <typename T> using vector = std::vector<T, VectorAllocator<T>>;

} // namespace ns

#endif // VEC_HEADER_INCLUDED
