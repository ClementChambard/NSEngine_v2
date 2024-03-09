#ifndef VULKAN_COMMAND_BUFFER_HEADER_INCLUDED
#define VULKAN_COMMAND_BUFFER_HEADER_INCLUDED

#include "./vulkan_types.inl"

namespace ns::vulkan {

void command_buffer_allocate(Context *context, VkCommandPool pool,
                             bool is_primary,
                             CommandBuffer *out_command_buffer);

void command_buffer_free(Context *context, VkCommandPool pool,
                         CommandBuffer *command_buffer);

void command_buffer_begin(CommandBuffer *command_buffer, bool is_single_use,
                          bool is_renderpass_continue,
                          bool is_simultaneous_use);

void command_buffer_end(CommandBuffer *command_buffer);

void command_buffer_update_submitted(CommandBuffer *command_buffer);

void command_buffer_reset(CommandBuffer *command_buffer);

void command_buffer_allocate_and_begin_single_use(
    Context *context, VkCommandPool pool, CommandBuffer *out_command_buffer);

void command_buffer_end_single_use(Context *context, VkCommandPool pool,
                                   CommandBuffer *command_buffer,
                                   VkQueue queue);

} // namespace ns::vulkan

#endif // VULKAN_COMMAND_BUFFER_HEADER_INCLUDED
