#include "./vulkan_backend.h"

#include "../../containers/vector.h"
#include "../../core/application.h"
#include "../../core/logger.h"
#include "../../core/ns_string.h"
#include "../../math/types/math_types.h"
#include "./shaders/vulkan_material_shader.h"
#include "./vulkan_buffer.h"
#include "./vulkan_command_buffer.h"
#include "./vulkan_device.h"
#include "./vulkan_fence.h"
#include "./vulkan_framebuffer.h"
#include "./vulkan_image.h"
#include "./vulkan_platform.h"
#include "./vulkan_renderpass.h"
#include "./vulkan_swapchain.h"
#include "./vulkan_types.inl"
#include "./vulkan_utils.h"

namespace ns::vulkan {

static Context context;
static u32 cached_framebuffer_width = 0;
static u32 cached_framebuffer_height = 0;

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data, ptr user_data);

i32 find_memory_index(u32 type_filter, u32 property_flags);
bool create_buffers(Context *context);

void create_command_buffers(renderer_backend *backend);
void regenerate_framebuffers(renderer_backend *backend, Swapchain *swapchain,
                             Renderpass *renderpass);
bool recreate_swapchain(renderer_backend *backend);

void upload_data_range(Context *context, VkCommandPool pool, VkFence fence,
                       VkQueue queue, Buffer *buffer, usize offset, usize size,
                       ptr data) {
  VkBufferUsageFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  Buffer staging;
  buffer_create(context, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, flags, true,
                &staging);
  buffer_load_data(context, &staging, 0, size, 0, data);
  buffer_copy_to(context, pool, fence, queue, staging.handle, 0, buffer->handle,
                 offset, size);
  buffer_destroy(context, &staging);
}

bool renderer_backend_initialize(renderer_backend *backend,
                                 cstr application_name) {
  context.find_memory_index = find_memory_index;

  // TODO(ClementChambard): custom allocator
  context.allocator = nullptr;

  application_get_framebuffer_size(&cached_framebuffer_width,
                                   &cached_framebuffer_height);
  u32 framebuffer_width =
      (cached_framebuffer_width != 0) ? cached_framebuffer_width : 800;
  u32 framebuffer_height =
      (cached_framebuffer_height != 0) ? cached_framebuffer_height : 600;
  cached_framebuffer_width = 0;
  cached_framebuffer_height = 0;

  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.apiVersion = VK_API_VERSION_1_2;
  app_info.pApplicationName = application_name;
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "NS Engine";
  app_info.engineVersion = VK_MAKE_VERSION(2, 0, 0);

  ns::vector<cstr> required_extensions{};
  required_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
  platform_get_required_extension_names(&required_extensions);

  ns::vector<cstr> required_validation_layers{};

#if defined(_DEBUG)
  required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

  NS_DEBUG("Required extensions: ");
  for (auto &s : required_extensions) {
    NS_DEBUG("- %s", s);
  }

  NS_INFO("Validation layers enabled. Enumerating...");
  required_validation_layers.push_back("VK_LAYER_KHRONOS_validation");

  u32 available_layer_count = 0;
  VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, nullptr));
  ns::vector<VkLayerProperties> available_layers(available_layer_count);
  VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count,
                                              available_layers.data()));

  for (usize i = 0; i < required_validation_layers.size(); i++) {
    NS_INFO("Searching for layer: %s...", required_validation_layers[i]);
    bool found = false;
    for (u32 j = 0; j < available_layer_count; j++) {
      if (string_eq(required_validation_layers[i],
                    available_layers[j].layerName)) {
        found = true;
        NS_INFO("Found.");
        break;
      }
    }

    if (!found) {
      NS_FATAL("Required validation layer is missing: %s",
               required_validation_layers[i]);
      return false;
    }
  }
  NS_INFO("All required validation layers are present.");
#endif

  VkInstanceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  create_info.enabledExtensionCount = required_extensions.size();
  create_info.ppEnabledExtensionNames = required_extensions.data();
  create_info.enabledLayerCount = required_validation_layers.size();
  create_info.ppEnabledLayerNames = required_validation_layers.data();

  VK_CHECK(
      vkCreateInstance(&create_info, context.allocator, &context.instance));
  NS_INFO("Vulkan instance created.");

#if defined(_DEBUG)
  NS_DEBUG("Creating Vulkan debugger...");
  VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
  debug_create_info.sType =
      VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  debug_create_info.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT; // |
  /* VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
     VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT; */
  debug_create_info.messageType =
      VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
  debug_create_info.pfnUserCallback = vk_debug_callback;
  debug_create_info.pUserData = 0;

  PFN_vkCreateDebugUtilsMessengerEXT func =
      (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
          context.instance, "vkCreateDebugUtilsMessengerEXT");
  NS_ASSERT_M(func, "Failed to create debug messenger!");
  VK_CHECK(func(context.instance, &debug_create_info, context.allocator,
                &context.debug_messenger));
  NS_DEBUG("Vulkan debugger created.")
#endif

  NS_DEBUG("Creating Vulkan surface...");
  if (!platform_create_vulkan_surface(&context)) {
    NS_ERROR("Failed to create platform surface");
    return false;
  }
  NS_DEBUG("Vulkan surface created.");

  if (!device_create(&context)) {
    NS_ERROR("Failed to create device");
    return false;
  }

  swapchain_create(&context, framebuffer_width, framebuffer_height,
                   &context.swapchain);

  renderpass_create(&context, &context.main_renderpass, 0, 0, framebuffer_width,
                    framebuffer_height, 0.0f, 0.0f, 0.2f, 1.0f, 1.0f, 0);

  context.swapchain.framebuffers.resize(context.swapchain.image_count);
  regenerate_framebuffers(backend, &context.swapchain,
                          &context.main_renderpass);

  create_command_buffers(backend);

  context.image_available_semaphores.resize(
      context.swapchain.max_frames_in_flight);
  context.queue_complete_semaphores.resize(
      context.swapchain.max_frames_in_flight);
  context.in_flight_fences.resize(context.swapchain.max_frames_in_flight);

  for (u8 i = 0; i < context.swapchain.max_frames_in_flight; i++) {
    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(context.device, &semaphore_info, context.allocator,
                      &context.image_available_semaphores[i]);
    vkCreateSemaphore(context.device, &semaphore_info, context.allocator,
                      &context.queue_complete_semaphores[i]);
    fence_create(&context, true, &context.in_flight_fences[i]);
  }

  context.images_in_flight.resize(context.swapchain.image_count);
  for (u32 i = 0; i < context.swapchain.image_count; i++) {
    context.images_in_flight[i] = nullptr;
  }

  if (!material_shader_create(&context, &context.material_shader)) {
    NS_ERROR("Error loading built-in basic_lighting shader.");
    return false;
  }

  create_buffers(&context);

  // TODO(ClementChambard): Temporary test code
  const u32 vertex_count = 4;
  vertex_3d verts[vertex_count];
  mem_zero(verts, sizeof(vertex_3d) * vertex_count);
  verts[0].position.x = -5.0f;
  verts[0].position.y = -5.0f;
  verts[0].texcoord.u = 0.0f;
  verts[0].texcoord.v = 0.0f;
  verts[1].position.x = 5.0f;
  verts[1].position.y = -5.0f;
  verts[1].texcoord.u = 1.0f;
  verts[1].texcoord.v = 0.0f;
  verts[2].position.x = 5.0f;
  verts[2].position.y = 5.0f;
  verts[2].texcoord.u = 1.0f;
  verts[2].texcoord.v = 1.0f;
  verts[3].position.x = -5.0f;
  verts[3].position.y = 5.0f;
  verts[3].texcoord.u = 0.0f;
  verts[3].texcoord.v = 1.0f;

  const u32 index_count = 6;
  u32 indices[index_count] = {0, 1, 2, 0, 2, 3};

  upload_data_range(&context, context.device.graphics_command_pool, 0,
                    context.device.graphics_queue,
                    &context.object_vertex_buffer, 0,
                    sizeof(vertex_3d) * vertex_count, verts);
  upload_data_range(&context, context.device.graphics_command_pool, 0,
                    context.device.graphics_queue, &context.object_index_buffer,
                    0, sizeof(u32) * index_count, indices);

  u32 object_id = 0;
  if (!material_shader_acquire_resources(&context, &context.material_shader,
                                         &object_id)) {
    NS_ERROR("Failed to acquire shader resources.");
    return false;
  }

  // TODO(ClementChambard): End temp code

  NS_INFO("Vulkan renderer initialized successfully.");
  return true;
}

void renderer_backend_shutdown(renderer_backend * /* backend */) {
  vkDeviceWaitIdle(context.device);

  buffer_destroy(&context, &context.object_vertex_buffer);
  buffer_destroy(&context, &context.object_index_buffer);

  material_shader_destroy(&context, &context.material_shader);

  for (u8 i = 0; i < context.swapchain.max_frames_in_flight; i++) {
    if (context.image_available_semaphores[i]) {
      vkDestroySemaphore(context.device, context.image_available_semaphores[i],
                         context.allocator);
    }
    if (context.queue_complete_semaphores[i]) {
      vkDestroySemaphore(context.device, context.queue_complete_semaphores[i],
                         context.allocator);
    }
    fence_destroy(&context, &context.in_flight_fences[i]);
  }
  vector<VkSemaphore>().swap(context.image_available_semaphores);
  vector<VkSemaphore>().swap(context.queue_complete_semaphores);
  vector<Fence>().swap(context.in_flight_fences);
  vector<Fence *>().swap(context.images_in_flight);

  for (u32 i = 0; i < context.swapchain.image_count; i++) {
    if (context.graphics_command_buffers[i].handle) {
      command_buffer_free(&context, context.device.graphics_command_pool,
                          &context.graphics_command_buffers[i]);
    }
  }
  vector<CommandBuffer>().swap(context.graphics_command_buffers);

  for (u32 i = 0; i < context.swapchain.image_count; i++) {
    framebuffer_destroy(&context, &context.swapchain.framebuffers[i]);
  }
  vector<Framebuffer>().swap(context.swapchain.framebuffers);

  renderpass_destroy(&context, &context.main_renderpass);

  NS_DEBUG("Destroying Vulkan swapchain...");
  swapchain_destroy(&context, &context.swapchain);

  NS_DEBUG("Destroying Vulkan device...");
  device_destroy(&context);

  NS_DEBUG("Destroying Vulkan surface...");
  if (context.surface) {
    vkDestroySurfaceKHR(context.instance, context.surface, context.allocator);
    context.surface = VK_NULL_HANDLE;
  }

#if defined(_DEBUG)
  NS_DEBUG("Destroying Vulkan debugger...");
  if (context.debug_messenger) {
    PFN_vkDestroyDebugUtilsMessengerEXT func =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            context.instance, "vkDestroyDebugUtilsMessengerEXT");
    func(context.instance, context.debug_messenger, context.allocator);
  }
#endif

  NS_DEBUG("Destroying Vulkan instance...");
  vkDestroyInstance(context.instance, context.allocator);
}

void renderer_backend_on_resized(renderer_backend * /* backend */, u16 width,
                                 u16 height) {
  cached_framebuffer_width = width;
  cached_framebuffer_height = height;
  context.framebuffer_size_generation++;

  NS_INFO("Vulkan renderer backend->resized: w/h/gen: %i/%i/%llu", width,
          height, context.framebuffer_size_generation);
}

bool renderer_backend_begin_frame(renderer_backend *backend, f32 delta_time) {
  context.frame_delta_time = delta_time;
  if (context.recreating_swapchain) {
    VkResult result = vkDeviceWaitIdle(context.device);
    if (!result_is_success(result)) {
      NS_ERROR("vulkan_renderer_backend_begin_frame vkDeviceWaitIdle (1) "
               "failed: '%s'",
               result_string(result, true));
      return false;
    }
    NS_INFO("Recreating swapchain, booting.");
    return false;
  }

  if (context.framebuffer_size_generation !=
      context.framebuffer_size_last_generation) {
    VkResult result = vkDeviceWaitIdle(context.device);
    if (!result_is_success(result)) {
      NS_ERROR("vulkan_renderer_backend_begin_frame vkDeviceWaitIdle (2) "
               "failed: '%s'",
               result_string(result, true));
      return false;
    }

    if (!recreate_swapchain(backend)) {
      return false;
    }

    NS_INFO("Resized. booting.");
    return false;
  }

  if (!fence_wait(&context, &context.in_flight_fences[context.current_frame],
                  UINT64_MAX)) {
    NS_WARN("In-flight fence wait failure!");
  }

  if (!swapchain_acquire_next_image_index(
          &context, &context.swapchain, UINT64_MAX,
          context.image_available_semaphores[context.current_frame], 0,
          &context.image_index)) {
    return false;
  }

  CommandBuffer *command_buffer =
      &context.graphics_command_buffers[context.image_index];
  command_buffer_reset(command_buffer);
  command_buffer_begin(command_buffer, false, false, false);

  const u32 framebuffer_width =
      context.device.swapchain_support.capabilities.currentExtent.width;
  const u32 framebuffer_height =
      context.device.swapchain_support.capabilities.currentExtent.height;

  VkViewport viewport;
  viewport.x = 0.0f;
  viewport.y = static_cast<f32>(framebuffer_height);
  viewport.width = static_cast<f32>(framebuffer_width);
  viewport.height = -static_cast<f32>(framebuffer_height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor;
  scissor.offset.x = scissor.offset.y = 0;
  scissor.extent.width = framebuffer_width;
  scissor.extent.height = framebuffer_height;

  vkCmdSetViewport(command_buffer->handle, 0, 1, &viewport);
  vkCmdSetScissor(command_buffer->handle, 0, 1, &scissor);

  context.main_renderpass.w = framebuffer_width;
  context.main_renderpass.h = framebuffer_height;

  renderpass_begin(command_buffer, &context.main_renderpass,
                   context.swapchain.framebuffers[context.image_index].handle);

  return true;
}

void renderer_update_global_state(mat4 projection, mat4 view,
                                  vec3 /*view_position*/,
                                  vec4 /*ambient_color*/, i32 /*mode*/) {
  material_shader_use(&context, &context.material_shader);

  context.material_shader.global_ubo.projection = projection;
  context.material_shader.global_ubo.view = view;

  material_shader_update_global_state(&context, &context.material_shader,
                                      context.frame_delta_time);
}

bool renderer_backend_end_frame(renderer_backend * /* backend */,
                                f32 /* delta_time */) {
  CommandBuffer *command_buffer =
      &context.graphics_command_buffers[context.image_index];

  renderpass_end(command_buffer, &context.main_renderpass);

  command_buffer_end(command_buffer);

  if (context.images_in_flight[context.image_index] != nullptr) {
    fence_wait(&context, context.images_in_flight[context.image_index],
               UINT64_MAX);
  }

  context.images_in_flight[context.image_index] =
      &context.in_flight_fences[context.current_frame];

  fence_reset(&context, &context.in_flight_fences[context.current_frame]);

  VkSubmitInfo submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffer->handle;
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores =
      &context.queue_complete_semaphores[context.current_frame];
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores =
      &context.image_available_semaphores[context.current_frame];
  VkPipelineStageFlags flags[1] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submit_info.pWaitDstStageMask = flags;

  VkResult result =
      vkQueueSubmit(context.device.graphics_queue, 1, &submit_info,
                    context.in_flight_fences[context.current_frame].handle);
  if (result != VK_SUCCESS) {
    NS_ERROR("vkQueueSubmit failed with result '%s'",
             result_string(result, true));
    return false;
  }

  command_buffer_update_submitted(command_buffer);

  swapchain_present(&context, &context.swapchain, context.device.graphics_queue,
                    context.device.present_queue,
                    context.queue_complete_semaphores[context.current_frame],
                    context.image_index);

  return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                  VkDebugUtilsMessageTypeFlagsEXT /* message_types */,
                  const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                  ptr /* user_data */) {
  switch (message_severity) {
  default:
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
    NS_ERROR(callback_data->pMessage);
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
    NS_WARN(callback_data->pMessage);
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
    NS_INFO(callback_data->pMessage);
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
    NS_TRACE(callback_data->pMessage);
    break;
  }
  return VK_FALSE;
}

i32 find_memory_index(u32 type_filter, u32 property_flags) {
  VkPhysicalDeviceMemoryProperties memory_properties;
  vkGetPhysicalDeviceMemoryProperties(context.device, &memory_properties);

  for (u32 i = 0; i < memory_properties.memoryTypeCount; i++) {
    if (type_filter & (1 << i) &&
        (memory_properties.memoryTypes[i].propertyFlags & property_flags) ==
            property_flags) {
      return i;
    }
  }

  NS_WARN("Unable to find suitable memory type!");
  return -1;
}

void create_command_buffers(renderer_backend * /*backend*/) {
  context.graphics_command_buffers.resize(context.swapchain.image_count);
  for (u32 i = 0; i < context.swapchain.image_count; i++) {
    if (context.graphics_command_buffers[i].handle) {
      command_buffer_free(&context, context.device.graphics_command_pool,
                          &context.graphics_command_buffers[i]);
    }
    command_buffer_allocate(&context, context.device.graphics_command_pool,
                            true, &context.graphics_command_buffers[i]);
  }

  NS_DEBUG("Vulkan command buffers created.");
}

void regenerate_framebuffers(renderer_backend * /*backend*/,
                             Swapchain *swapchain, Renderpass *renderpass) {
  for (u32 i = 0; i < swapchain->image_count; i++) {
    u32 attachment_count = 2;
    VkImageView attachments[] = {
        swapchain->views[i],
        swapchain->depth_attachment.view,
    };

    framebuffer_create(
        &context, renderpass,
        context.device.swapchain_support.capabilities.currentExtent.width,
        context.device.swapchain_support.capabilities.currentExtent.height,
        attachment_count, attachments, &context.swapchain.framebuffers[i]);
  }
}

bool recreate_swapchain(renderer_backend *backend) {
  if (context.recreating_swapchain) {
    NS_DEBUG("recreate_swapchain called when already recreating. Booting.");
    return false;
  }

  // XXX: context.framebuffer_width/height is not working ?
  // if (context.framebuffer_width == 0 || context.framebuffer_height == 0) {
  //   NS_DEBUG("recreate_swapchain called when window is < 1 in a dimension. "
  //            "Booting.");
  //   return false;
  // }

  context.recreating_swapchain = true;
  vkDeviceWaitIdle(context.device);

  for (u32 i = 0; i < context.swapchain.image_count; i++) {
    context.images_in_flight[i] = nullptr;
  }

  device_query_swapchain_support(context.device, context.surface,
                                 &context.device.swapchain_support);
  device_detect_depth_format(&context.device);

  swapchain_recreate(&context, cached_framebuffer_width,
                     cached_framebuffer_height, &context.swapchain);

  u32 framebuffer_width = context.device.swapchain_support.capabilities
                              .currentExtent.width; // cached_framebuffer_width;
  u32 framebuffer_height =
      context.device.swapchain_support.capabilities.currentExtent
          .height; // cached_framebuffer_height;
  context.main_renderpass.x = 0;
  context.main_renderpass.y = 0;
  context.main_renderpass.w = framebuffer_width;
  context.main_renderpass.h = framebuffer_height;
  cached_framebuffer_width = 0;
  cached_framebuffer_height = 0;

  context.framebuffer_size_last_generation =
      context.framebuffer_size_generation;

  for (u32 i = 0; i < context.swapchain.image_count; i++) {
    command_buffer_free(&context, context.device.graphics_command_pool,
                        &context.graphics_command_buffers[i]);
    framebuffer_destroy(&context, &context.swapchain.framebuffers[i]);
  }

  regenerate_framebuffers(backend, &context.swapchain,
                          &context.main_renderpass);

  create_command_buffers(backend);

  context.recreating_swapchain = false;

  return true;
}

bool create_buffers(Context *context) {
  VkMemoryPropertyFlagBits memory_properties_flags =
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

  const usize vertex_buffer_size = sizeof(vertex_3d) * 1024 * 1024;
  if (!buffer_create(
          context, vertex_buffer_size,
          static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                             VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT),
          memory_properties_flags, true, &context->object_vertex_buffer)) {
    NS_ERROR("Error creating vertex buffer.");
    return false;
  }
  context->geometry_vertex_offset = 0;

  const usize index_buffer_size = sizeof(u32) * 1024 * 1024;
  if (!buffer_create(
          context, index_buffer_size,
          static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                             VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT),
          memory_properties_flags, true, &context->object_index_buffer)) {
    NS_ERROR("Error creating index buffer.");
    return false;
  }
  context->geometry_index_offset = 0;

  return true;
}

// temp
void renderer_update_object(geometry_render_data data) {
  material_shader_update_object(&context, &context.material_shader, data);
  CommandBuffer *command_buffer =
      &context.graphics_command_buffers[context.image_index];

  // TODO(ClementChambard): Temporary code
  material_shader_use(&context, &context.material_shader);
  VkDeviceSize offsets[1] = {0};
  vkCmdBindVertexBuffers(command_buffer->handle, 0, 1,
                         &context.object_vertex_buffer.handle, offsets);
  vkCmdBindIndexBuffer(command_buffer->handle,
                       context.object_index_buffer.handle, 0,
                       VK_INDEX_TYPE_UINT32);
  vkCmdDrawIndexed(command_buffer->handle, 6, 1, 0, 0, 0);
  // TODO(ClementChambard): Temporary code end
}

void renderer_create_texture(cstr /*name*/, i32 width, i32 height,
                             i32 channel_count, robytes pixels,
                             bool has_transparency, Texture *out_texture) {
  out_texture->width = width;
  out_texture->height = height;
  out_texture->channel_count = channel_count;
  out_texture->generation = INVALID_ID;

  // TODO(ClementChambard): Use an allocator for this
  out_texture->internal_data = ns::alloc(sizeof(TextureData), mem_tag::TEXTURE);
  TextureData *texture_data =
      reinterpret_cast<TextureData *>(out_texture->internal_data);

  VkDeviceSize image_size = width * height * channel_count;
  VkFormat image_format = VK_FORMAT_R8G8B8A8_UNORM;
  VkBufferUsageFlagBits usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  VkMemoryPropertyFlags memory_prop_flags =
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  Buffer staging;
  buffer_create(&context, image_size, usage, memory_prop_flags, true, &staging);
  buffer_load_data(&context, &staging, 0, image_size, 0, pixels);

  image_create(
      &context, VK_IMAGE_TYPE_2D, width, height, image_format,
      VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
          VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true, VK_IMAGE_ASPECT_COLOR_BIT,
      &texture_data->image);

  CommandBuffer tmp_buf;
  VkCommandPool pool = context.device.graphics_command_pool;
  VkQueue queue = context.device.graphics_queue;
  command_buffer_allocate_and_begin_single_use(&context, pool, &tmp_buf);

  image_transition_layout(&context, &tmp_buf, &texture_data->image,
                          image_format, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  image_copy_from_buffer(&context, &texture_data->image, staging, &tmp_buf);

  image_transition_layout(&context, &tmp_buf, &texture_data->image,
                          image_format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  command_buffer_end_single_use(&context, pool, &tmp_buf, queue);

  buffer_destroy(&context, &staging);

  VkSamplerCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  info.magFilter = VK_FILTER_LINEAR;
  info.minFilter = VK_FILTER_LINEAR;
  info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  info.anisotropyEnable = VK_TRUE;
  info.maxAnisotropy = 16;
  info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  info.unnormalizedCoordinates = VK_FALSE;
  info.compareEnable = VK_FALSE;
  info.compareOp = VK_COMPARE_OP_ALWAYS;
  info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  info.mipLodBias = 0.0f;
  info.minLod = 0.0f;
  info.maxLod = 0.0f;
  VkResult result = vkCreateSampler(context.device, &info, context.allocator,
                                    &texture_data->sampler);

  if (result != VK_SUCCESS) {
    NS_ERROR("Error creating texture sampler: %s", result_string(result, true));
    return;
  }

  out_texture->has_transparency = has_transparency;
  out_texture->generation++;
}

void renderer_destroy_texture(Texture *texture) {
  vkDeviceWaitIdle(context.device);

  TextureData *texture_data =
      reinterpret_cast<TextureData *>(texture->internal_data);

  if (!texture_data) {
    return;
  }

  image_destroy(&context, &texture_data->image);
  mem_zero(&texture_data->image, sizeof(Image));
  vkDestroySampler(context.device, texture_data->sampler, context.allocator);
  texture_data->sampler = VK_NULL_HANDLE;

  ns::free(texture->internal_data, sizeof(TextureData), mem_tag::TEXTURE);
  mem_zero(texture, sizeof(Texture));
}

} // namespace ns::vulkan
