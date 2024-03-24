#ifndef VULKAN_TYPES_INLINE_INCLUDED
#define VULKAN_TYPES_INLINE_INCLUDED

#include "../../containers/freelist.h"
#include "../../containers/vec.h"
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

  usize freelist_memory_requirement;
  ptr freelist_block;
  freelist buffer_freelist;

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

enum class ClearFlags : u8 {
  NONE = 0x0,
  COLOR = 0x1,
  DEPTH = 0x2,
  COLOR_AND_DEPTH = 0x3,
  STENCIL = 0x4,
  COLOR_AND_STENCIL = 0x5,
  DEPTH_AND_STENCIL = 0x6,
  ALL = 0x7,
};

inline ClearFlags operator|(ClearFlags lhs, ClearFlags rhs) {
  return static_cast<ClearFlags>(static_cast<u8>(lhs) | static_cast<u8>(rhs));
}

inline ClearFlags operator&(ClearFlags lhs, ClearFlags rhs) {
  return static_cast<ClearFlags>(static_cast<u8>(lhs) & static_cast<u8>(rhs));
}

inline ClearFlags operator^(ClearFlags lhs, ClearFlags rhs) {
  return static_cast<ClearFlags>(static_cast<u8>(lhs) ^ static_cast<u8>(rhs));
}

struct Renderpass {
  VkRenderPass handle = VK_NULL_HANDLE;
  vec4 render_area;
  vec4 clear_color;
  f32 clear_depth;
  u32 clear_stencil;
  ClearFlags clear_flags;
  bool has_prev_pass;
  bool has_next_pass;

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

struct Swapchain {
  VkSurfaceFormatKHR image_format;
  u8 max_frames_in_flight;
  VkSwapchainKHR handle;
  u32 image_count;
  VkImage *images;
  VkImageView *views;
  Image depth_attachment;
  VkFramebuffer framebuffers[3];

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

struct GeometryData {
  NSID id;
  u32 generation;
  u32 vertex_count;
  u64 vertex_buffer_offset;
  u32 vertex_element_size;
  u32 index_count;
  u64 index_buffer_offset;
  u32 index_element_size;
};

struct MaterialShader {
  struct GlobalUBO {
    mat4 projection;
    mat4 view;
    mat4 m_reserved0;
    mat4 m_reserved1;
  };

  struct InstanceUBO {
    vec4 diffuse_color;
    vec4 v_reserved0;
    vec4 v_reserved1;
    vec4 v_reserved2;
    mat4 m_reserved3;
    mat4 m_reserved4;
    mat4 m_reserved5;
  };

  static constexpr usize STAGE_COUNT = 2;
  static constexpr usize DESCRIPTOR_COUNT = 2;
  static constexpr usize SAMPLER_COUNT = 1;
  static constexpr usize MAX_MATERIAL_COUNT = 1024;

  struct InstanceState {
    VkDescriptorSet descriptor_sets[3];
    DescriptorState descriptor_states[DESCRIPTOR_COUNT];
  };

  ShaderStage stages[STAGE_COUNT];

  VkDescriptorPool global_descriptor_pool;
  VkDescriptorSetLayout global_descriptor_set_layout;
  VkDescriptorSet global_descriptor_sets[3];

  GlobalUBO global_ubo;

  Buffer global_uniform_buffer;

  VkDescriptorPool object_descriptor_pool;
  VkDescriptorSetLayout object_descriptor_set_layout;
  Buffer object_uniform_buffer;
  u32 object_uniform_buffer_index;

  TextureUse sampler_uses[SAMPLER_COUNT];

  InstanceState instance_states[MAX_MATERIAL_COUNT];

  Pipeline pipeline;
};

struct UiShader {
  struct GlobalUBO {
    mat4 projection;
    mat4 view;
    mat4 m_reserved0;
    mat4 m_reserved1;
  };

  struct InstanceUBO {
    vec4 diffuse_color;
    vec4 v_reserved0;
    vec4 v_reserved1;
    vec4 v_reserved2;
    mat4 m_reserved3;
    mat4 m_reserved4;
    mat4 m_reserved5;
  };

  static constexpr usize STAGE_COUNT = 2;
  static constexpr usize DESCRIPTOR_COUNT = 2;
  static constexpr usize SAMPLER_COUNT = 1;
  static constexpr usize MAX_UI_COUNT = 1024;

  struct InstanceState {
    VkDescriptorSet descriptor_sets[3];
    DescriptorState descriptor_states[DESCRIPTOR_COUNT];
  };

  ShaderStage stages[STAGE_COUNT];

  VkDescriptorPool global_descriptor_pool;
  VkDescriptorSetLayout global_descriptor_set_layout;
  VkDescriptorSet global_descriptor_sets[3];

  GlobalUBO global_ubo;

  Buffer global_uniform_buffer;

  VkDescriptorPool object_descriptor_pool;
  VkDescriptorSetLayout object_descriptor_set_layout;
  Buffer object_uniform_buffer;
  u32 object_uniform_buffer_index;

  TextureUse sampler_uses[SAMPLER_COUNT];

  InstanceState instance_states[MAX_UI_COUNT];

  Pipeline pipeline;
};

struct Context {
  static constexpr usize MAX_GEOMETRY_COUNT = 4096;

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
  Renderpass ui_renderpass;

  Buffer object_vertex_buffer;
  Buffer object_index_buffer;

  Vec<CommandBuffer> graphics_command_buffers;

  Vec<VkSemaphore> image_available_semaphores;
  Vec<VkSemaphore> queue_complete_semaphores;
  u32 in_flight_fence_count;
  VkFence in_flight_fences[2];
  VkFence *images_in_flight[3];

  u32 image_index;
  u32 current_frame;

  bool recreating_swapchain;

  MaterialShader material_shader;
  UiShader ui_shader;

  GeometryData geometries[MAX_GEOMETRY_COUNT];

  VkFramebuffer world_framebuffers[3];

  i32 (*find_memory_index)(u32 type_filter, u32 property_flags);
};

struct TextureData {
  Image image;
  VkSampler sampler;
};

} // namespace ns::vulkan

#endif // VULKAN_TYPES_INLINE_INCLUDED
