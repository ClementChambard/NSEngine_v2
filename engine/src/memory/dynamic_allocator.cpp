#include "./dynamic_allocator.h"

#include "../containers/freelist.h"
#include "../core/logger.h"
#include "../core/memory.h"

namespace ns {
struct dynamic_allocator_state {
  usize total_size;
  freelist list;
  ptr freelist_block;
  ptr memory_block;
};

bool dynamic_allocator_create(usize total_size, usize *memory_requirement,
                              ptr memory, dynamic_allocator *out_allocator) {
  if (total_size < 1) {
    NS_ERROR(
        "dynamic_allocator_create - Total size cannot be 0. Create failed.");
    return false;
  }
  if (!memory_requirement) {
    NS_ERROR("dynamic_allocator_create - Memory requirement cannot be null. "
             "Create failed.");
    return false;
  }
  usize freelist_requirement = 0;
  freelist_create(total_size, &freelist_requirement, 0, 0);
  *memory_requirement =
      freelist_requirement + sizeof(dynamic_allocator_state) + total_size;

  if (!memory) {
    return true;
  }

  out_allocator->memory = memory;
  dynamic_allocator_state *state =
      reinterpret_cast<dynamic_allocator_state *>(memory);
  state->total_size = total_size;
  state->freelist_block =
      reinterpret_cast<u8 *>(memory) + sizeof(dynamic_allocator_state);
  state->memory_block =
      reinterpret_cast<u8 *>(state->freelist_block) + freelist_requirement;

  freelist_create(total_size, &freelist_requirement, state->freelist_block,
                  &state->list);

  mem_zero(state->memory_block, total_size);
  return true;
}

bool dynamic_allocator_destroy(dynamic_allocator *allocator) {
  if (allocator) {
    dynamic_allocator_state *state =
        reinterpret_cast<dynamic_allocator_state *>(allocator->memory);
    freelist_destroy(&state->list);
    mem_zero(state->memory_block, state->total_size);
    state->total_size = 0;
    allocator->memory = nullptr;
    return true;
  }
  NS_WARN("dynamic_allocator_destroy - Allocator is null. Destroy failed.");
  return false;
}

ptr dynamic_allocator_allocate(dynamic_allocator *allocator, usize size) {
  if (allocator && size) {
    dynamic_allocator_state *state =
        reinterpret_cast<dynamic_allocator_state *>(allocator->memory);
    u64 offset = 0;
    if (freelist_allocate_block(state->list, size, &offset)) {
      return reinterpret_cast<u8 *>(state->memory_block) + offset;
    } else {
      NS_ERROR("dynamic_allocator_allocate - Could not allocate %lluB. (total "
               "space available: %lluB)",
               size, freelist_free_space(state->list));
      return nullptr;
    }
  }

  NS_ERROR("dynamic_allocator_allocate - Allocator or size is null. "
           "Allocate failed.");
  return nullptr;
}

ptr dynamic_allocator_reallocate(dynamic_allocator *allocator, ptr memory,
                                 usize old_size, usize new_size) {
  if (!allocator || !memory) {
    return nullptr;
  }
  if (old_size >= new_size) {
    return memory;
  }
  dynamic_allocator_state *state =
      reinterpret_cast<dynamic_allocator_state *>(allocator->memory);
  if (memory < state->memory_block ||
      memory >
          reinterpret_cast<u8 *>(state->memory_block) + state->total_size) {
    return nullptr;
  }
  u64 offset = reinterpret_cast<u8 *>(memory) -
               reinterpret_cast<u8 *>(state->memory_block);
  u64 new_offset = 0;
  if (freelist_try_reallocate_block(state->list, old_size, new_size, offset,
                                    &new_offset)) {
    return memory;
  }
  ptr new_memory = reinterpret_cast<u8 *>(state->memory_block) + new_offset;
  mem_copy(new_memory, memory, old_size);
  freelist_free_block(state->list, old_size, offset);
  return nullptr;
}

bool dynamic_allocator_free(dynamic_allocator *allocator, ptr block,
                            usize size) {
  if (!allocator || !block || !size) {
    return false;
  }
  dynamic_allocator_state *state =
      reinterpret_cast<dynamic_allocator_state *>(allocator->memory);
  if (block < state->memory_block ||
      block > reinterpret_cast<u8 *>(state->memory_block) + state->total_size) {
    return false;
  }
  u64 offset = reinterpret_cast<u8 *>(block) -
               reinterpret_cast<u8 *>(state->memory_block);
  if (freelist_free_block(state->list, size, offset)) {
    return true;
  }
  return false;
}

usize dynamic_allocator_free_space(dynamic_allocator *allocator) {
  if (!allocator) {
    return 0;
  }
  dynamic_allocator_state *state =
      reinterpret_cast<dynamic_allocator_state *>(allocator->memory);
  return freelist_free_space(state->list);
}

} // namespace ns
