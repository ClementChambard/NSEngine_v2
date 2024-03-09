#include "./linear_allocator.h"

#include "../core/logger.h"
#include "../core/ns_memory.h"

linear_allocator::linear_allocator(usize total_size, ptr memory)
    : total_size(total_size), allocated(0) {
  owns_memory = memory == nullptr;
  if (memory) {
    this->memory = memory;
  } else {
    this->memory = ns::alloc(total_size, ns::mem_tag::LINEAR_ALLOCATOR);
  }
}

linear_allocator::~linear_allocator() {
  if (owns_memory && memory) {
    ns::free(memory, total_size, ns::mem_tag::LINEAR_ALLOCATOR);
  }
  memory = nullptr;
  total_size = 0;
  allocated = 0;
  owns_memory = false;
}

ptr linear_allocator::allocate(usize size) {
  if (!memory) {
    NS_ERROR("linear_allocator::allocate - Allocator not initialized.");
    return nullptr;
  }
  if (allocated + size > total_size) {
    u64 remaining = total_size - allocated;
    NS_ERROR("linear_allocator::allocate - Tried to allocate %lluB, only "
             "%lluB remaining.",
             size, remaining);
    return nullptr;
  }

  ptr block = reinterpret_cast<u8 *>(memory) + allocated;
  allocated += size;
  return block;
}

template <typename T> T *linear_allocator::allocate_n(u64 count) {
  usize size = count * sizeof(T);
  return reinterpret_cast<T *>(allocate(size));
}

void linear_allocator::free_all() {
  if (memory) {
    allocated = 0;
    ns::mem_zero(memory, total_size);
  }
}
