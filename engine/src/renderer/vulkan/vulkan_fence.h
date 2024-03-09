#ifndef VULKAN_FENCE_HEADER_INCLUDED
#define VULKAN_FENCE_HEADER_INCLUDED

#include "./vulkan_types.inl"

namespace ns::vulkan {

void fence_create(Context *context, bool create_signaled, Fence *out_fence);

void fence_destroy(Context *context, Fence *fence);

bool fence_wait(Context *context, Fence *fence, u64 timeout_ns);

void fence_reset(Context *context, Fence *fence);

} // namespace ns::vulkan

#endif // VULKAN_FENCE_HEADER_INCLUDED
