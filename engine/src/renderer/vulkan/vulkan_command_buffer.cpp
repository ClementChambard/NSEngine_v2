#include "./vulkan_command_buffer.h"

#include "../../core/ns_memory.h"

namespace ns::vulkan {

void command_buffer_allocate(Context *context, VkCommandPool pool,
                             bool is_primary,
                             CommandBuffer *out_command_buffer) {
  mem_zero(out_command_buffer, sizeof(CommandBuffer));
  out_command_buffer->state = CommandBuffer::State::NOT_ALLOCATED;

  VkCommandBufferAllocateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  info.commandPool = pool;
  info.level = is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY
                          : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
  info.commandBufferCount = 1;
  info.pNext = nullptr;

  VK_CHECK(vkAllocateCommandBuffers(context->device, &info,
                                    &out_command_buffer->handle));
  out_command_buffer->state = CommandBuffer::State::READY;
}

void command_buffer_free(Context *context, VkCommandPool pool,
                         CommandBuffer *command_buffer) {
  vkFreeCommandBuffers(context->device, pool, 1, &command_buffer->handle);

  command_buffer->handle = VK_NULL_HANDLE;
  command_buffer->state = CommandBuffer::State::NOT_ALLOCATED;
}

void command_buffer_begin(CommandBuffer *command_buffer, bool is_single_use,
                          bool is_renderpass_continue,
                          bool is_simultaneous_use) {
  VkCommandBufferBeginInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  info.flags = 0;
  if (is_single_use) {
    info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  }
  if (is_renderpass_continue) {
    info.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
  }
  if (is_simultaneous_use) {
    info.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
  }

  VK_CHECK(vkBeginCommandBuffer(*command_buffer, &info));
  command_buffer->state = CommandBuffer::State::RECORDING;
}

void command_buffer_end(CommandBuffer *command_buffer) {
  VK_CHECK(vkEndCommandBuffer(*command_buffer));
  command_buffer->state = CommandBuffer::State::RECORDING_ENDED;
}

void command_buffer_update_submitted(CommandBuffer *command_buffer) {
  command_buffer->state = CommandBuffer::State::SUBMITTED;
}

void command_buffer_reset(CommandBuffer *command_buffer) {
  command_buffer->state = CommandBuffer::State::READY;
}

void command_buffer_allocate_and_begin_single_use(
    Context *context, VkCommandPool pool, CommandBuffer *out_command_buffer) {
  command_buffer_allocate(context, pool, true, out_command_buffer);
  command_buffer_begin(out_command_buffer, true, false, false);
}

void command_buffer_end_single_use(Context *context, VkCommandPool pool,
                                   CommandBuffer *command_buffer,
                                   VkQueue queue) {
  command_buffer_end(command_buffer);

  VkSubmitInfo info{};
  info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  info.commandBufferCount = 1;
  info.pCommandBuffers = &command_buffer->handle;
  VK_CHECK(vkQueueSubmit(queue, 1, &info, 0));

  VK_CHECK(vkQueueWaitIdle(queue));

  command_buffer_free(context, pool, command_buffer);
}

} // namespace ns::vulkan
