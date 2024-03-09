#include "./vulkan_material_shader.h"

#include "../../../core/logger.h"
#include "../../../math/ns_math.h"
#include "../vulkan_buffer.h"
#include "../vulkan_pipeline.h"
#include "../vulkan_shader_utils.h"
#include <vulkan/vulkan_core.h>

#include "../../../systems/texture_system.h"

#define BUILTIN_SHADER_NAME_MATERIAL "Builtin.MaterialShader"

namespace ns::vulkan {

bool material_shader_create(Context *context, MaterialShader *out_shader) {
  char stage_type_strs[MaterialShader::SHADER_STAGE_COUNT][5] = {"vert",
                                                                 "frag"};
  VkShaderStageFlagBits stage_types[MaterialShader::SHADER_STAGE_COUNT] = {
      VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

  for (usize i = 0; i < MaterialShader::SHADER_STAGE_COUNT; i++) {
    if (!create_shader_module(context, BUILTIN_SHADER_NAME_MATERIAL,
                              stage_type_strs[i], stage_types[i], i,
                              out_shader->stages)) {
      NS_ERROR("Unable to create %s shader module for '%s'.",
               stage_type_strs[i], BUILTIN_SHADER_NAME_MATERIAL);
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

  const u32 local_sampler_count = 1;
  VkDescriptorSetLayoutBinding
      bindings[MaterialShader::SHADER_DESCRIPTOR_COUNT] = {
          {
              .binding = 0,
              .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
              .descriptorCount = 1,
              .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
              .pImmutableSamplers = nullptr,
          },
          {
              .binding = 1,
              .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
              .descriptorCount = 1,
              .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
              .pImmutableSamplers = nullptr,
          }};

  VkDescriptorSetLayoutCreateInfo layout_info{};
  layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layout_info.bindingCount = MaterialShader::SHADER_DESCRIPTOR_COUNT;
  layout_info.pBindings = bindings;

  VK_CHECK(vkCreateDescriptorSetLayout(
      context->device.logical_device, &layout_info, context->allocator,
      &out_shader->object_descriptor_set_layout));

  VkDescriptorPoolSize object_pool_sizes[2];
  object_pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  object_pool_sizes[0].descriptorCount = MaterialShader::MAX_OBJECT_COUNT;
  object_pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  object_pool_sizes[1].descriptorCount =
      local_sampler_count * MaterialShader::MAX_OBJECT_COUNT;

  VkDescriptorPoolCreateInfo object_pool_info{};
  object_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  object_pool_info.poolSizeCount = 2;
  object_pool_info.pPoolSizes = object_pool_sizes;
  object_pool_info.maxSets = MaterialShader::MAX_OBJECT_COUNT;
  VK_CHECK(vkCreateDescriptorPool(context->device.logical_device,
                                  &object_pool_info, context->allocator,
                                  &out_shader->object_descriptor_pool));

  f32 framebuffer_width =
      context->device.swapchain_support.capabilities.currentExtent.width;
  f32 framebuffer_height =
      context->device.swapchain_support.capabilities.currentExtent.height;
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

  static constexpr i32 attribute_count = 2;
  VkVertexInputAttributeDescription attribute_descs[attribute_count] = {
      {
          .location = 0,
          .binding = 0,
          .format = VK_FORMAT_R32G32B32_SFLOAT,
          .offset = 0,
      },
      {
          .location = 1,
          .binding = 0,
          .format = VK_FORMAT_R32G32_SFLOAT,
          .offset = sizeof(vec3),
      },
  };

  const i32 descriptor_set_layout_count = 2;
  VkDescriptorSetLayout layouts[descriptor_set_layout_count] = {
      out_shader->global_descriptor_set_layout,
      out_shader->object_descriptor_set_layout,
  };

  VkPipelineShaderStageCreateInfo
      stage_create_infos[MaterialShader::SHADER_STAGE_COUNT];
  mem_zero(stage_create_infos, sizeof(stage_create_infos));
  for (usize i = 0; i < MaterialShader::SHADER_STAGE_COUNT; i++) {
    stage_create_infos[i].sType =
        out_shader->stages[i].shader_stage_create_info.sType;
    stage_create_infos[i] = out_shader->stages[i].shader_stage_create_info;
  }

  if (!graphics_pipeline_create(
          context, &context->main_renderpass, attribute_count, attribute_descs,
          descriptor_set_layout_count, layouts,
          MaterialShader::SHADER_STAGE_COUNT, stage_create_infos, viewport,
          scissor, false, &out_shader->pipeline)) {
    NS_ERROR("Failed to load graphics pipeline for object shader.");
    return false;
  }

  u32 device_local_bit = context->device.supports_device_local_host_visible
                             ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
                             : 0;
  if (!buffer_create(context, sizeof(global_uniform_object) * 3,
                     static_cast<VkBufferUsageFlagBits>(
                         VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                         VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT),
                     device_local_bit | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     true, &out_shader->global_uniform_buffer)) {
    NS_ERROR("Vulkan buffer creation failed for object shader.");
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

  if (!buffer_create(context, sizeof(object_uniform_object),
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

void material_shader_destroy(Context *context, MaterialShader *shader) {
  buffer_destroy(context, &shader->global_uniform_buffer);

  buffer_destroy(context, &shader->object_uniform_buffer);

  pipeline_destroy(context, &shader->pipeline);

  vkDestroyDescriptorPool(context->device, shader->global_descriptor_pool,
                          context->allocator);

  vkDestroyDescriptorPool(context->device, shader->object_descriptor_pool,
                          context->allocator);

  vkDestroyDescriptorSetLayout(context->device,
                               shader->global_descriptor_set_layout,
                               context->allocator);

  vkDestroyDescriptorSetLayout(context->device,
                               shader->object_descriptor_set_layout,
                               context->allocator);

  for (usize i = 0; i < MaterialShader::SHADER_STAGE_COUNT; i++) {
    vkDestroyShaderModule(context->device, shader->stages[i].handle,
                          context->allocator);
    shader->stages[i].handle = VK_NULL_HANDLE;
  }
}

void material_shader_use(Context *context, MaterialShader *shader) {
  u32 image_index = context->image_index;
  pipeline_bind(&context->graphics_command_buffers[image_index],
                VK_PIPELINE_BIND_POINT_GRAPHICS, &shader->pipeline);
}

void material_shader_update_global_state(Context *context,
                                         MaterialShader *shader,
                                         f32 /*delta_time*/) {
  u32 image_index = context->image_index;
  VkDescriptorSet global_descriptor =
      shader->global_descriptor_sets[image_index];

  usize range = sizeof(global_uniform_object);
  usize offset = sizeof(global_uniform_object) * image_index;

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

void material_shader_update_object(Context *context, MaterialShader *shader,
                                   geometry_render_data data) {
  u32 image_index = context->image_index;
  vkCmdPushConstants(context->graphics_command_buffers[image_index],
                     shader->pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                     sizeof(mat4), &data.model);

  MaterialShader::ObjectState *object_state =
      &shader->object_states[data.object_id];
  VkDescriptorSet object_descriptor =
      object_state->descriptor_sets[image_index];

  VkWriteDescriptorSet
      descriptor_writes[MaterialShader::SHADER_DESCRIPTOR_COUNT];
  mem_zero(descriptor_writes, sizeof(VkWriteDescriptorSet) *
                                  MaterialShader::SHADER_DESCRIPTOR_COUNT);
  u32 descriptor_count = 0;
  u32 descriptor_index = 0;

  u32 range = sizeof(object_uniform_object);
  u32 offset = sizeof(object_uniform_object) * data.object_id;
  object_uniform_object obo;

  static f32 accumulator = 0.0f;
  accumulator += context->frame_delta_time;
  f32 s = (sin(accumulator) + 1.0f) / 2.0f;
  obo.diffuse_color = vec4(s, s, s, 1.0f);

  buffer_load_data(context, &shader->object_uniform_buffer, offset, range, 0,
                   &obo);

  if (object_state->descriptor_states[descriptor_index]
          .generations[image_index] == INVALID_ID) {
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

    object_state->descriptor_states[descriptor_index].generations[image_index] =
        1;
  }
  descriptor_index++;

  const u32 sampler_count = 1;

  VkDescriptorImageInfo image_infos[sampler_count];
  for (u32 i = 0; i < sampler_count; i++) {
    Texture *t = data.textures[i];
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

bool material_shader_acquire_resources(Context *context, MaterialShader *shader,
                                       u32 *out_object_id) {
  *out_object_id = shader->object_uniform_buffer_index;
  shader->object_uniform_buffer_index++;

  u32 object_id = *out_object_id;
  MaterialShader::ObjectState *object_state = &shader->object_states[object_id];
  for (u32 i = 0; i < MaterialShader::SHADER_DESCRIPTOR_COUNT; i++) {
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

void material_shader_release_resources(Context *context, MaterialShader *shader,
                                       u32 object_id) {
  MaterialShader::ObjectState *object_state = &shader->object_states[object_id];

  const u32 descriptor_set_count = 3;
  VkResult result =
      vkFreeDescriptorSets(context->device, shader->object_descriptor_pool,
                           descriptor_set_count, object_state->descriptor_sets);

  if (result != VK_SUCCESS) {
    NS_ERROR("Error freeing descriptor sets in shader");
  }

  for (u32 i = 0; i < MaterialShader::SHADER_DESCRIPTOR_COUNT; i++) {
    for (u32 j = 0; j < 3; j++) {
      object_state->descriptor_states[i].generations[j] = INVALID_ID;
      object_state->descriptor_states[i].ids[j] = INVALID_ID;
    }
  }
}

} // namespace ns::vulkan
