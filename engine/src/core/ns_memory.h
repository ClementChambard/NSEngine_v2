#ifndef NS_MEMORY_HEADER_INCLUDED
#define NS_MEMORY_HEADER_INCLUDED

#include "../defines.h"

namespace ns {

enum class mem_tag {
  UNKNOWN,
  LINEAR_ALLOCATOR,
  ARRAY,
  VECTOR,
  DICT,
  RING_QUEUE,
  BST,
  STRING,
  APPLICATION,
  JOB,
  TEXTURE,
  MATERIAL_INSTANCE,
  RENDERER,
  GAME,
  TRANSFORM,
  ENTITY,
  ENTITY_NODE,
  SCENE,

  MAX_TAGS
};

NS_API void memory_system_initialize(usize *memory_requirement, ptr state);

NS_API void memory_system_shutdown(ptr state);

/**
 * Allocate a block of memory
 * @param size the size of the block
 * @param tag the tag of the block
 */
NS_API ptr alloc(usize size, mem_tag tag);

/**
 * Free a block of memory
 * @param block the block to free
 * @param size the size of the block
 * @param tag the tag of the block
 */
NS_API void free(ptr block, usize size, mem_tag tag);

/**
 * Zero a block of memory
 * @param block the block to zero
 * @param size the size of the block
 */
NS_API ptr mem_zero(ptr block, usize size);

/**
 * Copy a block of memory
 * @param dest the destination block
 * @param source the source block
 * @param size the size of the block
 */
NS_API ptr mem_copy(ptr dest, roptr source, usize size);

/**
 * Set a block of memory
 * @param dest the destination block
 * @param value the value to set
 * @param size the size of the block
 */
NS_API ptr mem_set(ptr dest, i32 value, usize size);

/**
 * Get the memory usage string
 * @returns the memory usage string
 */
NS_API str get_memory_usage_str();

/**
 * Get the number of allocations
 * @returns the number of allocations
 */
NS_API u64 get_memory_alloc_count();

} // namespace ns

#endif // NS_MEMORY_HEADER_INCLUDED
