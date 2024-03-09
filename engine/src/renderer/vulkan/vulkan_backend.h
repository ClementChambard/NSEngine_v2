#ifndef VULKAN_BACKEND_HEADER_INCLUDED
#define VULKAN_BACKEND_HEADER_INCLUDED

#include "../renderer_backend.h"

namespace ns::vulkan {

bool renderer_backend_initialize(renderer_backend *backend,
                                 cstr application_name);

void renderer_backend_shutdown(renderer_backend *backend);

void renderer_backend_on_resized(renderer_backend *backend, u16 width,
                                 u16 height);

bool renderer_backend_begin_frame(renderer_backend *backend, f32 delta_time);

void renderer_update_global_state(mat4 projection, mat4 view,
                                  vec3 view_position, vec4 ambient_color,
                                  i32 mode);

bool renderer_backend_end_frame(renderer_backend *backend, f32 delta_time);

// temp
void renderer_update_object(geometry_render_data);

void renderer_create_texture(cstr name, i32 width, i32 height,
                             i32 channel_count, robytes pixels,
                             bool has_transparency, Texture *out_texture);

void renderer_destroy_texture(Texture *texture);

} // namespace ns::vulkan

#endif // VULKAN_BACKEND_HEADER_INCLUDED
