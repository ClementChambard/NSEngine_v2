#include "./vulkan_image.h"

#include "../../core/logger.h"

namespace ns::vulkan {

void image_create(Context *context, VkImageType /*image_type*/, u32 width,
                  u32 height, VkFormat format, VkImageTiling tiling,
                  VkImageUsageFlags usage, VkMemoryPropertyFlags memory_flags,
                  bool create_view, VkImageAspectFlags view_aspect_flags,
                  Image *out_image) {
  out_image->width = width;
  out_image->height = height;

  VkImageCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  create_info.imageType = VK_IMAGE_TYPE_2D;
  create_info.extent.width = width;
  create_info.extent.height = height;
  create_info.extent.depth = 1;
  create_info.mipLevels = 4;
  create_info.arrayLayers = 1;
  create_info.format = format;
  create_info.tiling = tiling;
  create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  create_info.usage = usage;
  create_info.samples = VK_SAMPLE_COUNT_1_BIT;
  create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VK_CHECK(vkCreateImage(context->device, &create_info, context->allocator,
                         &out_image->handle));

  VkMemoryRequirements memory_requirements;
  vkGetImageMemoryRequirements(context->device, *out_image,
                               &memory_requirements);

  i32 memory_type = context->find_memory_index(
      memory_requirements.memoryTypeBits, memory_flags);
  if (memory_type == -1) {
    NS_ERROR("Required memory type not found. Image not valid.");
  }

  VkMemoryAllocateInfo memory_allocate_info{};
  memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memory_allocate_info.allocationSize = memory_requirements.size;
  memory_allocate_info.memoryTypeIndex = memory_type;
  VK_CHECK(vkAllocateMemory(context->device, &memory_allocate_info,
                            context->allocator, &out_image->memory));

  VK_CHECK(
      vkBindImageMemory(context->device, *out_image, out_image->memory, 0));

  out_image->view = VK_NULL_HANDLE;
  if (create_view) {
    image_view_create(context, format, out_image, view_aspect_flags);
  }
}

void image_view_create(Context *context, VkFormat format, Image *image,
                       VkImageAspectFlags aspect_flags) {
  VkImageViewCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  create_info.image = *image;
  create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  create_info.format = format;
  create_info.subresourceRange.aspectMask = aspect_flags;
  create_info.subresourceRange.baseMipLevel = 0;
  create_info.subresourceRange.levelCount = 1;
  create_info.subresourceRange.baseArrayLayer = 0;
  create_info.subresourceRange.layerCount = 1;

  VK_CHECK(vkCreateImageView(context->device, &create_info, context->allocator,
                             &image->view));
}

void image_transition_layout(Context *context, CommandBuffer *command_buffer,
                             Image *image, VkFormat /*format*/,
                             VkImageLayout old_layout,
                             VkImageLayout new_layout) {
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = old_layout;
  barrier.newLayout = new_layout;
  barrier.srcQueueFamilyIndex = context->device.graphics_queue_index;
  barrier.dstQueueFamilyIndex = context->device.graphics_queue_index;
  barrier.image = *image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  VkPipelineStageFlags source_stage;
  VkPipelineStageFlags dest_stage;

  if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
      new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dest_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    dest_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else {
    NS_FATAL("Unsupported layout transition!");
    return;
  }

  vkCmdPipelineBarrier(*command_buffer, source_stage, dest_stage, 0, 0, nullptr,
                       0, nullptr, 1, &barrier);
}

void image_copy_from_buffer(Context * /*context*/, Image *image,
                            VkBuffer buffer, CommandBuffer *command_buffer) {
  VkBufferImageCopy region{};
  mem_zero(&region, sizeof(VkBufferImageCopy));
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  region.imageOffset = {0, 0, 0};
  region.imageExtent = {image->width, image->height, 1};

  vkCmdCopyBufferToImage(*command_buffer, buffer, *image,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void image_destroy(Context *context, Image *image) {
  if (image->view) {
    vkDestroyImageView(context->device, image->view, context->allocator);
    image->view = VK_NULL_HANDLE;
  }
  if (image->memory) {
    vkFreeMemory(context->device, image->memory, context->allocator);
    image->memory = nullptr;
  }
  if (image->handle) {
    vkDestroyImage(context->device, *image, context->allocator);
    image->handle = VK_NULL_HANDLE;
  }
}

} // namespace ns::vulkan
