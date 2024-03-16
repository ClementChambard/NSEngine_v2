#include "./vulkan_device.h"

#include "../../containers/vec.h"
#include "../../core/logger.h"
#include "../../core/memory.h"
#include "../../core/string.h"

namespace ns::vulkan {

struct PhysicalDeviceRequirements {
  bool graphics;
  bool present;
  bool compute;
  bool transfer;
  Vec<cstr> device_extension_names;
  bool sampler_anisotropy;
  bool discrete_gpu;
};

struct PhysicalDeviceQueueFamilyInfo {
  u32 graphics_family_index;
  u32 present_family_index;
  u32 compute_family_index;
  u32 transfer_family_index;
};

static bool select_physical_device(Context *context);
static bool physical_device_meets_requirements(
    VkPhysicalDevice device, VkSurfaceKHR surface,
    VkPhysicalDeviceProperties const *properties,
    VkPhysicalDeviceFeatures const *features,
    PhysicalDeviceRequirements const *requirements,
    PhysicalDeviceQueueFamilyInfo *out_queue_family_info,
    SwapchainSupportInfo *out_swapchain_support);

bool device_create(Context *context) {
  if (!select_physical_device(context)) {
    return false;
  }

  NS_INFO("Creating logical device...");

  bool present_shares_graphics_queue = context->device.graphics_queue_index ==
                                       context->device.present_queue_index;
  bool transfer_shares_graphics_queue = context->device.transfer_queue_index ==
                                        context->device.graphics_queue_index;
  u32 index_count = 1;
  if (!present_shares_graphics_queue) {
    index_count++;
  }
  if (!transfer_shares_graphics_queue) {
    index_count++;
  }
  u32 indices[32];
  u8 index = 0;
  indices[index++] = context->device.graphics_queue_index;
  if (!present_shares_graphics_queue) {
    indices[index++] = context->device.present_queue_index;
  }
  if (!transfer_shares_graphics_queue) {
    indices[index++] = context->device.transfer_queue_index;
  }

  VkDeviceQueueCreateInfo queue_create_infos[32];
  for (u32 i = 0; i < index_count; i++) {
    queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_infos[i].queueFamilyIndex = indices[i];
    queue_create_infos[i].queueCount = 1;
    // if (static_cast<i32>(indices[i]) == context->device.graphics_queue_index)
    // {
    //   queue_create_infos[i].queueCount = 2;
    // }
    queue_create_infos[i].flags = 0;
    queue_create_infos[i].pNext = nullptr;
    f32 queue_priority = 1.0f;
    queue_create_infos[i].pQueuePriorities = &queue_priority;
  }

  VkPhysicalDeviceFeatures device_features{};
  device_features.samplerAnisotropy = VK_TRUE;

  VkDeviceCreateInfo device_create_info{};
  device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_create_info.queueCreateInfoCount = index_count;
  device_create_info.pQueueCreateInfos = queue_create_infos;
  device_create_info.pEnabledFeatures = &device_features;
  device_create_info.enabledExtensionCount = 1;
  cstr extension_names = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
  device_create_info.ppEnabledExtensionNames = &extension_names;
  device_create_info.enabledLayerCount = 0;
  device_create_info.ppEnabledLayerNames = nullptr;

  VK_CHECK(vkCreateDevice(context->device, &device_create_info,
                          context->allocator, &context->device.logical_device));

  NS_INFO("Logical device created.");

  vkGetDeviceQueue(context->device, context->device.graphics_queue_index, 0,
                   &context->device.graphics_queue);

  vkGetDeviceQueue(context->device, context->device.present_queue_index, 0,
                   &context->device.present_queue);

  vkGetDeviceQueue(context->device, context->device.transfer_queue_index, 0,
                   &context->device.transfer_queue);

  NS_INFO("Queues obtained.");

  VkCommandPoolCreateInfo pool_create_info{};
  pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_create_info.queueFamilyIndex = context->device.graphics_queue_index;
  pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  VK_CHECK(vkCreateCommandPool(context->device, &pool_create_info,
                               context->allocator,
                               &context->device.graphics_command_pool));

  NS_INFO("Graphics command pool created.");

  return true;
}

void device_destroy(Context *context) {
  context->device.graphics_queue = VK_NULL_HANDLE;
  context->device.present_queue = VK_NULL_HANDLE;
  context->device.transfer_queue = VK_NULL_HANDLE;

  NS_INFO("Destroying command pools...");
  vkDestroyCommandPool(context->device, context->device.graphics_command_pool,
                       context->allocator);

  NS_INFO("Destroying logical device...");
  if (context->device.logical_device) {
    vkDestroyDevice(context->device, context->allocator);
    context->device.logical_device = VK_NULL_HANDLE;
  }

  NS_INFO("Releasing physical device resources...");
  context->device.physical_device = VK_NULL_HANDLE;

  if (context->device.swapchain_support.formats) {
    ns::free(context->device.swapchain_support.formats,
             sizeof(VkSurfaceFormatKHR) *
                 context->device.swapchain_support.format_count,
             MemTag::RENDERER);
    context->device.swapchain_support.format_count = 0;
    context->device.swapchain_support.formats = nullptr;
  }

  if (context->device.swapchain_support.present_modes) {
    ns::free(context->device.swapchain_support.present_modes,
             sizeof(VkPresentModeKHR) *
                 context->device.swapchain_support.present_mode_count,
             MemTag::RENDERER);
    context->device.swapchain_support.present_mode_count = 0;
    context->device.swapchain_support.present_modes = nullptr;
  }

  mem_zero(&context->device.swapchain_support.capabilities,
           sizeof(context->device.swapchain_support.capabilities));

  context->device.graphics_queue_index = -1;
  context->device.present_queue_index = -1;
  context->device.transfer_queue_index = -1;
}

void device_query_swapchain_support(VkPhysicalDevice physical_device,
                                    VkSurfaceKHR surface,
                                    SwapchainSupportInfo *out_support_info) {
  VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      physical_device, surface, &out_support_info->capabilities));

  VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
      physical_device, surface, &out_support_info->format_count, nullptr));
  if (out_support_info->format_count != 0) {
    if (!out_support_info->formats) {
      out_support_info->formats = reinterpret_cast<VkSurfaceFormatKHR *>(
          ns::alloc(sizeof(VkSurfaceFormatKHR) * out_support_info->format_count,
                    MemTag::RENDERER));
    }
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
        physical_device, surface, &out_support_info->format_count,
        out_support_info->formats));
  }

  VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
      physical_device, surface, &out_support_info->present_mode_count,
      nullptr));
  if (out_support_info->present_mode_count != 0) {
    if (!out_support_info->present_modes) {
      out_support_info->present_modes =
          reinterpret_cast<VkPresentModeKHR *>(ns::alloc(
              sizeof(VkPresentModeKHR) * out_support_info->present_mode_count,
              MemTag::RENDERER));
    }
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
        physical_device, surface, &out_support_info->present_mode_count,
        out_support_info->present_modes));
  }
}

bool device_detect_depth_format(Device *device) {
  const usize candidate_count = 3;
  VkFormat candidates[3] = {
      VK_FORMAT_D32_SFLOAT,
      VK_FORMAT_D32_SFLOAT_S8_UINT,
      VK_FORMAT_D24_UNORM_S8_UINT,
  };

  u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
  for (usize i = 0; i < candidate_count; i++) {
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(*device, candidates[i], &properties);

    if ((properties.linearTilingFeatures & flags) == flags) {
      device->depth_format = candidates[i];
      return true;
    } else if ((properties.optimalTilingFeatures & flags) == flags) {
      device->depth_format = candidates[i];
      return true;
    }
  }

  return false;
}

static bool select_physical_device(Context *context) {
  u32 physical_device_count = 0;
  VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physical_device_count,
                                      nullptr));
  if (physical_device_count == 0) {
    NS_FATAL("No devices which support Vulkan were found.");
    return false;
  }

  VkPhysicalDevice physical_devices[32];
  VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physical_device_count,
                                      physical_devices));
  for (u32 i = 0; i < physical_device_count; i++) {
    auto d = physical_devices[i];
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(d, &properties);

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(d, &features);

    VkPhysicalDeviceMemoryProperties memory;
    vkGetPhysicalDeviceMemoryProperties(d, &memory);

    bool supports_device_local_host_visible = false;
    for (u32 i = 0; i < memory.memoryTypeCount; i++) {
      if ((memory.memoryTypes[i].propertyFlags &
           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) &&
          (memory.memoryTypes[i].propertyFlags &
           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
        supports_device_local_host_visible = true;
        break;
      }
    }

    PhysicalDeviceRequirements requirements = {
        .graphics = true,
        .present = true,
        .compute = false, // enable this if compute will be required
        .transfer = true,
        .device_extension_names = {VK_KHR_SWAPCHAIN_EXTENSION_NAME},
        .sampler_anisotropy = true,
        .discrete_gpu = true,
    };

    PhysicalDeviceQueueFamilyInfo queue_info{};
    bool result = physical_device_meets_requirements(
        d, context->surface, &properties, &features, &requirements, &queue_info,
        &context->device.swapchain_support);

    if (result) {
      NS_INFO("Selected device: '%s'.", properties.deviceName);
      switch (properties.deviceType) {
      default:
      case VK_PHYSICAL_DEVICE_TYPE_OTHER:
        NS_INFO("GPU type is unknown.");
        break;
      case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        NS_INFO("GPU type is integrated.");
        break;
      case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        NS_INFO("GPU type is discrete.");
        break;
      case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        NS_INFO("GPU type is virtual.");
        break;
      case VK_PHYSICAL_DEVICE_TYPE_CPU:
        NS_INFO("GPU type is CPU.");
        break;
      }

      NS_INFO("GPU Driver version: %d.%d.%d",
              VK_VERSION_MAJOR(properties.driverVersion),
              VK_VERSION_MINOR(properties.driverVersion),
              VK_VERSION_PATCH(properties.driverVersion));

      NS_INFO("Vulkan API version: %d.%d.%d",
              VK_VERSION_MAJOR(properties.apiVersion),
              VK_VERSION_MINOR(properties.apiVersion),
              VK_VERSION_PATCH(properties.apiVersion));

      for (u32 i = 0; i < memory.memoryHeapCount; i++) {
        f32 memory_size_gib = static_cast<f32>(memory.memoryHeaps[i].size) /
                              1024.f / 1024.f / 1024.f;
        if (memory.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
          NS_INFO("Local GPU memory: %.2f GiB", memory_size_gib);
        } else {
          NS_INFO("Shared system memory: %.2f GiB", memory_size_gib);
        }
      }

      context->device.physical_device = d;
      context->device.graphics_queue_index = queue_info.graphics_family_index;
      context->device.present_queue_index = queue_info.present_family_index;
      context->device.transfer_queue_index = queue_info.transfer_family_index;
      // compute
      context->device.properties = properties;
      context->device.features = features;
      context->device.memory = memory;
      context->device.supports_device_local_host_visible =
          supports_device_local_host_visible;
      break;
    }
  }

  if (!context->device.physical_device) {
    NS_ERROR("No physical devices were found which met the requirements.");
    return false;
  }

  NS_INFO("Physical device selected.");
  return true;
}

static bool physical_device_meets_requirements(
    VkPhysicalDevice device, VkSurfaceKHR surface,
    VkPhysicalDeviceProperties const *properties,
    VkPhysicalDeviceFeatures const *features,
    PhysicalDeviceRequirements const *requirements,
    PhysicalDeviceQueueFamilyInfo *out_queue_info,
    SwapchainSupportInfo *out_swapchain_support) {
  out_queue_info->graphics_family_index = -1;
  out_queue_info->present_family_index = -1;
  out_queue_info->compute_family_index = -1;
  out_queue_info->transfer_family_index = -1;

  if (requirements->discrete_gpu) {
    if (properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      NS_INFO("Device is not a discrete GPU, and one is required. Skipping.");
      return false;
    }
  }

  u32 queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
                                           nullptr);
  VkQueueFamilyProperties queue_families[32];
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
                                           queue_families);

  NS_INFO("Graphics | Present | Compute | Transfer | Name");
  u8 min_transfer_score = 255;
  for (u32 i = 0; i < queue_family_count; i++) {
    u8 current_transfer_score = 0;

    if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      out_queue_info->graphics_family_index = i;
      current_transfer_score++;
    }

    if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
      out_queue_info->compute_family_index = i;
      current_transfer_score++;
    }

    if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
      if (current_transfer_score <= min_transfer_score) {
        min_transfer_score = current_transfer_score;
        out_queue_info->transfer_family_index = i;
      }
    }

    VkBool32 supports_present = VK_FALSE;
    VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface,
                                                  &supports_present));
    if (supports_present) {
      out_queue_info->present_family_index = i;
    }
  }
  NS_INFO("       %d |       %d |       %d |        %d | %s",
          out_queue_info->graphics_family_index != static_cast<u32>(-1),
          out_queue_info->present_family_index != static_cast<u32>(-1),
          out_queue_info->compute_family_index != static_cast<u32>(-1),
          out_queue_info->transfer_family_index != static_cast<u32>(-1),
          properties->deviceName);

  if ((requirements->graphics &&
       out_queue_info->graphics_family_index == static_cast<u32>(-1)) ||
      (requirements->present &&
       out_queue_info->present_family_index == static_cast<u32>(-1)) ||
      (requirements->compute &&
       out_queue_info->compute_family_index == static_cast<u32>(-1)) ||
      (requirements->transfer &&
       out_queue_info->transfer_family_index == static_cast<u32>(-1))) {
    return false;
  }

  NS_INFO("Device meets queue requirements.");
  NS_TRACE("Graphics Family Index: %i", out_queue_info->graphics_family_index);
  NS_TRACE("Present Family Index: %i", out_queue_info->present_family_index);
  NS_TRACE("Compute Family Index: %i", out_queue_info->compute_family_index);
  NS_TRACE("Transfer Family Index: %i", out_queue_info->transfer_family_index);

  device_query_swapchain_support(device, surface, out_swapchain_support);

  if (out_swapchain_support->format_count < 1 ||
      out_swapchain_support->present_mode_count < 1) {
    if (out_swapchain_support->formats) {
      ns::free(out_swapchain_support->formats,
               sizeof(VkSurfaceFormatKHR) * out_swapchain_support->format_count,
               MemTag::RENDERER);
    }
    if (out_swapchain_support->present_modes) {
      ns::free(out_swapchain_support->present_modes,
               sizeof(VkPresentModeKHR) *
                   out_swapchain_support->present_mode_count,
               MemTag::RENDERER);
    }
    NS_INFO("Required swapchain support not present. skipping device.");
    return false;
  }

  u32 available_extension_count = 0;
  VkExtensionProperties *available_extensions = 0;
  VK_CHECK(vkEnumerateDeviceExtensionProperties(
      device, 0, &available_extension_count, nullptr));
  if (available_extension_count != 0) {
    available_extensions = reinterpret_cast<VkExtensionProperties *>(
        ns::alloc(sizeof(VkExtensionProperties) * available_extension_count,
                  MemTag::RENDERER));
    VK_CHECK(vkEnumerateDeviceExtensionProperties(
        device, 0, &available_extension_count, available_extensions));

    for (auto &re : requirements->device_extension_names) {
      bool found = false;
      for (u32 i = 0; i < available_extension_count; i++) {
        if (string_eq(re, available_extensions[i].extensionName)) {
          found = true;
          break;
        }
      }

      if (!found) {
        NS_INFO("Required extension not found: '%s', skipping device.", re);
        ns::free(available_extensions,
                 sizeof(VkExtensionProperties) * available_extension_count,
                 MemTag::RENDERER);
        return false;
      }
    }
  }
  ns::free(available_extensions,
           sizeof(VkExtensionProperties) * available_extension_count,
           MemTag::RENDERER);

  if (requirements->sampler_anisotropy && !features->samplerAnisotropy) {
    NS_INFO("Device does not support samplerAnisotropy, skipping.");
    return false;
  }

  return true;
}

} // namespace ns::vulkan
