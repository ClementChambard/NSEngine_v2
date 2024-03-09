#ifndef RENDERER_FRONTEND_HEADER_INCLUDED
#define RENDERER_FRONTEND_HEADER_INCLUDED

#include "./renderer_types.inl"

namespace ns {

struct static_mesh_data;
struct platform_state;

bool renderer_system_initialize(u64 *memory_requirement, ptr state,
                                cstr application_name);
void renderer_system_shutdown(ptr state);

void renderer_on_resized(u16 width, u16 height);

bool renderer_draw_frame(render_packet *packet);

// HACK: remove NS_API when possible
NS_API void renderer_set_view(mat4 view);

void renderer_create_texture(cstr name, i32 width, i32 height,
                             i32 channel_count, robytes pixels,
                             bool has_transparency, Texture *out_texture);

void renderer_destroy_texture(Texture *texture);

} // namespace ns

#endif // RENDERER_FRONTEND_HEADER_INCLUDED
