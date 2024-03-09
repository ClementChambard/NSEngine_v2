#include "./vulkan_renderpass.h"

#include "../../core/ns_memory.h"

namespace ns::vulkan {

void renderpass_create(Context *context, Renderpass *out_renderpass, f32 x,
                       f32 y, f32 w, f32 h, f32 r, f32 g, f32 b, f32 a,
                       f32 depth, u32 stencil) {
  out_renderpass->x = x;
  out_renderpass->y = y;
  out_renderpass->w = w;
  out_renderpass->h = h;
  out_renderpass->r = r;
  out_renderpass->g = g;
  out_renderpass->b = b;
  out_renderpass->a = a;
  out_renderpass->depth = depth;
  out_renderpass->stencil = stencil;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  const u32 attachment_description_count = 2;

  VkAttachmentDescription attachment_descriptions[attachment_description_count];
  attachment_descriptions[0].format = context->swapchain.image_format.format;
  attachment_descriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
  attachment_descriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachment_descriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachment_descriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachment_descriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachment_descriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachment_descriptions[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  attachment_descriptions[0].flags = 0;
  attachment_descriptions[1].format = context->device.depth_format;
  attachment_descriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
  attachment_descriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachment_descriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachment_descriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachment_descriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachment_descriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachment_descriptions[1].finalLayout =
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  attachment_descriptions[1].flags = 0;

  VkAttachmentReference color_attachment_reference{};
  color_attachment_reference.attachment = 0;
  color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depth_attachment_reference{};
  depth_attachment_reference.attachment = 1;
  depth_attachment_reference.layout =
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_reference;
  subpass.pDepthStencilAttachment = &depth_attachment_reference;
  subpass.inputAttachmentCount = 0;
  subpass.pInputAttachments = nullptr;
  subpass.pResolveAttachments = nullptr;
  subpass.preserveAttachmentCount = 0;
  subpass.pPreserveAttachments = nullptr;

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependency.dependencyFlags = 0;

  VkRenderPassCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  info.attachmentCount = attachment_description_count;
  info.pAttachments = attachment_descriptions;
  info.subpassCount = 1;
  info.pSubpasses = &subpass;
  info.dependencyCount = 1;
  info.pDependencies = &dependency;
  info.pNext = nullptr;
  info.flags = 0;

  VK_CHECK(vkCreateRenderPass(context->device, &info, context->allocator,
                              &out_renderpass->handle));
}

void renderpass_destroy(Context *context, Renderpass *renderpass) {
  if (renderpass && renderpass->handle) {
    vkDestroyRenderPass(context->device, *renderpass, context->allocator);
    renderpass->handle = VK_NULL_HANDLE;
  }
}

void renderpass_begin(CommandBuffer *command_buffer, Renderpass *renderpass,
                      VkFramebuffer frame_buffer) {
  VkRenderPassBeginInfo info{};
  info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  info.renderPass = *renderpass;
  info.framebuffer = frame_buffer;
  info.renderArea.offset.x = renderpass->x;
  info.renderArea.offset.y = renderpass->y;
  info.renderArea.extent.width = renderpass->w;
  info.renderArea.extent.height = renderpass->h;

  VkClearValue clear_values[2];
  mem_zero(clear_values, sizeof(VkClearValue) * 2);
  clear_values[0].color.float32[0] = renderpass->r;
  clear_values[0].color.float32[1] = renderpass->g;
  clear_values[0].color.float32[2] = renderpass->b;
  clear_values[0].color.float32[3] = renderpass->a;
  clear_values[1].depthStencil.depth = renderpass->depth;
  clear_values[1].depthStencil.stencil = renderpass->stencil;

  info.clearValueCount = 2;
  info.pClearValues = clear_values;

  vkCmdBeginRenderPass(*command_buffer, &info, VK_SUBPASS_CONTENTS_INLINE);
  command_buffer->state = CommandBuffer::State::IN_RENDER_PASS;
}

void renderpass_end(CommandBuffer *command_buffer,
                    Renderpass * /*renderpass*/) {
  vkCmdEndRenderPass(*command_buffer);
  command_buffer->state = CommandBuffer::State::RECORDING;
}

} // namespace ns::vulkan
