#ifndef GEOMETRY_SYSTEM_HEADER_INCLUDED
#define GEOMETRY_SYSTEM_HEADER_INCLUDED

#include "../renderer/renderer_types.inl"

namespace ns {

struct geometry_system_config {
  u32 max_geometry_count;
};

struct geometry_config {
  u32 vertex_size;
  u32 vertex_count;
  ptr vertices;
  u32 index_size;
  u32 index_count;
  ptr indices;
  char name[Geometry::NAME_MAX_LENGTH];
  char material_name[Material::NAME_MAX_LENGTH];

  static geometry_config plane(f32 width, f32 height, u32 x_segment_count,
                               u32 y_segment_count, f32 tile_x, f32 tile_y,
                               cstr name, cstr material_name);
};

#define DEFAULT_GEOMETRY_NAME "default"

bool geometry_system_initialize(u64 *memory_requirement, ptr state,
                                geometry_system_config config);

void geometry_system_shutdown(ptr state);

Geometry *geometry_system_acquire(NSID id);

Geometry *geometry_system_acquire(geometry_config config, bool auto_release);

void geometry_system_release(Geometry *geometry);

Geometry *geometry_system_get_default();

Geometry *geometry_system_get_default_2d();

} // namespace ns

#endif // GEOMETRY_SYSTEM_HEADER_INCLUDED
