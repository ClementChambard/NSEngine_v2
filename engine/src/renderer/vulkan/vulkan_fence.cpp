#include "./vulkan_fence.h"

#include "../../core/logger.h"

namespace ns::vulkan {

void fence_create(Context *context, bool create_signaled, Fence *out_fence) {
  out_fence->is_signaled = create_signaled;
  VkFenceCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  if (out_fence->is_signaled) {
    info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  }

  VK_CHECK(vkCreateFence(context->device, &info, context->allocator,
                         &out_fence->handle));
}

void fence_destroy(Context *context, Fence *fence) {
  if (fence->handle) {
    vkDestroyFence(context->device, *fence, context->allocator);
    fence->handle = VK_NULL_HANDLE;
  }
  fence->is_signaled = false;
}

bool fence_wait(Context *context, Fence *fence, u64 timeout_ns) {
  if (!fence->is_signaled) {
    VkResult result =
        vkWaitForFences(context->device, 1, &fence->handle, true, timeout_ns);
    switch (result) {
    case VK_SUCCESS:
      fence->is_signaled = true;
      return true;
    case VK_TIMEOUT:
      NS_WARN("vk_fence_wait - Timed out.");
      break;
    case VK_ERROR_DEVICE_LOST:
      NS_ERROR("vk_fence_wait - Device lost.");
      break;
    case VK_ERROR_OUT_OF_HOST_MEMORY:
      NS_ERROR("vk_fence_wait - Out of host memory.");
      break;
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
      NS_ERROR("vk_fence_wait - Out of device memory.");
      break;
    default:
      NS_ERROR("vk_fence_wait - An unknown error has occurred.");
      break;
    }
  } else {
    return true;
  }

  return false;
}

void fence_reset(Context *context, Fence *fence) {
  if (fence->is_signaled) {
    VK_CHECK(vkResetFences(context->device, 1, &fence->handle));
    fence->is_signaled = false;
  }
}

} // namespace ns::vulkan
