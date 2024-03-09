#ifndef VULKAN_DEVICE_HEADER_INCLUDED
#define VULKAN_DEVICE_HEADER_INCLUDED

#include "./vulkan_types.inl"

namespace ns::vulkan {

bool device_create(Context *context);

void device_destroy(Context *context);

void device_query_swapchain_support(VkPhysicalDevice physical_device,
                                    VkSurfaceKHR surface,
                                    SwapchainSupportInfo *out_support_info);

bool device_detect_depth_format(Device *device);

} // namespace ns::vulkan

#endif // VULKAN_DEVICE_HEADER_INCLUDED
