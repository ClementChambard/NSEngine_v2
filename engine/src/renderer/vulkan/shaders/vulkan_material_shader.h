#ifndef VULKAN_OBJECT_SHADER_HEADER_INCLUDED
#define VULKAN_OBJECT_SHADER_HEADER_INCLUDED

#include "../../renderer_types.inl"
#include "../vulkan_types.inl"

namespace ns::vulkan {

bool material_shader_create(Context *context, MaterialShader *out_shader);

void material_shader_destroy(Context *context, MaterialShader *shader);

void material_shader_use(Context *context, MaterialShader *shader);

void material_shader_update_global_state(Context *context,
                                         MaterialShader *shader,
                                         f32 delta_time);

void material_shader_update_object(Context *context, MaterialShader *shader,
                                   geometry_render_data data);

bool material_shader_acquire_resources(Context *context, MaterialShader *shader,
                                       u32 *out_object_id);

void material_shader_release_resources(Context *context, MaterialShader *shader,
                                       u32 object_id);

} // namespace ns::vulkan

#endif // VULKAN_OBJECT_SHADER_HEADER_INCLUDED
