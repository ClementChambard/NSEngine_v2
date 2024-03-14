#include "./vulkan_renderpass.h"

#include "../../core/ns_memory.h"

namespace ns::vulkan {

void renderpass_create(Context *context, Renderpass *out_renderpass,
                       vec4 render_area, vec4 color, f32 depth, u32 stencil,
                       ClearFlags clear_flags, bool has_prev_pass,
                       bool has_next_pass) {
  out_renderpass->render_area = render_area;
  out_renderpass->clear_color = color;
  out_renderpass->clear_depth = depth;
  out_renderpass->clear_stencil = stencil;
  out_renderpass->clear_flags = clear_flags;
  out_renderpass->has_prev_pass = has_prev_pass;
  out_renderpass->has_next_pass = has_next_pass;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

  u32 attachment_description_count = 0;
  VkAttachmentDescription attachment_descriptions[2];

  bool do_clear_color = (clear_flags & ClearFlags::COLOR) != ClearFlags::NONE;
  attachment_descriptions[0].format = context->swapchain.image_format.format;
  attachment_descriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
  attachment_descriptions[0].loadOp =
      do_clear_color ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
  attachment_descriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachment_descriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachment_descriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachment_descriptions[0].initialLayout =
      has_prev_pass ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                    : VK_IMAGE_LAYOUT_UNDEFINED;
  attachment_descriptions[0].finalLayout =
      has_next_pass ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                    : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  attachment_descriptions[0].flags = 0;
  attachment_description_count++;

  VkAttachmentReference color_attachment_reference{};
  color_attachment_reference.attachment = 0;
  color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depth_attachment_reference{};

  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_reference;
  subpass.inputAttachmentCount = 0;
  subpass.pInputAttachments = nullptr;
  subpass.pResolveAttachments = nullptr;
  subpass.preserveAttachmentCount = 0;
  subpass.pPreserveAttachments = nullptr;

  bool do_clear_depth = (clear_flags & ClearFlags::DEPTH) != ClearFlags::NONE;
  if (do_clear_depth) {
    attachment_descriptions[1].format = context->device.depth_format;
    attachment_descriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_descriptions[1].loadOp = do_clear_depth
                                            ? VK_ATTACHMENT_LOAD_OP_CLEAR
                                            : VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment_descriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_descriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_descriptions[1].stencilStoreOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_descriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_descriptions[1].finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachment_descriptions[1].flags = 0;
    attachment_description_count++;

    depth_attachment_reference.attachment = 1;
    depth_attachment_reference.layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    subpass.pDepthStencilAttachment = &depth_attachment_reference;
  } else {
    mem_zero(&attachment_descriptions[1], sizeof(VkAttachmentDescription));
    subpass.pDepthStencilAttachment = nullptr;
  }

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
  info.renderArea.offset.x = renderpass->render_area.x;
  info.renderArea.offset.y = renderpass->render_area.y;
  info.renderArea.extent.width = renderpass->render_area.z;
  info.renderArea.extent.height = renderpass->render_area.w;
  info.clearValueCount = 0;
  info.pClearValues = nullptr;

  VkClearValue clear_values[2];
  mem_zero(clear_values, sizeof(VkClearValue) * 2);
  bool do_clear_color =
      (renderpass->clear_flags & ClearFlags::COLOR) != ClearFlags::NONE;
  if (do_clear_color) {
    mem_copy(clear_values[info.clearValueCount].color.float32,
             &renderpass->clear_color, sizeof(vec4));
    info.clearValueCount++;
  }

  bool do_clear_depth =
      (renderpass->clear_flags & ClearFlags::DEPTH) != ClearFlags::NONE;
  if (do_clear_depth) {
    clear_values[info.clearValueCount].depthStencil.depth =
        renderpass->clear_depth;

    bool do_clear_stencil =
        (renderpass->clear_flags & ClearFlags::STENCIL) != ClearFlags::NONE;
    clear_values[info.clearValueCount].depthStencil.stencil =
        do_clear_stencil ? renderpass->clear_stencil : 0;
    info.clearValueCount++;
  }

  info.pClearValues = info.clearValueCount > 0 ? clear_values : nullptr;

  vkCmdBeginRenderPass(*command_buffer, &info, VK_SUBPASS_CONTENTS_INLINE);
  command_buffer->state = CommandBuffer::State::IN_RENDER_PASS;
}

void renderpass_end(CommandBuffer *command_buffer,
                    Renderpass * /*renderpass*/) {
  vkCmdEndRenderPass(*command_buffer);
  command_buffer->state = CommandBuffer::State::RECORDING;
}

} // namespace ns::vulkan
