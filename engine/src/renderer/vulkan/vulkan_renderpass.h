#ifndef VULKAN_RENDERPASS_HEADER_INCLUDED
#define VULKAN_RENDERPASS_HEADER_INCLUDED

#include "./vulkan_types.inl"

namespace ns::vulkan {

void renderpass_create(Context *context, Renderpass *out_renderpass, f32 x,
                       f32 y, f32 w, f32 h, f32 r, f32 g, f32 b, f32 a,
                       f32 depth, u32 stencil);

void renderpass_destroy(Context *context, Renderpass *renderpass);

void renderpass_begin(CommandBuffer *command_buffer, Renderpass *renderpass,
                      VkFramebuffer frame_buffer);

void renderpass_end(CommandBuffer *command_buffer, Renderpass *renderpass);

} // namespace ns::vulkan

#endif // VULKAN_RENDERPASS_HEADER_INCLUDED
