#ifndef VULKAN_TYPES_INLINE_INCLUDED
#define VULKAN_TYPES_INLINE_INCLUDED

#include "../../containers/vector.h"
#include "../../core/asserts.h"
#include "../../defines.h"
#include "../renderer_types.inl"

#include <vulkan/vulkan.h>

#ifdef NS_ASSERTIONS_ENABLED
#define VK_CHECK(expr)                                                         \
  { NS_ASSERT(expr == VK_SUCCESS); }
#else
#define VK_CHECK(expr)                                                         \
  {                                                                            \
    if (expr != VK_SUCCESS) {                                                  \
      NS_ERROR("%s: failure", #expr);                                          \
      return false;                                                            \
    }                                                                          \
  }
#endif

namespace ns::vulkan {

struct Buffer {
  usize total_size;
  VkBuffer handle;
  VkBufferUsageFlagBits usage;
  bool is_locked;
  VkDeviceMemory memory;
  i32 memory_index;
  u32 memory_property_flags;

  operator VkBuffer() const { return handle; }
};

struct SwapchainSupportInfo {
  VkSurfaceCapabilitiesKHR capabilities;
  u32 format_count;
  VkSurfaceFormatKHR *formats;
  u32 present_mode_count;
  VkPresentModeKHR *present_modes;
};

struct Device {
  VkPhysicalDevice physical_device;
  VkDevice logical_device;
  SwapchainSupportInfo swapchain_support;
  i32 graphics_queue_index;
  i32 present_queue_index;
  i32 transfer_queue_index;
  VkQueue graphics_queue;
  VkQueue present_queue;
  VkQueue transfer_queue;
  VkCommandPool graphics_command_pool;
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceFeatures features;
  VkPhysicalDeviceMemoryProperties memory;
  VkFormat depth_format;
  bool supports_device_local_host_visible;

  operator VkPhysicalDevice() const { return physical_device; }
  operator VkDevice() const { return logical_device; }
};

struct Image {
  VkImage handle;
  VkDeviceMemory memory;
  VkImageView view;
  u32 width;
  u32 height;

  operator VkImage() const { return handle; }
};

struct Renderpass {
  VkRenderPass handle = VK_NULL_HANDLE;
  f32 x, y, w, h;
  f32 r, g, b, a;
  f32 depth;
  u32 stencil;

  enum class State {
    READY,
    RECORDING,
    IN_RENDER_PASS,
    RECORDING_ENDED,
    SUBMITTED,
    NOT_ALLOCATED,
  } state = State::NOT_ALLOCATED;

  operator VkRenderPass() const { return handle; }
};

struct Framebuffer {
  VkFramebuffer handle;
  u32 attachment_count;
  VkImageView *attachments;
  Renderpass *renderpass;

  operator VkFramebuffer() const { return handle; }
};

struct Swapchain {
  VkSurfaceFormatKHR image_format;
  u8 max_frames_in_flight;
  VkSwapchainKHR handle;
  u32 image_count;
  VkImage *images;
  VkImageView *views;
  Image depth_attachment;
  vector<Framebuffer> framebuffers;

  operator VkSwapchainKHR() const { return handle; }
};

struct CommandBuffer {
  VkCommandBuffer handle = VK_NULL_HANDLE;

  enum class State {
    READY,
    RECORDING,
    IN_RENDER_PASS,
    RECORDING_ENDED,
    SUBMITTED,
    NOT_ALLOCATED,
  } state = State::NOT_ALLOCATED;

  operator VkCommandBuffer() const { return handle; }
};

struct Fence {
  VkFence handle;
  bool is_signaled;

  operator VkFence() const { return handle; }
};

struct ShaderStage {
  VkShaderModuleCreateInfo create_info;
  VkShaderModule handle;
  VkPipelineShaderStageCreateInfo shader_stage_create_info;

  operator VkShaderModule() const { return handle; }
};

struct Pipeline {
  VkPipeline handle;
  VkPipelineLayout layout;

  operator VkPipeline() const { return handle; }
};

struct DescriptorState {
  u32 generations[3];
  NSID ids[3];
};

struct MaterialShader {
  static constexpr usize SHADER_STAGE_COUNT = 2;
  static constexpr usize SHADER_DESCRIPTOR_COUNT = 2;
  static constexpr usize MAX_OBJECT_COUNT = 1024;

  struct ObjectState {
    VkDescriptorSet descriptor_sets[3];
    DescriptorState descriptor_states[SHADER_DESCRIPTOR_COUNT];
  };

  ShaderStage stages[SHADER_STAGE_COUNT];

  VkDescriptorPool global_descriptor_pool;
  VkDescriptorSetLayout global_descriptor_set_layout;
  VkDescriptorSet global_descriptor_sets[3];

  global_uniform_object global_ubo;

  Buffer global_uniform_buffer;

  VkDescriptorPool object_descriptor_pool;
  VkDescriptorSetLayout object_descriptor_set_layout;
  Buffer object_uniform_buffer;
  u32 object_uniform_buffer_index;

  ObjectState object_states[MAX_OBJECT_COUNT];

  Pipeline pipeline;
};

struct Context {
  f32 frame_delta_time;
  // XXX: ERROR when using these: not in sync with swapchain capabilities
  // u32 framebuffer_width;
  // u32 framebuffer_height;
  u64 framebuffer_size_generation;
  u64 framebuffer_size_last_generation;
  VkInstance instance;
  VkAllocationCallbacks *allocator;
  VkSurfaceKHR surface;

#if defined(_DEBUG)
  VkDebugUtilsMessengerEXT debug_messenger;
#endif

  Device device;

  Swapchain swapchain;
  Renderpass main_renderpass;

  Buffer object_vertex_buffer;
  Buffer object_index_buffer;

  vector<CommandBuffer> graphics_command_buffers;

  vector<VkSemaphore> image_available_semaphores;
  vector<VkSemaphore> queue_complete_semaphores;
  u32 in_flight_fence_count;
  vector<Fence> in_flight_fences;
  vector<Fence *> images_in_flight;

  u32 image_index;
  u32 current_frame;

  bool recreating_swapchain;

  MaterialShader material_shader;

  usize geometry_vertex_offset;
  usize geometry_index_offset;

  i32 (*find_memory_index)(u32 type_filter, u32 property_flags);
};

struct TextureData {
  Image image;
  VkSampler sampler;
};

} // namespace ns::vulkan

#endif // VULKAN_TYPES_INLINE_INCLUDED
