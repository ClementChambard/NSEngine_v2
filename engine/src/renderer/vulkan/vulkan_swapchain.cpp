#include "./vulkan_swapchain.h"
#include "../../core/logger.h"
#include "../../core/memory.h"
#include "./vulkan_device.h"
#include "./vulkan_image.h"

namespace ns::vulkan {

static void create(Context *context, u32 width, u32 height,
                   Swapchain *swapchain);
static void destroy(Context *context, Swapchain *swapchain);

void swapchain_create(Context *context, u32 width, u32 height,
                      Swapchain *out_swapchain) {
  create(context, width, height, out_swapchain);
}

void swapchain_recreate(Context *context, u32 width, u32 height,
                        Swapchain *swapchain) {
  destroy(context, swapchain);
  create(context, width, height, swapchain);
}

void swapchain_destroy(Context *context, Swapchain *swapchain) {
  destroy(context, swapchain);
  ns::free(swapchain->images, sizeof(VkImage) * swapchain->image_count,
           MemTag::RENDERER);
  ns::free(swapchain->views, sizeof(VkImageView) * swapchain->image_count,
           MemTag::RENDERER);
  swapchain->images = nullptr;
  swapchain->views = nullptr;
  swapchain->image_count = 0;
}

bool swapchain_acquire_next_image_index(Context *context, Swapchain *swapchain,
                                        u64 timeout_ns,
                                        VkSemaphore image_available_semaphore,
                                        VkFence fence, u32 *out_image_index) {
  VkResult result =
      vkAcquireNextImageKHR(context->device, *swapchain, timeout_ns,
                            image_available_semaphore, fence, out_image_index);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    device_query_swapchain_support(context->device, context->surface,
                                   &context->device.swapchain_support);
    swapchain_recreate(
        context,
        context->device.swapchain_support.capabilities.currentExtent.width,
        context->device.swapchain_support.capabilities.currentExtent.height,
        swapchain);
    return false;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    NS_FATAL("Failed to acquire swapchain image");
    return false;
  }

  return true;
}

void swapchain_present(Context *context, Swapchain *swapchain,
                       VkQueue /*graphics_queue*/, VkQueue present_queue,
                       VkSemaphore render_complete_semaphore,
                       u32 present_image_index) {
  VkPresentInfoKHR present_info{};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = &render_complete_semaphore;
  present_info.swapchainCount = 1;
  present_info.pSwapchains = &swapchain->handle;
  present_info.pImageIndices = &present_image_index;
  present_info.pResults = 0;

  VkResult result = vkQueuePresentKHR(present_queue, &present_info);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    swapchain_recreate(
        context,
        context->device.swapchain_support.capabilities.currentExtent.width,
        context->device.swapchain_support.capabilities.currentExtent.height,
        swapchain);
  } else if (result != VK_SUCCESS) {
    NS_FATAL("Failed to present swapchain image!");
  }

  context->current_frame =
      (context->current_frame + 1) % swapchain->max_frames_in_flight;
}

static void create(Context *context, u32 width, u32 height,
                   Swapchain *swapchain) {
  VkExtent2D swapchain_extent = {width, height};

  bool found = false;
  for (u32 i = 0; i < context->device.swapchain_support.format_count; i++) {
    VkSurfaceFormatKHR format = context->device.swapchain_support.formats[i];

    if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      swapchain->image_format = format;
      found = true;
      break;
    }
  }

  if (!found)
    swapchain->image_format = context->device.swapchain_support.formats[0];

  VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
  for (u32 i = 0; i < context->device.swapchain_support.present_mode_count;
       i++) {
    VkPresentModeKHR mode = context->device.swapchain_support.present_modes[i];
    if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      present_mode = mode;
      break;
    }
  }

  device_query_swapchain_support(context->device, context->surface,
                                 &context->device.swapchain_support);

  if (context->device.swapchain_support.capabilities.currentExtent.width !=
      UINT32_MAX) {
    swapchain_extent =
        context->device.swapchain_support.capabilities.currentExtent;
  }

  VkExtent2D min =
      context->device.swapchain_support.capabilities.minImageExtent;
  VkExtent2D max =
      context->device.swapchain_support.capabilities.maxImageExtent;
  swapchain_extent.width =
      (swapchain_extent.width < min.width   ? min.width
       : swapchain_extent.width > max.width ? max.width
                                            : swapchain_extent.width);
  swapchain_extent.height =
      (swapchain_extent.height < min.height   ? min.height
       : swapchain_extent.height > max.height ? max.height
                                              : swapchain_extent.height);

  u32 image_count =
      context->device.swapchain_support.capabilities.minImageCount + 1;
  if (context->device.swapchain_support.capabilities.maxImageCount > 0 &&
      image_count >
          context->device.swapchain_support.capabilities.maxImageCount) {
    image_count = context->device.swapchain_support.capabilities.maxImageCount;
  }

  swapchain->max_frames_in_flight = image_count - 1;

  VkSwapchainCreateInfoKHR swapchain_create_info{};
  swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchain_create_info.surface = context->surface;
  swapchain_create_info.minImageCount = image_count;
  swapchain_create_info.imageFormat = swapchain->image_format.format;
  swapchain_create_info.imageColorSpace = swapchain->image_format.colorSpace;
  swapchain_create_info.imageExtent = swapchain_extent;
  swapchain_create_info.imageArrayLayers = 1;
  swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  if (context->device.graphics_queue_index !=
      context->device.present_queue_index) {
    u32 queueFamilyIndices[] = {
        static_cast<u32>(context->device.graphics_queue_index),
        static_cast<u32>(context->device.present_queue_index)};
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapchain_create_info.queueFamilyIndexCount = 2;
    swapchain_create_info.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.queueFamilyIndexCount = 0;
    swapchain_create_info.pQueueFamilyIndices = nullptr;
  }

  swapchain_create_info.preTransform =
      context->device.swapchain_support.capabilities.currentTransform;
  swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchain_create_info.presentMode = present_mode;
  swapchain_create_info.clipped = VK_TRUE;
  swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

  VK_CHECK(vkCreateSwapchainKHR(context->device, &swapchain_create_info,
                                context->allocator, &swapchain->handle));

  context->current_frame = 0;
  swapchain->image_count = 0;

  VK_CHECK(vkGetSwapchainImagesKHR(context->device, *swapchain,
                                   &swapchain->image_count, nullptr));
  if (!swapchain->images) {
    swapchain->images = reinterpret_cast<VkImage *>(
        ns::alloc(sizeof(VkImage) * swapchain->image_count, MemTag::RENDERER));
  }
  if (!swapchain->views) {
    swapchain->views = reinterpret_cast<VkImageView *>(ns::alloc(
        sizeof(VkImageView) * swapchain->image_count, MemTag::RENDERER));
  }
  VK_CHECK(vkGetSwapchainImagesKHR(context->device, *swapchain,
                                   &swapchain->image_count, swapchain->images));

  for (u32 i = 0; i < swapchain->image_count; ++i) {
    VkImageViewCreateInfo view_info{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = swapchain->images[i];
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = swapchain->image_format.format;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(context->device, &view_info, context->allocator,
                               &swapchain->views[i]));
  }

  if (!device_detect_depth_format(&context->device)) {
    context->device.depth_format = VK_FORMAT_UNDEFINED;
    NS_FATAL("Failed to find a supported format!");
  }

  image_create(context, VK_IMAGE_TYPE_2D, swapchain_extent.width,
               swapchain_extent.height, context->device.depth_format,
               VK_IMAGE_TILING_OPTIMAL,
               VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true,
               VK_IMAGE_ASPECT_DEPTH_BIT, &swapchain->depth_attachment);

  NS_INFO("Swapchain created successfully.");
}

static void destroy(Context *context, Swapchain *swapchain) {
  vkDeviceWaitIdle(context->device);

  image_destroy(context, &swapchain->depth_attachment);

  for (u32 i = 0; i < swapchain->image_count; i++) {
    vkDestroyImageView(context->device, swapchain->views[i],
                       context->allocator);
  }

  vkDestroySwapchainKHR(context->device, *swapchain, context->allocator);
}

} // namespace ns::vulkan
