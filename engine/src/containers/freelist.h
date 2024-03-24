#ifndef FREELIST_HEADER_INCLUDED
#define FREELIST_HEADER_INCLUDED

#include "../defines.h"

namespace ns {

struct freelist {
  void *memory;
};

NS_API void freelist_create(u32 total_size, usize *memory_requirement,
                            void *memory, freelist *out_list);

NS_API void freelist_destroy(freelist *list);

NS_API bool freelist_allocate_block(freelist list, usize size, u64 *out_offset);

NS_API bool freelist_try_reallocate_block(freelist list, usize old_size,
                                          usize new_size, u64 offset,
                                          u64 *new_offset);

NS_API bool freelist_free_block(freelist list, usize size, u64 offset);

NS_API bool freelist_resize(freelist list, usize *memory_requirement,
                            ptr new_memory, usize new_size,
                            ptr *out_old_memory);

NS_API void freelist_clear(freelist list);

NS_API usize freelist_free_space(freelist list);

}; // namespace ns

#endif // FREELIST_HEADER_INCLUDED
