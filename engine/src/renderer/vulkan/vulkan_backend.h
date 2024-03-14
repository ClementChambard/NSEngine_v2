#ifndef VULKAN_BACKEND_HEADER_INCLUDED
#define VULKAN_BACKEND_HEADER_INCLUDED

#include "../renderer_backend.h"

namespace ns::vulkan {

bool backend_initialize(renderer_backend *backend, cstr application_name);

void backend_shutdown(renderer_backend *backend);

void backend_on_resized(renderer_backend *backend, u16 width, u16 height);

bool backend_begin_frame(renderer_backend *backend, f32 delta_time);

void backend_update_global_world_state(mat4 projection, mat4 view,
                                       vec3 view_position, vec4 ambient_color,
                                       i32 mode);

void backend_update_global_ui_state(mat4 projection, mat4 view, i32 mode);

bool backend_end_frame(renderer_backend *backend, f32 delta_time);

bool backend_begin_renderpass(renderer_backend *backend, u8 renderpass_id);
bool backend_end_renderpass(renderer_backend *backend, u8 renderpass_id);

void backend_draw_geometry(geometry_render_data);

void backend_create_texture(robytes pixels, Texture *texture);

void backend_destroy_texture(Texture *texture);

bool backend_create_material(Material *material);

void backend_destroy_material(Material *material);

bool backend_create_geometry(Geometry *geometry, u32 vertex_size,
                             u32 vertex_count, roptr vertices, u32 index_size,
                             u32 index_count, roptr indices);

void backend_destroy_geometry(Geometry *geometry);

} // namespace ns::vulkan

#endif // VULKAN_BACKEND_HEADER_INCLUDED
