#ifndef RENDERER_FRONTEND_HEADER_INCLUDED
#define RENDERER_FRONTEND_HEADER_INCLUDED

#include "./renderer_types.inl"

namespace ns {

struct platform_state;

bool renderer_system_initialize(u64 *memory_requirement, ptr state,
                                cstr application_name);
void renderer_system_shutdown(ptr state);

void renderer_on_resized(u16 width, u16 height);

bool renderer_draw_frame(render_packet *packet);

// HACK: remove NS_API when possible
NS_API void renderer_set_view(mat4 view);

void renderer_create_texture(robytes pixels, Texture *texture);

void renderer_destroy_texture(Texture *texture);

bool renderer_create_material(Material *material);

void renderer_destroy_material(Material *material);

bool renderer_create_geometry(Geometry *geometry, u32 vertex_size,
                              u32 vertex_count, roptr vertices, u32 index_size,
                              u32 index_count, roptr indices);

void renderer_destroy_geometry(Geometry *geometry);

} // namespace ns

#endif // RENDERER_FRONTEND_HEADER_INCLUDED
