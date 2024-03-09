#include "./vulkan_framebuffer.h"

#include "../../core/ns_memory.h"

namespace ns::vulkan {

void framebuffer_create(Context *context, Renderpass *renderpass, u32 width,
                        u32 height, u32 attachment_count,
                        VkImageView *attachments,
                        Framebuffer *out_framebuffer) {
  out_framebuffer->attachments = reinterpret_cast<VkImageView *>(
      ns::alloc(sizeof(VkImageView) * attachment_count, mem_tag::RENDERER));
  for (u32 i = 0; i < attachment_count; i++) {
    out_framebuffer->attachments[i] = attachments[i];
  }
  out_framebuffer->renderpass = renderpass;
  out_framebuffer->attachment_count = attachment_count;

  VkFramebufferCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  info.renderPass = *renderpass;
  info.attachmentCount = attachment_count;
  info.pAttachments = out_framebuffer->attachments;
  info.width = width;
  info.height = height;
  info.layers = 1;

  VK_CHECK(vkCreateFramebuffer(context->device, &info, context->allocator,
                               &out_framebuffer->handle));
}

void framebuffer_destroy(Context *context, Framebuffer *framebuffer) {
  vkDestroyFramebuffer(context->device, *framebuffer, context->allocator);
  if (framebuffer->attachments) {
    ns::free(framebuffer->attachments,
             sizeof(VkImageView) * framebuffer->attachment_count,
             mem_tag::RENDERER);
    framebuffer->attachments = nullptr;
  }
  framebuffer->handle = VK_NULL_HANDLE;
  framebuffer->attachment_count = 0;
  framebuffer->renderpass = nullptr;
}

} // namespace ns::vulkan
