#include "./vulkan_buffer.h"

#include "../../core/logger.h"
#include "../../core/ns_memory.h"
#include "./vulkan_buffer.h"
#include "./vulkan_command_buffer.h"
#include "./vulkan_utils.h"

namespace ns::vulkan {

bool buffer_create(Context *context, usize size, VkBufferUsageFlagBits usage,
                   u32 memory_property_flags, bool bind_on_create,
                   Buffer *out_buffer) {
  mem_zero(out_buffer, sizeof(Buffer));
  out_buffer->total_size = size;
  out_buffer->usage = usage;
  out_buffer->memory_property_flags = memory_property_flags;

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
    return false;
  }

  if (bind_on_create) {
    buffer_bind(context, out_buffer, 0);
  }

  return true;
}

void buffer_destroy(Context *context, Buffer *buffer) {
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
