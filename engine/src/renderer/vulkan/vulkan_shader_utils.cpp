#include "./vulkan_shader_utils.h"

#include "../../core/ns_memory.h"
#include "../../core/ns_string.h"

#include "../../systems/resource_system.h"

namespace ns::vulkan {

bool create_shader_module(Context *context, cstr name, cstr type_str,
                          VkShaderStageFlagBits shader_stage_flag,
                          u32 stage_index, ShaderStage *shader_stages) {
  char file_name[512];
  string_fmt(file_name, sizeof(file_name), "shaders/%s.%s.spv", name, type_str);

  Resource bin_res;
  if (!resource_system_load(file_name, ResourceType::BINARY, &bin_res)) {
    NS_ERROR("Unable to read shader module: %s.", file_name);
    return false;
  }

  mem_zero(&shader_stages[stage_index].create_info,
           sizeof(VkShaderModuleCreateInfo));
  shader_stages[stage_index].create_info.sType =
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shader_stages[stage_index].create_info.codeSize = bin_res.data_size;
  shader_stages[stage_index].create_info.pCode =
      reinterpret_cast<u32 *>(bin_res.data);

  VK_CHECK(vkCreateShaderModule(
      context->device, &shader_stages[stage_index].create_info,
      context->allocator, &shader_stages[stage_index].handle));

  resource_system_unload(&bin_res);

  mem_zero(&shader_stages[stage_index].shader_stage_create_info,
           sizeof(VkPipelineShaderStageCreateInfo));
  shader_stages[stage_index].shader_stage_create_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader_stages[stage_index].shader_stage_create_info.stage = shader_stage_flag;
  shader_stages[stage_index].shader_stage_create_info.module =
      shader_stages[stage_index];
  shader_stages[stage_index].shader_stage_create_info.pName = "main";

  return true;
}

} // namespace ns::vulkan
