#include "./vulkan_shader_utils.h"

#include "../../core/ns_memory.h"
#include "../../core/ns_string.h"
#include "../../platform/filesystem.h"

namespace ns::vulkan {

bool create_shader_module(Context *context, cstr name, cstr type_str,
                          VkShaderStageFlagBits shader_stage_flag,
                          u32 stage_index, ShaderStage *shader_stages) {
  char file_name[512];
  string_fmt(file_name, sizeof(file_name), "assets/shaders/%s.%s.spv", name,
             type_str);

  mem_zero(&shader_stages[stage_index].create_info,
           sizeof(VkShaderModuleCreateInfo));
  shader_stages[stage_index].create_info.sType =
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

  fs::File handle;
  if (!fs::open(file_name, fs::Mode::READ, true, &handle)) {
    NS_ERROR("Unable to read shader module: %s.", file_name);
    return false;
  }

  usize size = 0;
  u8 *file_buffer = nullptr;
  if (!fs::read_all_bytes(&handle, &file_buffer, &size)) {
    NS_ERROR("Unable to binary read shader module: %s.", file_name);
    return false;
  }
  shader_stages[stage_index].create_info.codeSize = size;
  shader_stages[stage_index].create_info.pCode =
      reinterpret_cast<u32 *>(file_buffer);

  fs::close(&handle);

  VK_CHECK(vkCreateShaderModule(
      context->device, &shader_stages[stage_index].create_info,
      context->allocator, &shader_stages[stage_index].handle));

  mem_zero(&shader_stages[stage_index].shader_stage_create_info,
           sizeof(VkPipelineShaderStageCreateInfo));
  shader_stages[stage_index].shader_stage_create_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader_stages[stage_index].shader_stage_create_info.stage = shader_stage_flag;
  shader_stages[stage_index].shader_stage_create_info.module =
      shader_stages[stage_index];
  shader_stages[stage_index].shader_stage_create_info.pName = "main";

  if (file_buffer) {
    ns::free(file_buffer, sizeof(u8) * size, mem_tag::STRING);
    file_buffer = nullptr;
  }

  return true;
}

} // namespace ns::vulkan
