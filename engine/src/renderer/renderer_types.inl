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

struct global_uniform_object {
  mat4 projection;
  mat4 view;
  mat4 m_reserved0;
  mat4 m_reserved1;
};

struct material_uniform_object {
  vec4 diffuse_color;
  vec4 v_reserved0;
  vec4 v_reserved1;
  vec4 v_reserved2;
  mat4 m_reserved3;
  mat4 m_reserved4;
  mat4 m_reserved5;
};

struct geometry_render_data {
  mat4 model;
  Geometry *geometry;
};

struct renderer_backend {
  u64 frame_number;

  bool (*initialize)(renderer_backend *backend, cstr application_name);

  void (*shutdown)(renderer_backend *backend);

  void (*resized)(renderer_backend *backend, u16 width, u16 height);

  bool (*begin_frame)(renderer_backend *backend, f32 delta_time);
  void (*update_global_state)(mat4 projection, mat4 view, vec3 view_position,
                              vec4 ambient_color, i32 mode);
  bool (*end_frame)(renderer_backend *backend, f32 delta_time);

  void (*draw_geometry)(geometry_render_data data);

  void (*create_texture)(robytes pixels, Texture *texture);
  void (*destroy_texture)(Texture *texture);

  bool (*create_material)(Material *material);
  void (*destroy_material)(Material *material);

  bool (*create_geometry)(Geometry *geometry, u32 vertex_count,
                          vertex_3d const *vertices, u32 index_count,
                          u32 const *indices);
  void (*destroy_geometry)(Geometry *geometry);
};

struct render_packet {
  f32 delta_time;
  u32 geometry_count;
  geometry_render_data *geometries;
};

} // namespace ns

#endif // RENDERER_TYPES_INLINE_INCLUDED
