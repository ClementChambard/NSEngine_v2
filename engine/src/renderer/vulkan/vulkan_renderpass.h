#ifndef VULKAN_RENDERPASS_HEADER_INCLUDED
#define VULKAN_RENDERPASS_HEADER_INCLUDED

#include "./vulkan_types.inl"

namespace ns::vulkan {

void renderpass_create(Context *context, Renderpass *out_renderpass,
                       vec4 render_area, vec4 color, f32 depth, u32 stencil,
                       ClearFlags clear_flags, bool has_prev_pass,
                       bool has_next_pass);

void renderpass_destroy(Context *context, Renderpass *renderpass);

void renderpass_begin(CommandBuffer *command_buffer, Renderpass *renderpass,
                      VkFramebuffer frame_buffer);

void renderpass_end(CommandBuffer *command_buffer, Renderpass *renderpass);

} // namespace ns::vulkan

#endif // VULKAN_RENDERPASS_HEADER_INCLUDED
