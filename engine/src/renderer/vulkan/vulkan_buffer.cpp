#include "./vulkan_buffer.h"

#include "../../core/logger.h"
#include "../../core/memory.h"
#include "./vulkan_buffer.h"
#include "./vulkan_command_buffer.h"
#include "./vulkan_utils.h"

namespace ns::vulkan {

void cleanup_freelist(Buffer *buffer) {
  freelist_destroy(&buffer->buffer_freelist);
  ns::free(buffer->freelist_block, buffer->freelist_memory_requirement,
           MemTag::RENDERER);
  buffer->freelist_block = nullptr;
  buffer->freelist_memory_requirement = 0;
}

bool buffer_create(Context *context, usize size, VkBufferUsageFlagBits usage,
                   u32 memory_property_flags, bool bind_on_create,
                   Buffer *out_buffer) {
  mem_zero(out_buffer, sizeof(Buffer));
  out_buffer->total_size = size;
  out_buffer->usage = usage;
  out_buffer->memory_property_flags = memory_property_flags;

  out_buffer->freelist_memory_requirement = 0;
  freelist_create(size, &out_buffer->freelist_memory_requirement, nullptr,
                  nullptr);
  out_buffer->freelist_block =
      ns::alloc(out_buffer->freelist_memory_requirement, MemTag::RENDERER);
  freelist_create(size, &out_buffer->freelist_memory_requirement,
                  out_buffer->freelist_block, &out_buffer->buffer_freelist);

  VkBufferCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  info.size = size;
  info.usage = usage;
  info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VK_CHECK(vkCreateBuffer(context->device, &info, context->allocator,
                          &out_buffer->handle));

  VkMemoryRequirements requirements;
  vkGetBufferMemoryRequirements(context->device, *out_buffer, &requirements);
  out_buffer->memory_index = context->find_memory_index(
      requirements.memoryTypeBits, out_buffer->memory_property_flags);
  if (out_buffer->memory_index == -1) {
    NS_ERROR("Unable to create vulkan buffer because required memory type "
             "index was not found.");
    cleanup_freelist(out_buffer);
    return false;
  }

  VkMemoryAllocateInfo mem_info{};
  mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  mem_info.allocationSize = requirements.size;
  mem_info.memoryTypeIndex = static_cast<u32>(out_buffer->memory_index);

  VkResult result = vkAllocateMemory(context->device, &mem_info,
                                     context->allocator, &out_buffer->memory);

  if (result != VK_SUCCESS) {
    NS_ERROR("Unable to create vulkan buffer because the required memory "
             "allocation failed. Error: %i",
             result);
    cleanup_freelist(out_buffer);
    return false;
  }

  if (bind_on_create) {
    buffer_bind(context, out_buffer, 0);
  }

  return true;
}

void buffer_destroy(Context *context, Buffer *buffer) {
  if (buffer->freelist_block) {
    cleanup_freelist(buffer);
  }
  if (buffer->memory) {
    vkFreeMemory(context->device, buffer->memory, context->allocator);
    buffer->memory = VK_NULL_HANDLE;
  }
  if (buffer->handle) {
    vkDestroyBuffer(context->device, *buffer, context->allocator);
    buffer->handle = VK_NULL_HANDLE;
  }
  buffer->total_size = 0;
  buffer->usage = static_cast<VkBufferUsageFlagBits>(0);
  buffer->is_locked = false;
}

bool buffer_resize(Context *context, usize new_size, Buffer *buffer,
                   VkQueue queue, VkCommandPool pool) {
  if (new_size < buffer->total_size) {
    NS_ERROR("buffer_resize - new size must be greater than the current size");
    return false;
  }

  u64 new_memory_requirement = 0;
  freelist_resize(buffer->buffer_freelist, &new_memory_requirement, nullptr,
                  new_size, nullptr);
  ptr new_block = ns::alloc(new_memory_requirement, MemTag::RENDERER);
  ptr old_block = nullptr;
  if (!freelist_resize(buffer->buffer_freelist, &new_memory_requirement,
                       new_block, new_size, &old_block)) {
    NS_ERROR("buffer_resize - failed to resize the freelist");
    ns::free(new_block, new_memory_requirement, MemTag::RENDERER);
    return false;
  }
  ns::free(old_block, buffer->freelist_memory_requirement, MemTag::RENDERER);
  buffer->freelist_memory_requirement = new_memory_requirement;
  buffer->freelist_block = new_block;
  buffer->total_size = new_size;

  VkBufferCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  info.size = new_size;
  info.usage = buffer->usage;
  info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VkBuffer new_buffer;
  VK_CHECK(
      vkCreateBuffer(context->device, &info, context->allocator, &new_buffer));

  VkMemoryRequirements requirements;
  vkGetBufferMemoryRequirements(context->device, new_buffer, &requirements);

  VkMemoryAllocateInfo mem_info{};
  mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  mem_info.allocationSize = requirements.size;
  mem_info.memoryTypeIndex = static_cast<u32>(buffer->memory_index);

  VkDeviceMemory new_memory;
  VkResult result = vkAllocateMemory(context->device, &mem_info,
                                     context->allocator, &new_memory);

  if (result != VK_SUCCESS) {
    NS_ERROR("Unable to create vulkan buffer because the required memory "
             "allocation failed. Error: %i",
             result);
    return false;
  }

  VK_CHECK(vkBindBufferMemory(context->device, new_buffer, new_memory, 0));

  buffer_copy_to(context, pool, 0, queue, *buffer, 0, new_buffer, 0,
                 buffer->total_size);

  vkDeviceWaitIdle(context->device);

  if (buffer->memory) {
    vkFreeMemory(context->device, buffer->memory, context->allocator);
    buffer->memory = VK_NULL_HANDLE;
  }
  if (buffer->handle) {
    vkDestroyBuffer(context->device, *buffer, context->allocator);
    buffer->handle = VK_NULL_HANDLE;
  }

  buffer->total_size = new_size;
  buffer->memory = new_memory;
  buffer->handle = new_buffer;

  return true;
}

void buffer_bind(Context *context, Buffer *buffer, usize offset) {
  VK_CHECK(
      vkBindBufferMemory(context->device, *buffer, buffer->memory, offset));
}

ptr buffer_lock_memory(Context *context, Buffer *buffer, usize offset,
                       usize size, u32 flags) {
  ptr data;
  VK_CHECK(
      vkMapMemory(context->device, buffer->memory, offset, size, flags, &data));
  return data;
}

void buffer_unlock_memory(Context *context, Buffer *buffer) {
  vkUnmapMemory(context->device, buffer->memory);
}

bool buffer_allocate(Buffer *buffer, u64 size, u64 *out_offset) {
  if (!buffer || !size || !out_offset) {
    NS_ERROR("buffer_allocate - invalid parameters");
    return false;
  }

  return freelist_allocate_block(buffer->buffer_freelist, size, out_offset);
}

bool buffer_free(Buffer *buffer, u64 size, u64 offset) {
  if (!buffer || !size) {
    NS_ERROR("buffer_free - invalid parameters");
    return false;
  }

  return freelist_free_block(buffer->buffer_freelist, size, offset);
}

void buffer_load_data(Context *context, Buffer *buffer, usize offset,
                      usize size, u32 flags, roptr data) {
  ptr data_ptr;
  VK_CHECK(vkMapMemory(context->device, buffer->memory, offset, size, flags,
                       &data_ptr));
  mem_copy(data_ptr, data, size);
  vkUnmapMemory(context->device, buffer->memory);
}

void buffer_copy_to(Context *context, VkCommandPool pool, VkFence /* fence */,
                    VkQueue queue, VkBuffer source, usize source_offset,
                    VkBuffer dest, usize dest_offset, usize size) {
  vkQueueWaitIdle(queue);
  CommandBuffer tmp_cmdbuf;
  command_buffer_allocate_and_begin_single_use(context, pool, &tmp_cmdbuf);
  VkBufferCopy copy_region;
  copy_region.srcOffset = source_offset;
  copy_region.dstOffset = dest_offset;
  copy_region.size = size;

  vkCmdCopyBuffer(tmp_cmdbuf, source, dest, 1, &copy_region);

  command_buffer_end_single_use(context, pool, &tmp_cmdbuf, queue);
}

} // namespace ns::vulkan
