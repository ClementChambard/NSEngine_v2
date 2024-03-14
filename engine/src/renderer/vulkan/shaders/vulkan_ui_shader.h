#ifndef VULKAN_UI_SHADER_HEADER_INCLUDED
#define VULKAN_UI_SHADER_HEADER_INCLUDED

#include "../vulkan_types.inl"

namespace ns::vulkan {

bool ui_shader_create(Context *context, UiShader *out_shader);

void ui_shader_destroy(Context *context, UiShader *shader);

void ui_shader_use(Context *context, UiShader *shader);

void ui_shader_update_global_state(Context *context, UiShader *shader,
                                   f32 delta_time);

void ui_shader_apply_material(Context *context, UiShader *shader,
                              Material *material);

void ui_shader_set_model(Context *context, UiShader *shader, mat4 model);
void ui_shader_apply_ui(Context *context, UiShader *shader, Material *material);

bool ui_shader_acquire_resources(Context *context, UiShader *shader,
                                 Material *material);

void ui_shader_release_resources(Context *context, UiShader *shader,
                                 Material *material);

} // namespace ns::vulkan

#endif // VULKAN_UI_SHADER_HEADER_INCLUDED
