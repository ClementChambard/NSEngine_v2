#include "./vulkan_backend.h"

#include "../../containers/vec.h"
#include "../../core/application.h"
#include "../../core/logger.h"
#include "../../core/string.h"
#include "../../math/math.h"
#include "./shaders/vulkan_material_shader.h"
#include "./shaders/vulkan_ui_shader.h"
#include "./vulkan_buffer.h"
#include "./vulkan_command_buffer.h"
#include "./vulkan_device.h"
#include "./vulkan_image.h"
#include "./vulkan_platform.h"
#include "./vulkan_renderpass.h"
#include "./vulkan_swapchain.h"
#include "./vulkan_types.inl"
#include "./vulkan_utils.h"

#include "../../systems/material_system.h"

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
void regenerate_framebuffers();
bool recreate_swapchain(renderer_backend *backend);

void upload_data_range(Context *context, VkCommandPool pool, VkFence fence,
                       VkQueue queue, Buffer *buffer, usize offset, usize size,
                       roptr data) {
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

void free_data_range(Buffer * /*buffer*/, u64 /*offset*/, u64 /*size*/) {}

bool backend_initialize(renderer_backend *backend, cstr application_name) {
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

  Vec<cstr> required_extensions{};
  required_extensions.push(VK_KHR_SURFACE_EXTENSION_NAME);
  platform::get_required_extension_names(&required_extensions);

  Vec<cstr> required_validation_layers{};

#if defined(_DEBUG)
  required_extensions.push(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

  NS_DEBUG("Required extensions: ");
  for (auto &s : required_extensions) {
    NS_DEBUG("- %s", s);
  }

  NS_INFO("Validation layers enabled. Enumerating...");
  required_validation_layers.push("VK_LAYER_KHRONOS_validation");
  // Dump all vk calls
  // required_validation_layers.push("VK_LAYER_LUNARG_api_dump");

  u32 available_layer_count = 0;
  VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, nullptr));
  Vec<VkLayerProperties> available_layers(available_layer_count);
  VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count,
                                              available_layers));

  for (usize i = 0; i < required_validation_layers.len(); i++) {
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
  create_info.enabledExtensionCount = required_extensions.len();
  create_info.ppEnabledExtensionNames = required_extensions;
  create_info.enabledLayerCount = required_validation_layers.len();
  create_info.ppEnabledLayerNames = required_validation_layers;

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
  if (!platform::create_vulkan_surface(&context)) {
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

  renderpass_create(&context, &context.main_renderpass,
                    {0.0f, 0.0f, static_cast<f32>(framebuffer_width),
                     static_cast<f32>(framebuffer_height)},
                    {0.0f, 0.0f, 0.2f, 1.0f}, 1.0f, 0, ClearFlags::ALL, false,
                    true);

  renderpass_create(&context, &context.ui_renderpass,
                    {0.0f, 0.0f, static_cast<f32>(framebuffer_width),
                     static_cast<f32>(framebuffer_height)},
                    {}, 1.0f, 0, ClearFlags::NONE, true, false);

  regenerate_framebuffers();

  create_command_buffers(backend);

  context.image_available_semaphores.resize(
      context.swapchain.max_frames_in_flight);
  context.queue_complete_semaphores.resize(
      context.swapchain.max_frames_in_flight);

  for (u8 i = 0; i < context.swapchain.max_frames_in_flight; i++) {
    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(context.device, &semaphore_info, context.allocator,
                      &context.image_available_semaphores[i]);
    vkCreateSemaphore(context.device, &semaphore_info, context.allocator,
                      &context.queue_complete_semaphores[i]);
    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    VK_CHECK(vkCreateFence(context.device, &fence_info, context.allocator,
                           &context.in_flight_fences[i]));
  }

  for (u32 i = 0; i < context.swapchain.image_count; i++) {
    context.images_in_flight[i] = nullptr;
  }

  if (!material_shader_create(&context, &context.material_shader)) {
    NS_ERROR("Error loading built-in basic_lighting shader.");
    return false;
  }

  if (!ui_shader_create(&context, &context.ui_shader)) {
    NS_ERROR("Error loading built-in ui shader.");
    return false;
  }

  create_buffers(&context);

  for (u32 i = 0; i < Context::MAX_GEOMETRY_COUNT; i++) {
    context.geometries[i].id = INVALID_ID;
  }

  NS_INFO("Vulkan renderer initialized successfully.");
  return true;
}

void backend_shutdown(renderer_backend * /* backend */) {
  vkDeviceWaitIdle(context.device);

  buffer_destroy(&context, &context.object_vertex_buffer);
  buffer_destroy(&context, &context.object_index_buffer);

  ui_shader_destroy(&context, &context.ui_shader);
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
    vkDestroyFence(context.device, context.in_flight_fences[i],
                   context.allocator);
  }
  context.image_available_semaphores.free();
  context.queue_complete_semaphores.free();

  for (u32 i = 0; i < context.swapchain.image_count; i++) {
    if (context.graphics_command_buffers[i].handle) {
      command_buffer_free(&context, context.device.graphics_command_pool,
                          &context.graphics_command_buffers[i]);
    }
  }
  context.graphics_command_buffers.free();

  for (u32 i = 0; i < context.swapchain.image_count; i++) {
    vkDestroyFramebuffer(context.device, context.world_framebuffers[i],
                         context.allocator);
    vkDestroyFramebuffer(context.device, context.swapchain.framebuffers[i],
                         context.allocator);
  }

  renderpass_destroy(&context, &context.ui_renderpass);
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

void backend_on_resized(renderer_backend * /* backend */, u16 width,
                        u16 height) {
  cached_framebuffer_width = width;
  cached_framebuffer_height = height;
  context.framebuffer_size_generation++;

  NS_INFO("Vulkan renderer backend->resized: w/h/gen: %i/%i/%llu", width,
          height, context.framebuffer_size_generation);
}

bool backend_begin_frame(renderer_backend *backend, f32 delta_time) {
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

  VkResult result = vkWaitForFences(
      context.device, 1, &context.in_flight_fences[context.current_frame], true,
      UINT64_MAX);
  if (!result_is_success(result)) {
    NS_ERROR("in flight fence wait failure! error: '%s'",
             result_string(result, true));
    return false;
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

  context.main_renderpass.render_area.z = framebuffer_width;
  context.main_renderpass.render_area.w = framebuffer_height;
  context.ui_renderpass.render_area.z = framebuffer_width;
  context.ui_renderpass.render_area.w = framebuffer_height;

  return true;
}

void backend_update_global_world_state(mat4 projection, mat4 view,
                                       vec3 /*view_position*/,
                                       vec4 /*ambient_color*/, i32 /*mode*/) {
  material_shader_use(&context, &context.material_shader);

  context.material_shader.global_ubo.projection = projection;
  context.material_shader.global_ubo.view = view;

  material_shader_update_global_state(&context, &context.material_shader,
                                      context.frame_delta_time);
}

void backend_update_global_ui_state(mat4 projection, mat4 view, i32 /*mode*/) {
  ui_shader_use(&context, &context.ui_shader);

  context.ui_shader.global_ubo.projection = projection;
  context.ui_shader.global_ubo.view = view;

  ui_shader_update_global_state(&context, &context.ui_shader,
                                context.frame_delta_time);
}

bool backend_end_frame(renderer_backend * /* backend */, f32 /* delta_time */) {
  CommandBuffer *command_buffer =
      &context.graphics_command_buffers[context.image_index];

  command_buffer_end(command_buffer);

  if (context.images_in_flight[context.image_index] != nullptr) {
    VkResult result = vkWaitForFences(
        context.device, 1, context.images_in_flight[context.image_index], true,
        UINT64_MAX);
    if (!result_is_success(result)) {
      NS_FATAL("vkWaitForFences - error: '%s'", result_string(result, true));
      return false;
    }
  }

  context.images_in_flight[context.image_index] =
      &context.in_flight_fences[context.current_frame];

  VK_CHECK(vkResetFences(context.device, 1,
                         &context.in_flight_fences[context.current_frame]));

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
                    context.in_flight_fences[context.current_frame]);
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

void regenerate_framebuffers() {
  for (u32 i = 0; i < context.swapchain.image_count; i++) {
    VkImageView world_attachments[2] = {
        context.swapchain.views[i], context.swapchain.depth_attachment.view};

    VkFramebufferCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.renderPass = context.main_renderpass;
    info.attachmentCount = 2;
    info.pAttachments = world_attachments;
    info.width =
        context.device.swapchain_support.capabilities.currentExtent.width;
    info.height =
        context.device.swapchain_support.capabilities.currentExtent.height;
    info.layers = 1;

    VK_CHECK(vkCreateFramebuffer(context.device, &info, context.allocator,
                                 &context.world_framebuffers[i]));

    VkImageView ui_attachments[1] = {
        context.swapchain.views[i],
    };

    VkFramebufferCreateInfo depth_info{};
    depth_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    depth_info.renderPass = context.ui_renderpass;
    depth_info.attachmentCount = 1;
    depth_info.pAttachments = ui_attachments;
    depth_info.width =
        context.device.swapchain_support.capabilities.currentExtent.width;
    depth_info.height =
        context.device.swapchain_support.capabilities.currentExtent.height;
    depth_info.layers = 1;

    VK_CHECK(vkCreateFramebuffer(context.device, &depth_info, context.allocator,
                                 &context.swapchain.framebuffers[i]));
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

  f32 framebuffer_width =
      static_cast<f32>(context.device.swapchain_support.capabilities
                           .currentExtent.width); // cached_framebuffer_width;
  f32 framebuffer_height =
      static_cast<f32>(context.device.swapchain_support.capabilities
                           .currentExtent.height); // cached_framebuffer_height;
  context.main_renderpass.render_area = {0, 0, framebuffer_width,
                                         framebuffer_height};
  context.ui_renderpass.render_area = {0, 0, framebuffer_width,
                                       framebuffer_height};
  cached_framebuffer_width = 0;
  cached_framebuffer_height = 0;

  context.framebuffer_size_last_generation =
      context.framebuffer_size_generation;

  for (u32 i = 0; i < context.swapchain.image_count; i++) {
    command_buffer_free(&context, context.device.graphics_command_pool,
                        &context.graphics_command_buffers[i]);
    vkDestroyFramebuffer(context.device, context.world_framebuffers[i],
                         context.allocator);
    vkDestroyFramebuffer(context.device, context.swapchain.framebuffers[i],
                         context.allocator);
  }

  regenerate_framebuffers();

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

void backend_create_texture(robytes pixels, Texture *texture) {
  // TODO(ClementChambard): Use an allocator for this
  texture->internal_data = ns::alloc(sizeof(TextureData), MemTag::TEXTURE);
  TextureData *texture_data =
      reinterpret_cast<TextureData *>(texture->internal_data);

  VkDeviceSize image_size =
      texture->width * texture->height * texture->channel_count;
  VkFormat image_format = VK_FORMAT_R8G8B8A8_UNORM;
  VkBufferUsageFlagBits usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  VkMemoryPropertyFlags memory_prop_flags =
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  Buffer staging;
  buffer_create(&context, image_size, usage, memory_prop_flags, true, &staging);
  buffer_load_data(&context, &staging, 0, image_size, 0, pixels);

  image_create(
      &context, VK_IMAGE_TYPE_2D, texture->width, texture->height, image_format,
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

  texture->generation++;
}

void backend_destroy_texture(Texture *texture) {
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

  ns::free(texture->internal_data, sizeof(TextureData), MemTag::TEXTURE);
  mem_zero(texture, sizeof(Texture));
}

bool backend_create_material(Material *material) {
  if (!material) {
    NS_ERROR("vulkan::renderer_create_material - material is null");
    return false;
  }
  switch (material->type) {
  case MaterialType::WORLD:
    if (!material_shader_acquire_resources(&context, &context.material_shader,
                                           material)) {
      NS_ERROR("vulkan::renderer_create_material - "
               "material_shader_acquire_resources failed");
      return false;
    }
    break;
  case MaterialType::UI:
    if (!ui_shader_acquire_resources(&context, &context.ui_shader, material)) {
      NS_ERROR("vulkan::renderer_create_material - "
               "ui_shader_acquire_resources failed");
      return false;
    }
    break;
  default:
    NS_ERROR("vulkan::renderer_create_material - unknown material type");
    return false;
  }
  NS_TRACE("Renderer: material created.");
  return true;
}

void backend_destroy_material(Material *material) {
  if (!material) {
    NS_WARN("vulkan::renderer_destroy_material - material is null");
    return;
  }
  if (material->internal_id == INVALID_ID) {
    NS_WARN(
        "vulkan::renderer_destroy_material - material has invalid internal id");
    return;
  }

  switch (material->type) {
  case MaterialType::WORLD:
    material_shader_release_resources(&context, &context.material_shader,
                                      material);
    break;
  case MaterialType::UI:
    ui_shader_release_resources(&context, &context.ui_shader, material);
    break;
  default:
    NS_WARN("vulkan::renderer_destroy_material - unknown material type");
    return;
  }
}

bool backend_create_geometry(Geometry *geometry, u32 vertex_size,
                             u32 vertex_count, roptr vertices, u32 index_size,
                             u32 index_count, roptr indices) {
  if (!vertex_count || !vertices) {
    NS_ERROR("vulkan::renderer_create_geometry - vertex_count or vertices "
             "is null");
    return false;
  }

  bool is_reupload = geometry->internal_id != INVALID_ID;
  GeometryData old_range;

  GeometryData *internal_data = nullptr;
  if (is_reupload) {
    internal_data = &context.geometries[geometry->internal_id];

    old_range.index_buffer_offset = internal_data->index_buffer_offset;
    old_range.index_count = internal_data->index_count;
    old_range.vertex_buffer_offset = internal_data->vertex_buffer_offset;
    old_range.vertex_count = internal_data->vertex_count;
    old_range.index_element_size = internal_data->index_element_size;
    old_range.vertex_element_size = internal_data->vertex_element_size;
  } else {
    for (u32 i = 0; i < Context::MAX_GEOMETRY_COUNT; i++) {
      if (context.geometries[i].id == INVALID_ID) {
        geometry->internal_id = i;
        context.geometries[i].id = i;
        internal_data = &context.geometries[i];
        break;
      }
    }
  }
  if (!internal_data) {
    NS_ERROR("vulkan::renderer_create_geometry - failed to find free geometry "
             "index");
    return false;
  }

  VkCommandPool pool = context.device.graphics_command_pool;
  VkQueue queue = context.device.graphics_queue;

  internal_data->vertex_buffer_offset = context.geometry_vertex_offset;
  internal_data->vertex_count = vertex_count;
  internal_data->vertex_element_size = vertex_size;
  u32 total_size = vertex_size * vertex_count;
  upload_data_range(&context, pool, 0, queue, &context.object_vertex_buffer,
                    internal_data->vertex_buffer_offset, total_size, vertices);
  context.geometry_vertex_offset += total_size;

  if (index_count && indices) {
    internal_data->index_buffer_offset = context.geometry_index_offset;
    internal_data->index_count = index_count;
    internal_data->index_element_size = index_size;
    total_size = index_size * index_count;
    upload_data_range(&context, pool, 0, queue, &context.object_index_buffer,
                      internal_data->index_buffer_offset, total_size, indices);
    context.geometry_index_offset += total_size;
  }

  if (internal_data->generation == INVALID_ID) {
    internal_data->generation = 0;
  } else {
    internal_data->generation++;
  }

  if (is_reupload) {
    free_data_range(&context.object_vertex_buffer,
                    old_range.vertex_buffer_offset,
                    old_range.vertex_element_size * old_range.vertex_count);
    if (old_range.index_count > 0) {
      free_data_range(&context.object_index_buffer,
                      old_range.index_buffer_offset,
                      old_range.index_element_size * old_range.index_count);
    }
  }

  return true;
}

void backend_destroy_geometry(Geometry *geometry) {
  if (!geometry || geometry->internal_id == INVALID_ID) {
    return;
  }
  vkDeviceWaitIdle(context.device);
  GeometryData *internal_data = &context.geometries[geometry->internal_id];

  free_data_range(
      &context.object_vertex_buffer, internal_data->vertex_buffer_offset,
      internal_data->vertex_element_size * internal_data->vertex_count);
  if (internal_data->index_count > 0) {
    free_data_range(
        &context.object_index_buffer, internal_data->index_buffer_offset,
        internal_data->index_element_size * internal_data->index_count);
  }

  mem_zero(internal_data, sizeof(GeometryData));
  internal_data->generation = INVALID_ID;
  internal_data->id = INVALID_ID;
}

void backend_draw_geometry(geometry_render_data data) {
  if (!data.geometry || data.geometry->internal_id == INVALID_ID) {
    return;
  }

  GeometryData *buffer_data = &context.geometries[data.geometry->internal_id];
  VkCommandBuffer command_buffer =
      context.graphics_command_buffers[context.image_index];

  Material *m;
  if (data.geometry->material) {
    m = data.geometry->material;
  } else {
    m = material_system_get_default();
  }

  switch (m->type) {
  case MaterialType::WORLD:
    material_shader_set_model(&context, &context.material_shader, data.model);
    material_shader_apply_material(&context, &context.material_shader, m);
    break;
  case MaterialType::UI:
    ui_shader_set_model(&context, &context.ui_shader, data.model);
    ui_shader_apply_material(&context, &context.ui_shader, m);
    break;
  default:
    NS_ERROR("Unknown material type");
    break;
  }

  VkDeviceSize offsets[1] = {buffer_data->vertex_buffer_offset};
  vkCmdBindVertexBuffers(command_buffer, 0, 1,
                         &context.object_vertex_buffer.handle, offsets);

  if (buffer_data->index_count > 0) {
    vkCmdBindIndexBuffer(command_buffer, context.object_index_buffer.handle,
                         buffer_data->index_buffer_offset,
                         VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(command_buffer, buffer_data->index_count, 1, 0, 0, 0);
  } else {
    vkCmdDraw(command_buffer, buffer_data->vertex_count, 1, 0, 0);
  }
}

bool backend_begin_renderpass(renderer_backend * /*backend*/,
                              u8 renderpass_id) {
  Renderpass *renderpass = nullptr;
  VkFramebuffer framebuffer = VK_NULL_HANDLE;
  CommandBuffer *command_buffer =
      &context.graphics_command_buffers[context.image_index];

  switch (static_cast<builtin_renderpass>(renderpass_id)) {
  case builtin_renderpass::WORLD:
    renderpass = &context.main_renderpass;
    framebuffer = context.world_framebuffers[context.image_index];
    break;
  case builtin_renderpass::UI:
    renderpass = &context.ui_renderpass;
    framebuffer = context.swapchain.framebuffers[context.image_index];
    break;
  }

  renderpass_begin(command_buffer, renderpass, framebuffer);

  // use appropriate shader
  switch (static_cast<builtin_renderpass>(renderpass_id)) {
  case builtin_renderpass::WORLD:
    material_shader_use(&context, &context.material_shader);
    break;
  case builtin_renderpass::UI:
    ui_shader_use(&context, &context.ui_shader);
    break;
  }

  return true;
}

bool backend_end_renderpass(renderer_backend * /*backend*/, u8 renderpass_id) {
  CommandBuffer *command_buffer =
      &context.graphics_command_buffers[context.image_index];
  switch (static_cast<builtin_renderpass>(renderpass_id)) {
  case builtin_renderpass::WORLD:
    renderpass_end(command_buffer, &context.main_renderpass);
    break;
  case builtin_renderpass::UI:
    renderpass_end(command_buffer, &context.ui_renderpass);
    break;
  }
  return true;
}

} // namespace ns::vulkan
