#ifndef VULKAN_UTILS_HEADER_INCLUDED
#define VULKAN_UTILS_HEADER_INCLUDED

#include "./vulkan_types.inl"

namespace ns::vulkan {

cstr result_string(VkResult result, bool get_extended);

bool result_is_success(VkResult result);

} // namespace ns::vulkan

#endif // VULKAN_UTILS_HEADER_INCLUDED
