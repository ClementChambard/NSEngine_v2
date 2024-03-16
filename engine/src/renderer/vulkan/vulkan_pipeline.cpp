#include "./vulkan_pipeline.h"
#include "../../core/logger.h"
#include "../../math/math.h"
#include "./vulkan_utils.h"

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
                              Pipeline *out_pipeline) {
  VkPipelineViewportStateCreateInfo viewport_state{};
  viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state.viewportCount = 1;
  viewport_state.pViewports = &viewport;
  viewport_state.scissorCount = 1;
  viewport_state.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterization{};
  rasterization.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterization.depthClampEnable = VK_FALSE;
  rasterization.rasterizerDiscardEnable = VK_FALSE;
  rasterization.polygonMode =
      is_wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
  rasterization.lineWidth = 1.0f;
  rasterization.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterization.depthBiasEnable = VK_FALSE;
  rasterization.depthBiasConstantFactor = 0.0f;
  rasterization.depthBiasClamp = 0.0f;
  rasterization.depthBiasSlopeFactor = 0.0f;

  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f;
  multisampling.pSampleMask = nullptr;
  multisampling.alphaToCoverageEnable = VK_FALSE;
  multisampling.alphaToOneEnable = VK_FALSE;

  VkPipelineDepthStencilStateCreateInfo depth_stencil{};
  if (depth_test_enabled) {
    depth_stencil.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;
  }

  VkPipelineColorBlendAttachmentState color_blend_attachment_state{};
  color_blend_attachment_state.blendEnable = VK_TRUE;
  color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  color_blend_attachment_state.dstColorBlendFactor =
      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
  color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  color_blend_attachment_state.dstAlphaBlendFactor =
      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
  color_blend_attachment_state.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  VkPipelineColorBlendStateCreateInfo color_blend{};
  color_blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_blend.logicOpEnable = VK_FALSE;
  color_blend.logicOp = VK_LOGIC_OP_COPY;
  color_blend.attachmentCount = 1;
  color_blend.pAttachments = &color_blend_attachment_state;

  static constexpr u32 dynamic_state_count = 3;
  VkDynamicState dynamic_states[dynamic_state_count] = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR,
      VK_DYNAMIC_STATE_LINE_WIDTH,
  };

  VkPipelineDynamicStateCreateInfo dynamic_state{};
  dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state.dynamicStateCount = dynamic_state_count;
  dynamic_state.pDynamicStates = dynamic_states;

  VkVertexInputBindingDescription binding_description;
  binding_description.binding = 0;
  binding_description.stride = stride;
  binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  VkPipelineVertexInputStateCreateInfo input{};
  input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  input.vertexBindingDescriptionCount = 1;
  input.pVertexBindingDescriptions = &binding_description;
  input.vertexAttributeDescriptionCount = attribute_count;
  input.pVertexAttributeDescriptions = attributes;

  VkPipelineInputAssemblyStateCreateInfo input_assembly{};
  input_assembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assembly.primitiveRestartEnable = VK_FALSE;

  VkPushConstantRange push_constant;
  push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  push_constant.offset = sizeof(mat4) * 0;
  push_constant.size = sizeof(mat4) * 2;

  VkPipelineLayoutCreateInfo layout_info{};
  layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layout_info.pushConstantRangeCount = 1;
  layout_info.pPushConstantRanges = &push_constant;
  layout_info.setLayoutCount = descriptor_set_layout_count;
  layout_info.pSetLayouts = descriptor_set_layouts;

  VK_CHECK(vkCreatePipelineLayout(context->device, &layout_info,
                                  context->allocator, &out_pipeline->layout));

  VkGraphicsPipelineCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  info.stageCount = stage_count;
  info.pStages = stages;
  info.pVertexInputState = &input;
  info.pInputAssemblyState = &input_assembly;
  info.pViewportState = &viewport_state;
  info.pRasterizationState = &rasterization;
  info.pMultisampleState = &multisampling;
  info.pDepthStencilState = depth_test_enabled ? &depth_stencil : nullptr;
  info.pColorBlendState = &color_blend;
  info.pDynamicState = &dynamic_state;
  info.pTessellationState = nullptr;
  info.layout = out_pipeline->layout;
  info.renderPass = *renderpass;
  info.subpass = 0;
  info.basePipelineHandle = VK_NULL_HANDLE;
  info.basePipelineIndex = -1;

  VkResult result =
      vkCreateGraphicsPipelines(context->device, VK_NULL_HANDLE, 1, &info,
                                context->allocator, &out_pipeline->handle);

  if (result_is_success(result)) {
    NS_DEBUG("Graphics pipeline created!");
    return true;
  }

  NS_ERROR("vkCreateGraphicsPipelines failed with %s.",
           result_string(result, true));
  return false;
}

void pipeline_destroy(Context *context, Pipeline *pipeline) {
  if (!pipeline)
    return;
  if (pipeline->handle) {
    vkDestroyPipeline(context->device, *pipeline, context->allocator);
    pipeline->handle = VK_NULL_HANDLE;
  }

  if (pipeline->layout) {
    vkDestroyPipelineLayout(context->device, pipeline->layout,
                            context->allocator);
    pipeline->layout = VK_NULL_HANDLE;
  }
}

void pipeline_bind(CommandBuffer *command_buffer,
                   VkPipelineBindPoint bind_point, Pipeline *pipeline) {
  vkCmdBindPipeline(*command_buffer, bind_point, *pipeline);
}

} // namespace ns::vulkan
