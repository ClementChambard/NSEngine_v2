#ifndef VULKAN_MATERIAL_SHADER_HEADER_INCLUDED
#define VULKAN_MATERIAL_SHADER_HEADER_INCLUDED

#include "../vulkan_types.inl"

namespace ns::vulkan {

bool material_shader_create(Context *context, MaterialShader *out_shader);

void material_shader_destroy(Context *context, MaterialShader *shader);

void material_shader_use(Context *context, MaterialShader *shader);

void material_shader_update_global_state(Context *context,
                                         MaterialShader *shader,
                                         f32 delta_time);

void material_shader_set_model(Context *context, MaterialShader *shader,
                               mat4 model);
void material_shader_apply_material(Context *context, MaterialShader *shader,
                                    Material *material);

bool material_shader_acquire_resources(Context *context, MaterialShader *shader,
                                       Material *material);

void material_shader_release_resources(Context *context, MaterialShader *shader,
                                       Material *material);

} // namespace ns::vulkan

#endif // VULKAN_MATERIAL_SHADER_HEADER_INCLUDED
