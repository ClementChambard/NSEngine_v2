#ifndef LINEAR_ALLOCATOR_HEADER_INCLUDED
#define LINEAR_ALLOCATOR_HEADER_INCLUDED

#include "../defines.h"

struct linear_allocator {
  usize total_size;
  usize allocated;
  ptr memory;
  bool owns_memory;

  NS_API linear_allocator(usize total_size, ptr memory);
  NS_API ~linear_allocator();

  NS_API ptr allocate(usize size);
  NS_API void free_all();

  template <typename T> NS_API T *allocate_n(u64 count);
};

#endif // LINEAR_ALLOCATOR_HEADER_INCLUDED
