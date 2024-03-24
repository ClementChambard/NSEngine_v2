#include "./geometry_system.h"

#include "../core/logger.h"
#include "../core/memory.h"
#include "../core/string.h"
#include "../renderer/renderer_frontend.h"
#include "./material_system.h"

namespace ns {

struct geometry_reference {
  u64 reference_count;
  Geometry geometry;
  bool auto_release;
};

struct geometry_system_state {
  geometry_system_config config;
  Geometry default_geometry;
  Geometry default_2d_geometry;
  geometry_reference *registered_geometries;
};

static geometry_system_state *state_ptr = nullptr;

bool create_default_geometries(geometry_system_state *state);
bool create_geometry(geometry_system_state *state, geometry_config config,
                     Geometry *g);
void destroy_geometry(geometry_system_state *state, Geometry *g);

bool geometry_system_initialize(u64 *memory_requirement, ptr state,
                                geometry_system_config config) {
  if (config.max_geometry_count == 0) {
    NS_FATAL("geometry_system_initialize - Max geometry count must be greater "
             "than 0");
    return false;
  }
  u64 struct_requirement = sizeof(geometry_system_state);
  u64 array_requirement =
      sizeof(geometry_reference) * config.max_geometry_count;
  *memory_requirement = struct_requirement + array_requirement;
  if (state == nullptr) {
    return true;
  }
  state_ptr = reinterpret_cast<geometry_system_state *>(state);
  state_ptr->config = config;
  ptr array_block = AS_BYTES(state) + struct_requirement;
  state_ptr->registered_geometries =
      reinterpret_cast<geometry_reference *>(array_block);

  u32 count = state_ptr->config.max_geometry_count;
  for (u32 i = 0; i < count; i++) {
    state_ptr->registered_geometries[i].geometry.id = INVALID_ID;
    state_ptr->registered_geometries[i].geometry.internal_id = INVALID_ID;
    state_ptr->registered_geometries[i].geometry.generation = INVALID_ID;
  }

  if (!create_default_geometries(state_ptr)) {
    NS_FATAL("Could not create default geometries");
    return false;
  }
  return true;
}

void geometry_system_shutdown(ptr /*state*/) {}

Geometry *geometry_system_acquire(NSID id) {
  if (id != INVALID_ID &&
      state_ptr->registered_geometries[id].geometry.id != INVALID_ID) {
    state_ptr->registered_geometries[id].reference_count++;
    return &state_ptr->registered_geometries[id].geometry;
  }
  NS_ERROR("geometry_system_acquire - Invalid geometry id. Returning nullptr");
  return nullptr;
}

Geometry *geometry_system_acquire(geometry_config config, bool auto_release) {
  Geometry *g = nullptr;
  for (u32 i = 0; i < state_ptr->config.max_geometry_count; i++) {
    if (state_ptr->registered_geometries[i].geometry.id == INVALID_ID) {
      state_ptr->registered_geometries[i].auto_release = auto_release;
      state_ptr->registered_geometries[i].reference_count = 1;
      g = &state_ptr->registered_geometries[i].geometry;
      g->id = i;
      break;
    }
  }
  if (g == nullptr) {
    NS_FATAL("geometry_system_acquire - Could not acquire geometry: no free "
             "slot available");
    return nullptr;
  }
  if (!create_geometry(state_ptr, config, g)) {
    NS_FATAL("geometry_system_acquire - Could not create geometry");
    return nullptr;
  }
  return g;
}

void geometry_system_release(Geometry *geometry) {
  if (geometry == nullptr || geometry->id == INVALID_ID) {
    return;
  }
  geometry_reference *ref = &state_ptr->registered_geometries[geometry->id];

  if (ref->geometry.id != geometry->id) {
    NS_FATAL("geometry_system_release - Geometry id does not match");
    return;
  }
  if (ref->reference_count > 0) {
    ref->reference_count--;
  }

  if (ref->reference_count < 1 && ref->auto_release) {
    destroy_geometry(state_ptr, &ref->geometry);
    ref->reference_count = 0;
    ref->auto_release = false;
  }
}

Geometry *geometry_system_get_default() {
  if (state_ptr == nullptr) {
    NS_FATAL("geometry_system_get_default - State pointer is null");
    return nullptr;
  }
  return &state_ptr->default_geometry;
}

Geometry *geometry_system_get_default_2d() {
  if (state_ptr == nullptr) {
    NS_FATAL("geometry_system_get_default_2d - State pointer is null");
    return nullptr;
  }
  return &state_ptr->default_2d_geometry;
}

geometry_config geometry_config::plane(f32 width, f32 height,
                                       u32 x_segment_count, u32 y_segment_count,
                                       f32 tile_x, f32 tile_y, cstr name,
                                       cstr material_name) {
  if (width == 0.0f) {
    NS_WARN("geometry_config::plane - Width must be greater than 0");
    width = 1.0f;
  }
  if (height == 0.0f) {
    NS_WARN("geometry_config::plane - Height must be greater than 0");
    height = 1.0f;
  }
  if (x_segment_count < 1) {
    NS_WARN("geometry_config::plane - X segment count must be greater than 0");
    x_segment_count = 1;
  }
  if (y_segment_count < 1) {
    NS_WARN("geometry_config::plane - Y segment count must be greater than 0");
    y_segment_count = 1;
  }

  if (tile_x == 0.0f) {
    NS_WARN("geometry_config::plane - Tile X must be greater than 0");
    tile_x = 1.0f;
  }
  if (tile_y == 0.0f) {
    NS_WARN("geometry_config::plane - Tile Y must be greater than 0");
    tile_y = 1.0f;
  }

  geometry_config config;
  config.vertex_size = sizeof(vertex_3d);
  config.vertex_count = x_segment_count * y_segment_count * 4;
  config.vertices = reinterpret_cast<vertex_3d *>(
      ns::alloc(sizeof(vertex_3d) * config.vertex_count, MemTag::ARRAY));
  config.index_size = sizeof(u32);
  config.index_count = x_segment_count * y_segment_count * 6;
  config.indices = reinterpret_cast<u32 *>(
      ns::alloc(sizeof(u32) * config.index_count, MemTag::ARRAY));

  f32 seg_width = width / static_cast<f32>(x_segment_count);
  f32 seg_height = height / static_cast<f32>(y_segment_count);
  f32 half_width = width / 2.0f;
  f32 half_height = height / 2.0f;
  for (u32 y = 0; y < y_segment_count; y++) {
    for (u32 x = 0; x < x_segment_count; x++) {
      f32 min_x = (x * seg_width) - half_width;
      f32 min_y = (y * seg_height) - half_height;
      f32 max_x = min_x + seg_width;
      f32 max_y = min_y + seg_height;
      f32 min_uvx = (x / static_cast<f32>(x_segment_count)) * tile_x;
      f32 min_uvy = (y / static_cast<f32>(y_segment_count)) * tile_y;
      f32 max_uvx = ((x + 1) / static_cast<f32>(x_segment_count)) * tile_x;
      f32 max_uvy = ((y + 1) / static_cast<f32>(y_segment_count)) * tile_y;

      u32 v_offset = ((y * x_segment_count) + x) * 4;
      vertex_3d *vertices = reinterpret_cast<vertex_3d *>(config.vertices);
      vertex_3d *v0 = &vertices[v_offset + 0];
      vertex_3d *v1 = &vertices[v_offset + 1];
      vertex_3d *v2 = &vertices[v_offset + 2];
      vertex_3d *v3 = &vertices[v_offset + 3];

      v0->position = {min_x, min_y, 0.0f};
      v0->texcoord = {min_uvx, min_uvy};
      v1->position = {max_x, max_y, 0.0f};
      v1->texcoord = {max_uvx, max_uvy};
      v2->position = {min_x, max_y, 0.0f};
      v2->texcoord = {min_uvx, max_uvy};
      v3->position = {max_x, min_y, 0.0f};
      v3->texcoord = {max_uvx, min_uvy};

      u32 i_offset = ((y * x_segment_count) + x) * 6;
      u32 *indices = reinterpret_cast<u32 *>(config.indices);
      indices[i_offset + 0] = v_offset + 0;
      indices[i_offset + 1] = v_offset + 1;
      indices[i_offset + 2] = v_offset + 2;
      indices[i_offset + 3] = v_offset + 0;
      indices[i_offset + 4] = v_offset + 3;
      indices[i_offset + 5] = v_offset + 1;
    }
  }

  if (name && string_length(name) > 0) {
    string_ncpy(config.name, name, Geometry::NAME_MAX_LENGTH);
  } else {
    string_ncpy(config.name, DEFAULT_GEOMETRY_NAME, Geometry::NAME_MAX_LENGTH);
  }

  if (material_name && string_length(material_name) > 0) {
    string_ncpy(config.material_name, material_name, Material::NAME_MAX_LENGTH);
  } else {
    string_ncpy(config.material_name, DEFAULT_MATERIAL_NAME,
                Material::NAME_MAX_LENGTH);
  }

  return config;
}

bool create_geometry(geometry_system_state *state, geometry_config config,
                     Geometry *g) {
  if (!renderer_create_geometry(g, config.vertex_size, config.vertex_count,
                                config.vertices, config.index_size,
                                config.index_count, config.indices)) {
    state->registered_geometries[g->id].reference_count = 0;
    state->registered_geometries[g->id].auto_release = false;
    g->id = INVALID_ID;
    g->generation = INVALID_ID;
    g->internal_id = INVALID_ID;
    return false;
  }

  if (string_length(config.material_name) > 0) {
    g->material = material_system_acquire(config.material_name);
    if (!g->material) {
      g->material = material_system_get_default();
    }
  }

  return true;
}

void destroy_geometry(geometry_system_state * /*state*/, Geometry *g) {
  renderer_destroy_geometry(g);
  g->internal_id = INVALID_ID;
  g->id = INVALID_ID;
  g->generation = INVALID_ID;

  string_clear(g->name);

  if (g->material && string_length(g->material->name) > 0) {
    material_system_release(g->material->name);
    g->material = nullptr;
  }
}

bool create_default_geometries(geometry_system_state *state) {
  vertex_3d verts[4];
  mem_zero(verts, sizeof(verts));

  const f32 f = 10.0f;

  verts[0].position = {-0.5f * f, -0.5f * f, 0.0f};
  verts[0].texcoord = {0.0f, 0.0f};
  verts[1].position = {0.5f * f, 0.5f * f, 0.0f};
  verts[1].texcoord = {1.0f, 1.0f};
  verts[2].position = {-0.5f * f, 0.5f * f, 0.0f};
  verts[2].texcoord = {0.0f, 1.0f};
  verts[3].position = {0.5f * f, -0.5f * f, 0.0f};
  verts[3].texcoord = {1.0f, 0.0f};

  u32 indices[6] = {0, 1, 2, 0, 3, 1};

  state->default_geometry.id = INVALID_ID;
  state->default_geometry.generation = INVALID_ID;
  state->default_geometry.internal_id = INVALID_ID;
  if (!renderer_create_geometry(&state->default_geometry, sizeof(vertex_3d), 4,
                                verts, sizeof(u32), 6, indices)) {
    NS_FATAL("Failed to create default geometry.");
    return false;
  }

  state->default_geometry.material = material_system_get_default();

  vertex_2d verts2[4];
  mem_zero(verts2, sizeof(verts2));

  verts2[0].position = {-0.5f * f, -0.5f * f};
  verts2[0].texcoord = {0.0f, 0.0f};
  verts2[1].position = {0.5f * f, 0.5f * f};
  verts2[1].texcoord = {1.0f, 1.0f};
  verts2[2].position = {-0.5f * f, 0.5f * f};
  verts2[2].texcoord = {0.0f, 1.0f};
  verts2[3].position = {0.5f * f, -0.5f * f};
  verts2[3].texcoord = {1.0f, 0.0f};

  u32 indices2[6] = {2, 1, 0, 3, 0, 1};

  if (!renderer_create_geometry(&state->default_2d_geometry, sizeof(vertex_2d),
                                4, verts2, sizeof(u32), 6, indices2)) {
    NS_FATAL("Failed to create default geometry.");
    return false;
  }

  state->default_2d_geometry.material = material_system_get_default();

  return true;
}

} // namespace ns
