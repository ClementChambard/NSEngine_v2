#ifndef VULKAN_PIPELINE_HEADER_INCLUDED
#define VULKAN_PIPELINE_HEADER_INCLUDED

#include "./vulkan_types.inl"

namespace ns::vulkan {

bool graphics_pipeline_create(Context *context, Renderpass *renderpass,
                              u32 stride, u32 attribute_count,
                              VkVertexInputAttributeDescription *attributes,
                              u32 descriptor_set_layout_count,
                              VkDescriptorSetLayout *descriptor_set_layouts,
                              u32 stage_count,
                              VkPipelineShaderStageCreateInfo *stages,
                              VkViewport viewport, VkRect2D scissor,
                              bool is_wireframe, bool depth_test_enabled,
                              Pipeline *out_pipeline);

void pipeline_destroy(Context *context, Pipeline *pipeline);

void pipeline_bind(CommandBuffer *command_buffer,
                   VkPipelineBindPoint bind_point, Pipeline *pipeline);

} // namespace ns::vulkan

#endif // VULKAN_PIPELINE_HEADER_INCLUDED
