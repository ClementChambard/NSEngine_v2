#ifndef FREELIST_HEADER_INCLUDED
#define FREELIST_HEADER_INCLUDED

#include "../defines.h"

namespace ns {

struct freelist {
  void *memory;
};

NS_API void freelist_create(u32 total_size, u64 *memory_requirement,
                            void *memory, freelist *out_list);

NS_API void freelist_destroy(freelist *list);

NS_API bool freelist_allocate_block(freelist list, u32 size, u32 *out_offset);

NS_API bool freelist_free_block(freelist list, u32 size, u32 offset);

NS_API void freelist_clear(freelist list);

NS_API usize freelist_free_space(freelist list);

}; // namespace ns

#endif // FREELIST_HEADER_INCLUDED
