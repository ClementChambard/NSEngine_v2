#ifndef VULKAN_FRAMEBUFFER_HEADER_INCLUDED
#define VULKAN_FRAMEBUFFER_HEADER_INCLUDED

#include "./vulkan_types.inl"

namespace ns::vulkan {

void framebuffer_create(Context *context, Renderpass *renderpass, u32 width,
                        u32 height, u32 attachment_count,
                        VkImageView *attachments, Framebuffer *out_framebuffer);

void framebuffer_destroy(Context *context, Framebuffer *framebuffer);

} // namespace ns::vulkan

#endif // VULKAN_FRAMEBUFFER_HEADER_INCLUDED
