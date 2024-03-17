#ifndef DYNAMIC_ALLOCATOR_HEADER_INCLUDED
#define DYNAMIC_ALLOCATOR_HEADER_INCLUDED

#include "../defines.h"

namespace ns {

struct dynamic_allocator {
  ptr memory;
};

NS_API bool dynamic_allocator_create(usize total_size,
                                     usize *memory_requirement, ptr memory,
                                     dynamic_allocator *out_allocator);

NS_API bool dynamic_allocator_destroy(dynamic_allocator *allocator);

NS_API ptr dynamic_allocator_allocate(dynamic_allocator *allocator, usize size);

NS_API ptr dynamic_allocator_reallocate(dynamic_allocator *allocator,
                                        ptr memory, usize old_size,
                                        usize new_size);

NS_API bool dynamic_allocator_free(dynamic_allocator *allocator, ptr memory,
                                   usize size);

NS_API usize dynamic_allocator_free_space(dynamic_allocator *allocator);

} // namespace ns

#endif // DYNAMIC_ALLOCATOR_HEADER_INCLUDED
