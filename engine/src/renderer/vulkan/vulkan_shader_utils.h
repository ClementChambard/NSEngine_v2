#ifndef VULKAN_SHADER_UTILS_HEADER_INCLUDED
#define VULKAN_SHADER_UTILS_HEADER_INCLUDED

#include "./vulkan_types.inl"

namespace ns::vulkan {

bool create_shader_module(Context *context, cstr name, cstr type_str,
                          VkShaderStageFlagBits shader_stage_flag,
                          u32 stage_index, ShaderStage *shader_stages);

}

#endif // VULKAN_SHADER_UTILS_HEADER_INCLUDED
