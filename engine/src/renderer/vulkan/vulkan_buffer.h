#ifndef VULKAN_BUFFER_HEADER_INCLUDED
#define VULKAN_BUFFER_HEADER_INCLUDED

#include "./vulkan_types.inl"

namespace ns::vulkan {

bool buffer_create(Context *context, usize size, VkBufferUsageFlagBits usage,
                   u32 memory_property_flags, bool bind_on_create,
                   Buffer *out_buffer);

void buffer_destroy(Context *context, Buffer *buffer);

bool buffer_resize(Context *context, usize new_size, Buffer *buffer,
                   VkQueue queue, VkCommandPool pool);

void buffer_bind(Context *context, Buffer *buffer, usize offset);

ptr buffer_lock_memory(Context *context, Buffer *buffer, usize offset,
                       usize size, u32 flags);
void buffer_unlock_memory(Context *context, Buffer *buffer);

bool buffer_allocate(Buffer *buffer, u64 size, u64 *out_offset);

bool buffer_free(Buffer *buffer, u64 size, u64 offset);

void buffer_load_data(Context *context, Buffer *buffer, usize offset,
                      usize size, u32 flags, roptr data);

void buffer_copy_to(Context *context, VkCommandPool pool, VkFence fence,
                    VkQueue queue, VkBuffer source, usize source_offset,
                    VkBuffer dest, usize dest_offset, usize size);

} // namespace ns::vulkan

#endif // VULKAN_BUFFER_HEADER_INCLUDED
