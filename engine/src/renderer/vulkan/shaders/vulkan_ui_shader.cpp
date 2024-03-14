#include "./vulkan_ui_shader.h"

#include "../../../core/ns_memory.h"
#include "../../../math/ns_math.h"
#include "../vulkan_buffer.h"
#include "../vulkan_pipeline.h"
#include "../vulkan_shader_utils.h"

#include "../../../systems/texture_system.h"

#define BUILTIN_SHADER_NAME_UI "Builtin.UIShader"

namespace ns::vulkan {

bool ui_shader_create(Context *context, UiShader *out_shader) {
  char stage_type_strs[UiShader::STAGE_COUNT][5] = {"vert", "frag"};
  VkShaderStageFlagBits stage_types[UiShader::STAGE_COUNT] = {
      VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

  for (usize i = 0; i < UiShader::STAGE_COUNT; i++) {
    if (!create_shader_module(context, BUILTIN_SHADER_NAME_UI,
                              stage_type_strs[i], stage_types[i], i,
                              out_shader->stages)) {
      NS_ERROR("Unable to create %s shader module for '%s'.",
               stage_type_strs[i], BUILTIN_SHADER_NAME_UI);
      return false;
    }
  }

  // Global descriptors
  VkDescriptorSetLayoutBinding global_ubo_layout_binding{};
  global_ubo_layout_binding.binding = 0;
  global_ubo_layout_binding.descriptorCount = 1;
  global_ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  global_ubo_layout_binding.pImmutableSamplers = nullptr;
  global_ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutCreateInfo global_layout_info{};
  global_layout_info.sType =
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  global_layout_info.bindingCount = 1;
  global_layout_info.pBindings = &global_ubo_layout_binding;
  VK_CHECK(vkCreateDescriptorSetLayout(
      context->device.logical_device, &global_layout_info, context->allocator,
      &out_shader->global_descriptor_set_layout));

  VkDescriptorPoolSize global_pool_size;
  global_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  global_pool_size.descriptorCount = context->swapchain.image_count;

  VkDescriptorPoolCreateInfo global_pool_info{};
  global_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  global_pool_info.poolSizeCount = 1;
  global_pool_info.pPoolSizes = &global_pool_size;
  global_pool_info.maxSets = context->swapchain.image_count;
  VK_CHECK(vkCreateDescriptorPool(context->device.logical_device,
                                  &global_pool_info, context->allocator,
                                  &out_shader->global_descriptor_pool));

  out_shader->sampler_uses[0] = TextureUse::MAP_DIFFUSE;

  VkDescriptorSetLayoutBinding bindings[UiShader::DESCRIPTOR_COUNT];
  bindings[0].binding = 0;
  bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  bindings[0].descriptorCount = 1;
  bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  bindings[0].pImmutableSamplers = nullptr;
  bindings[1].binding = 1;
  bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  bindings[1].descriptorCount = 1;
  bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  bindings[1].pImmutableSamplers = nullptr;

  VkDescriptorSetLayoutCreateInfo layout_info{};
  layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layout_info.bindingCount = UiShader::DESCRIPTOR_COUNT;
  layout_info.pBindings = bindings;
  VK_CHECK(vkCreateDescriptorSetLayout(
      context->device.logical_device, &layout_info, context->allocator,
      &out_shader->object_descriptor_set_layout));

  VkDescriptorPoolSize object_pool_sizes[2];
  object_pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  object_pool_sizes[0].descriptorCount = UiShader::MAX_UI_COUNT;
  object_pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  object_pool_sizes[1].descriptorCount =
      UiShader::SAMPLER_COUNT * UiShader::MAX_UI_COUNT;

  VkDescriptorPoolCreateInfo object_pool_info{};
  object_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  object_pool_info.poolSizeCount = 2;
  object_pool_info.pPoolSizes = object_pool_sizes;
  object_pool_info.maxSets = UiShader::MAX_UI_COUNT;
  object_pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  VK_CHECK(vkCreateDescriptorPool(context->device.logical_device,
                                  &object_pool_info, context->allocator,
                                  &out_shader->object_descriptor_pool));

  VkViewport viewport;
  viewport.x = 0.0f;
  viewport.y = static_cast<f32>(
      context->device.swapchain_support.capabilities.currentExtent.height);
  viewport.width = static_cast<f32>(
      context->device.swapchain_support.capabilities.currentExtent.width);
  viewport.height = -viewport.y;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor;
  scissor.offset = {0, 0};
  scissor.extent = context->device.swapchain_support.capabilities.currentExtent;

  VkVertexInputAttributeDescription attributes[2] = {
      {.location = 0, // pos
       .binding = 0,
       .format = VK_FORMAT_R32G32_SFLOAT,
       .offset = 0},
      {.location = 1, // uv
       .binding = 0,
       .format = VK_FORMAT_R32G32_SFLOAT,
       .offset = sizeof(vec2)}};

  const i32 descriptor_set_layout_count = 2;
  VkDescriptorSetLayout layouts[2] = {
      out_shader->global_descriptor_set_layout,
      out_shader->object_descriptor_set_layout,
  };

  VkPipelineShaderStageCreateInfo stage_create_infos[UiShader::STAGE_COUNT];
  mem_zero(stage_create_infos, sizeof(stage_create_infos));
  for (u32 i = 0; i < UiShader::STAGE_COUNT; i++) {
    stage_create_infos[i] = out_shader->stages[i].shader_stage_create_info;
  }

  if (!graphics_pipeline_create(
          context, &context->ui_renderpass, sizeof(vertex_2d), 2, attributes,
          descriptor_set_layout_count, layouts, UiShader::STAGE_COUNT,
          stage_create_infos, viewport, scissor, false, false,
          &out_shader->pipeline)) {
    NS_ERROR("Failed to create ui graphics pipeline");
    return false;
  }

  u32 device_local_bit = context->device.supports_device_local_host_visible
                             ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
                             : 0;
  if (!buffer_create(context, sizeof(UiShader::GlobalUBO) * 3,
                     static_cast<VkBufferUsageFlagBits>(
                         VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                         VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT),
                     device_local_bit | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     true, &out_shader->global_uniform_buffer)) {
    NS_ERROR("Vulkan buffer creation failed for ui shader.");
    return false;
  }

  VkDescriptorSetLayout global_layouts[3] = {
      out_shader->global_descriptor_set_layout,
      out_shader->global_descriptor_set_layout,
      out_shader->global_descriptor_set_layout};

  VkDescriptorSetAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.descriptorPool = out_shader->global_descriptor_pool;
  alloc_info.descriptorSetCount = 3;
  alloc_info.pSetLayouts = global_layouts;
  VK_CHECK(vkAllocateDescriptorSets(context->device.logical_device, &alloc_info,
                                    out_shader->global_descriptor_sets));

  if (!buffer_create(context,
                     sizeof(UiShader::InstanceUBO) * UiShader::MAX_UI_COUNT,
                     static_cast<VkBufferUsageFlagBits>(
                         VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                         VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT),
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     true, &out_shader->object_uniform_buffer)) {
    NS_ERROR("Material instance buffer creation failed for shader.");
    return false;
  }

  return true;
}

void ui_shader_destroy(Context *context, UiShader *shader) {
  vkDestroyDescriptorPool(context->device, shader->object_descriptor_pool,
                          context->allocator);
  vkDestroyDescriptorSetLayout(context->device,
                               shader->object_descriptor_set_layout,
                               context->allocator);

  buffer_destroy(context, &shader->global_uniform_buffer);
  buffer_destroy(context, &shader->object_uniform_buffer);

  pipeline_destroy(context, &shader->pipeline);

  vkDestroyDescriptorPool(context->device, shader->global_descriptor_pool,
                          context->allocator);
  vkDestroyDescriptorSetLayout(context->device,
                               shader->global_descriptor_set_layout,
                               context->allocator);

  for (u32 i = 0; i < UiShader::STAGE_COUNT; i++) {
    vkDestroyShaderModule(context->device, shader->stages[i].handle,
                          context->allocator);
    shader->stages[i].handle = VK_NULL_HANDLE;
  }
}

void ui_shader_use(Context *context, UiShader *shader) {
  u32 image_index = context->image_index;
  pipeline_bind(&context->graphics_command_buffers[image_index],
                VK_PIPELINE_BIND_POINT_GRAPHICS, &shader->pipeline);
}

void ui_shader_update_global_state(Context *context, UiShader *shader,
                                   f32 /*delta_time*/) {
  u32 image_index = context->image_index;
  VkDescriptorSet global_descriptor =
      shader->global_descriptor_sets[image_index];

  usize range = sizeof(UiShader::GlobalUBO);
  usize offset = sizeof(UiShader::GlobalUBO) * image_index;

  buffer_load_data(context, &shader->global_uniform_buffer, offset, range, 0,
                   &shader->global_ubo);

  VkDescriptorBufferInfo bufferInfo;
  bufferInfo.buffer = shader->global_uniform_buffer.handle;
  bufferInfo.offset = offset;
  bufferInfo.range = range;

  VkWriteDescriptorSet descriptor_write{};
  descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptor_write.dstSet = global_descriptor;
  descriptor_write.dstBinding = 0;
  descriptor_write.dstArrayElement = 0;
  descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptor_write.descriptorCount = 1;
  descriptor_write.pBufferInfo = &bufferInfo;

  vkUpdateDescriptorSets(context->device, 1, &descriptor_write, 0, nullptr);

  vkCmdBindDescriptorSets(context->graphics_command_buffers[image_index],
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          shader->pipeline.layout, 0, 1, &global_descriptor, 0,
                          nullptr);
}

void ui_shader_set_model(Context *context, UiShader *shader, mat4 model) {
  if (context && shader) {
    vkCmdPushConstants(context->graphics_command_buffers[context->image_index],
                       shader->pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                       sizeof(mat4), &model);
  }
}

void ui_shader_apply_material(Context *context, UiShader *shader,
                              Material *material) {
  if (!context || !shader || !material) {
    return;
  }
  u32 image_index = context->image_index;

  UiShader::InstanceState *object_state =
      &shader->instance_states[material->internal_id];
  VkDescriptorSet object_descriptor =
      object_state->descriptor_sets[image_index];

  VkWriteDescriptorSet descriptor_writes[UiShader::DESCRIPTOR_COUNT];
  mem_zero(descriptor_writes,
           sizeof(VkWriteDescriptorSet) * UiShader::DESCRIPTOR_COUNT);
  u32 descriptor_count = 0;
  u32 descriptor_index = 0;

  u32 range = sizeof(UiShader::InstanceUBO);
  u32 offset = sizeof(UiShader::InstanceUBO) * material->internal_id;
  UiShader::InstanceUBO inst_ubo;

  inst_ubo.diffuse_color = material->diffuse_color;

  buffer_load_data(context, &shader->object_uniform_buffer, offset, range, 0,
                   &inst_ubo);

  u32 *global_ubo_generation =
      &object_state->descriptor_states[descriptor_index]
           .generations[image_index];
  if (*global_ubo_generation == INVALID_ID ||
      *global_ubo_generation != material->generation) {
    VkDescriptorBufferInfo buffer_info;
    buffer_info.buffer = shader->object_uniform_buffer;
    buffer_info.offset = offset;
    buffer_info.range = range;

    VkWriteDescriptorSet descriptor{};
    descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor.dstSet = object_descriptor;
    descriptor.dstBinding = descriptor_index;
    descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor.descriptorCount = 1;
    descriptor.pBufferInfo = &buffer_info;

    descriptor_writes[descriptor_count++] = descriptor;

    *global_ubo_generation = material->generation;
  }
  descriptor_index++;

  VkDescriptorImageInfo image_infos[UiShader::SAMPLER_COUNT];
  for (u32 i = 0; i < UiShader::SAMPLER_COUNT; i++) {
    TextureUse use = shader->sampler_uses[i];
    Texture *t = nullptr;

    switch (use) {
    case TextureUse::MAP_DIFFUSE: {
      t = material->diffuse_map.texture;
      break;
    }
    default:
      NS_FATAL("Unable to bind sampler to unknown use.");
      return;
    }

    u32 *descriptor_generation =
        &object_state->descriptor_states[descriptor_index]
             .generations[image_index];
    u32 *descriptor_id =
        &object_state->descriptor_states[descriptor_index].ids[image_index];

    if (t->generation == INVALID_ID) {
      t = texture_system_get_default_texture();
      *descriptor_generation = INVALID_ID;
    }

    if (t &&
        (*descriptor_id != t->id || *descriptor_generation != t->generation ||
         *descriptor_generation == INVALID_ID)) {
      TextureData *internal_data =
          reinterpret_cast<TextureData *>(t->internal_data);
      image_infos[i].imageView = internal_data->image.view;
      image_infos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      image_infos[i].sampler = internal_data->sampler;

      VkWriteDescriptorSet descriptor{};
      descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptor.dstSet = object_descriptor;
      descriptor.dstBinding = descriptor_index;
      descriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      descriptor.descriptorCount = 1;
      descriptor.pImageInfo = &image_infos[i];

      descriptor_writes[descriptor_count++] = descriptor;

      if (t->generation != INVALID_ID) {
        *descriptor_generation = t->generation;
        *descriptor_id = t->id;
      }

      descriptor_index++;
    }
  }

  if (descriptor_count > 0) {
    vkUpdateDescriptorSets(context->device, descriptor_count, descriptor_writes,
                           0, nullptr);
  }

  vkCmdBindDescriptorSets(context->graphics_command_buffers[image_index],
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          shader->pipeline.layout, 1, 1, &object_descriptor, 0,
                          nullptr);
}

bool ui_shader_acquire_resources(Context *context, UiShader *shader,
                                 Material *material) {
  material->internal_id = shader->object_uniform_buffer_index;
  shader->object_uniform_buffer_index++;

  UiShader::InstanceState *object_state =
      &shader->instance_states[material->internal_id];
  for (u32 i = 0; i < UiShader::DESCRIPTOR_COUNT; i++) {
    for (u32 j = 0; j < 3; j++) {
      object_state->descriptor_states[i].generations[j] = INVALID_ID;
      object_state->descriptor_states[i].ids[j] = INVALID_ID;
    }
  }

  VkDescriptorSetLayout layouts[3] = {
      shader->object_descriptor_set_layout,
      shader->object_descriptor_set_layout,
      shader->object_descriptor_set_layout,
  };

  VkDescriptorSetAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.descriptorPool = shader->object_descriptor_pool;
  alloc_info.descriptorSetCount = 3;
  alloc_info.pSetLayouts = layouts;

  if (vkAllocateDescriptorSets(context->device, &alloc_info,
                               object_state->descriptor_sets) != VK_SUCCESS) {
    NS_ERROR("Error allocating descriptor sets in shader")
    return false;
  }

  return true;
}

void ui_shader_release_resources(Context *context, UiShader *shader,
                                 Material *material) {
  vkDeviceWaitIdle(context->device);

  UiShader::InstanceState *object_state =
      &shader->instance_states[material->internal_id];

  const u32 descriptor_set_count = 3;
  VkResult result =
      vkFreeDescriptorSets(context->device, shader->object_descriptor_pool,
                           descriptor_set_count, object_state->descriptor_sets);

  if (result != VK_SUCCESS) {
    NS_ERROR("Error freeing descriptor sets in shader");
  }

  for (u32 i = 0; i < UiShader::DESCRIPTOR_COUNT; i++) {
    for (u32 j = 0; j < 3; j++) {
      object_state->descriptor_states[i].generations[j] = INVALID_ID;
      object_state->descriptor_states[i].ids[j] = INVALID_ID;
    }
  }

  material->internal_id = INVALID_ID;
}

} // namespace ns::vulkan
