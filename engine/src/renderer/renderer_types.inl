#ifndef RENDERER_TYPES_INLINE_INCLUDED
#define RENDERER_TYPES_INLINE_INCLUDED

#include "../defines.h"
#include "../math/types/math_types.h"
#include "../resources/resource_types.h"

namespace ns {

enum renderer_backend_type {
  RENDERER_BACKEND_TYPE_VULKAN,
  RENDERER_BACKEND_TYPE_OPENGL,
  RENDERER_BACKEND_TYPE_DIRECTX,
};

struct geometry_render_data {
  mat4 model;
  Geometry *geometry;
};

enum class builtin_renderpass : u8 {
  WORLD = 0x01,
  UI = 0x02,
};

struct renderer_backend {
  u64 frame_number;

  bool (*initialize)(renderer_backend *backend, cstr application_name);

  void (*shutdown)(renderer_backend *backend);

  void (*resized)(renderer_backend *backend, u16 width, u16 height);

  bool (*begin_frame)(renderer_backend *backend, f32 delta_time);
  void (*update_global_world_state)(mat4 projection, mat4 view,
                                    vec3 view_position, vec4 ambient_color,
                                    i32 mode);
  void (*update_global_ui_state)(mat4 projection, mat4 view, i32 mode);
  bool (*end_frame)(renderer_backend *backend, f32 delta_time);

  bool (*begin_renderpass)(renderer_backend *backend, u8 renderpass_id);
  bool (*end_renderpass)(renderer_backend *backend, u8 renderpass_id);

  void (*draw_geometry)(geometry_render_data data);

  void (*create_texture)(robytes pixels, Texture *texture);
  void (*destroy_texture)(Texture *texture);

  bool (*create_material)(Material *material);
  void (*destroy_material)(Material *material);

  bool (*create_geometry)(Geometry *geometry, u32 vertex_size, u32 vertex_count,
                          roptr vertices, u32 index_size, u32 index_count,
                          roptr indices);
  void (*destroy_geometry)(Geometry *geometry);
};

struct render_packet {
  f32 delta_time;
  u32 geometry_count;
  geometry_render_data *geometries;
  u32 ui_geometry_count;
  geometry_render_data *ui_geometries;
};

} // namespace ns

#endif // RENDERER_TYPES_INLINE_INCLUDED
