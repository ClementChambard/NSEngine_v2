#ifndef VULKAN_SWAPCHAIN_HEADER_INCLUDED
#define VULKAN_SWAPCHAIN_HEADER_INCLUDED

#include "./vulkan_types.inl"

namespace ns::vulkan {

void swapchain_create(Context *context, u32 width, u32 height,
                      Swapchain *out_swapchain);

void swapchain_recreate(Context *context, u32 width, u32 height,
                        Swapchain *swapchain);

void swapchain_destroy(Context *context, Swapchain *swapchain);

bool swapchain_acquire_next_image_index(Context *context, Swapchain *swapchain,
                                        u64 timeout_ns,
                                        VkSemaphore image_available_semaphore,
                                        VkFence fence, u32 *out_image_index);

void swapchain_present(Context *context, Swapchain *swapchain,
                       VkQueue graphics_queue, VkQueue present_queue,
                       VkSemaphore render_complete_semaphore,
                       u32 present_image_index);

} // namespace ns::vulkan

#endif // VULKAN_SWAPCHAIN_HEADER_INCLUDED
