#ifndef NS_MEMORY_HEADER_INCLUDED
#define NS_MEMORY_HEADER_INCLUDED

#include "../defines.h"

namespace ns {

enum class MemTag {
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
NS_API ptr alloc(usize size, MemTag tag = MemTag::UNKNOWN);

/**
 * Allocate an array of a type
 * @param count the count of elements in the block
 * @param tag the tag of the block
 */
template <typename T>
NS_API T *alloc_n(usize count, MemTag tag = MemTag::UNKNOWN) {
  return reinterpret_cast<T *>(ns::alloc(count * sizeof(T), tag));
}

/**
 * Reallocate a block of memory
 * @param block the block to reallocate
 * @param prev_size the previous size of the block
 * @param new_size the new size of the block
 * @param tag the tag of the block
 */
NS_API ptr realloc(ptr block, usize prev_size, usize new_size,
                   MemTag tag = MemTag::UNKNOWN);

/**
 * Reallocate an array of a type
 * @param block the block to reallocate
 * @param prev_size the previous size of the block
 * @param new_size the new size of the block
 * @param tag the tag of the block
 */
template <typename T>
NS_API T *realloc_n(T *block, usize prev_count, usize new_count,
                    MemTag tag = MemTag::UNKNOWN) {
  return reinterpret_cast<T *>(
      ns::realloc(block, prev_count * sizeof(T), new_count * sizeof(T), tag));
}

/**
 * Free a block of memory
 * @param block the block to free
 * @param size the size of the block
 * @param tag the tag of the block
 */
NS_API void free(ptr block, usize size, MemTag tag = MemTag::UNKNOWN);

/**
 * Free an array of a type
 * @param block the block to free
 * @param count the count of elements in the block
 * @param tag the tag of the block
 */
template <typename T>
NS_API void free_n(T *block, usize count, MemTag tag = MemTag::UNKNOWN) {
  ns::free(block, count * sizeof(T), tag);
}

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
NS_API pstr get_memory_usage_str();

/**
 * Get the number of allocations
 * @returns the number of allocations
 */
NS_API u64 get_memory_alloc_count();

} // namespace ns

#endif // NS_MEMORY_HEADER_INCLUDED
