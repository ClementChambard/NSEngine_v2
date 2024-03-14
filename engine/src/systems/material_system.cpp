#include "./material_system.h"

#include "../containers/hashtable.h"
#include "../core/logger.h"
#include "../core/ns_string.h"
#include "../math/ns_math.h"
#include "../renderer/renderer_frontend.h"
#include "./texture_system.h"

#include "./resource_system.h"

#include <new>

namespace ns {

struct material_system_state {
  material_system_config config;
  Material default_material;
  Material *registered_materials;
  chashtable registered_material_table;
};

struct material_reference {
  u64 reference_count;
  u32 handle;
  bool auto_release;
};

static material_system_state *state_ptr = nullptr;

bool create_default_material(material_system_state *state);
bool load_material(MaterialConfig config, Material *m);
void destroy_material(Material *m);

bool material_system_initialize(usize *memory_requirement, ptr state,
                                material_system_config config) {
  if (config.max_material_count == 0) {
    NS_FATAL("material_system_initialize - Max material count must be greater "
             "than 0");
    return false;
  }
  u64 struct_requirement = sizeof(material_system_state);
  u64 array_requirement = sizeof(Material) * config.max_material_count;
  u64 hashtable_requirement =
      sizeof(material_reference) * config.max_material_count;
  *memory_requirement =
      struct_requirement + array_requirement + hashtable_requirement;
  if (state == nullptr) {
    return true;
  }
  state_ptr = reinterpret_cast<material_system_state *>(state);
  state_ptr->config = config;

  ptr array_block = AS_BYTES(state) + struct_requirement;
  state_ptr->registered_materials = reinterpret_cast<Material *>(array_block);

  ptr hashtable_block = AS_BYTES(array_block) + array_requirement;
  new (&state_ptr->registered_material_table) chashtable(
      sizeof(material_reference), config.max_material_count, hashtable_block);

  material_reference invalid_ref;
  invalid_ref.auto_release = false;
  invalid_ref.handle = INVALID_ID;
  invalid_ref.reference_count = 0;
  state_ptr->registered_material_table.fill(&invalid_ref);

  u32 count = state_ptr->config.max_material_count;
  for (u32 i = 0; i < count; i++) {
    state_ptr->registered_materials[i].id = INVALID_ID;
    state_ptr->registered_materials[i].generation = INVALID_ID;
    state_ptr->registered_materials[i].internal_id = INVALID_ID;
  }

  if (!create_default_material(state_ptr)) {
    NS_FATAL("Failed to create default material. Application cannot continue.");
    return false;
  }

  return true;
}

void material_system_shutdown(ptr state) {
  material_system_state *s = reinterpret_cast<material_system_state *>(state);
  if (!s) {
    return;
  }
  u32 count = s->config.max_material_count;
  for (u32 i = 0; i < count; i++) {
    if (s->registered_materials[i].generation != INVALID_ID) {
      destroy_material(&s->registered_materials[i]);
    }
  }
  destroy_material(&s->default_material);
  state_ptr = nullptr;
}

Material *material_system_acquire(cstr name) {
  if (state_ptr == nullptr) {
    NS_FATAL("material_system_acquire - State pointer is null");
    return nullptr;
  }

  Resource material_resource;
  if (!resource_system_load(name, ResourceType::MATERIAL, &material_resource)) {
    NS_ERROR("material_system_acquire failed to acquire material '%s'. Null "
             "pointer will be returned.",
             name);
    return nullptr;
  }

  Material *m = nullptr;
  if (material_resource.data) {
    m = material_system_acquire(
        *reinterpret_cast<MaterialConfig *>(material_resource.data));
  }

  resource_system_unload(&material_resource);

  if (!m) {
    NS_ERROR("material_system_acquire failed to acquire material '%s'. Null "
             "pointer will be returned.",
             name);
  }

  return m;
}

Material *material_system_acquire(MaterialConfig config) {
  if (state_ptr == nullptr) {
    NS_FATAL("material_system_acquire - State pointer is null");
    return nullptr;
  }
  if (string_EQ(config.name, DEFAULT_MATERIAL_NAME)) {
    return &state_ptr->default_material;
  }

  material_reference ref{};
  if (!state_ptr->registered_material_table.get(config.name, &ref)) {
    NS_ERROR("material_system_acquire failed to acquire material '%s'. Null "
             "pointer will be returned.",
             config.name);
    return nullptr;
  }
  if (ref.reference_count == 0) {
    ref.auto_release = config.auto_release;
  }
  ref.reference_count++;
  if (ref.handle == INVALID_ID) {
    // no material here: create it
    u32 count = state_ptr->config.max_material_count;
    Material *m = nullptr;
    for (u32 i = 0; i < count; i++) {
      if (state_ptr->registered_materials[i].id == INVALID_ID) {
        ref.handle = i;
        m = &state_ptr->registered_materials[i];
        break;
      }
    }

    if (m == nullptr || ref.handle == INVALID_ID) {
      NS_ERROR("Failed to acquire material '%s'. No more material slots "
               "available.",
               config.name);
      return nullptr;
    }

    if (!load_material(config, m)) {
      NS_ERROR("Failed to load material '%s'.", config.name);
      return nullptr;
    }

    if (m->generation == INVALID_ID) {
      m->generation = 0;
    } else {
      m->generation++;
    }

    m->id = ref.handle;
    NS_TRACE("Material '%s' does not yet exist. ref count = %d", config.name,
             ref.reference_count);
  } else {
    NS_TRACE("Material '%s' already exists. ref count = %d", config.name,
             ref.reference_count);
  }

  state_ptr->registered_material_table.set(config.name, &ref);

  return &state_ptr->registered_materials[ref.handle];
}

void material_system_release(cstr name) {
  if (state_ptr == nullptr) {
    NS_FATAL("material_system_release - State pointer is null");
    return;
  }
  if (string_EQ(name, DEFAULT_MATERIAL_NAME)) {
    return;
  }
  material_reference ref{};
  if (!state_ptr->registered_material_table.get(name, &ref)) {
    NS_ERROR("material_system_release failed to release material '%s'.", name);
    return;
  }
  if (ref.reference_count == 0) {
    NS_WARN("Tried to release non-existent material: '%s'.", name);
    return;
  }
  ref.reference_count--;
  if (ref.reference_count == 0 && ref.auto_release) {
    Material *m = &state_ptr->registered_materials[ref.handle];
    destroy_material(m);
    ref.handle = INVALID_ID;
    ref.auto_release = false;
    NS_TRACE("Released material '%s'. Material unloaded because reference "
             "count = 0.",
             name);
  } else {
    NS_TRACE("Released material '%s'. ref count = %d", name,
             ref.reference_count);
  }

  state_ptr->registered_material_table.set(name, &ref);
}

Material *material_system_get_default() {
  if (state_ptr == nullptr) {
    NS_FATAL("material_system_get_default - State pointer is null");
    return nullptr;
  }
  return &state_ptr->default_material;
}

bool load_material(MaterialConfig config, Material *m) {
  mem_zero(m, sizeof(Material));

  string_ncpy(m->name, config.name, Material::NAME_MAX_LENGTH);

  m->type = config.type;

  m->diffuse_color = config.diffuse_color;

  if (string_length(config.diffuse_map_name) > 0) {
    m->diffuse_map.use = TextureUse::MAP_DIFFUSE;
    m->diffuse_map.texture =
        texture_system_acquire(config.diffuse_map_name, true);
    if (!m->diffuse_map.texture) {
      NS_WARN("Unable to load texture '%s' for material '%s', using default.",
              config.diffuse_map_name, config.name);
      m->diffuse_map.texture = texture_system_get_default_texture();
    }
  } else {
    m->diffuse_map.use = TextureUse::UNKNOWN;
    m->diffuse_map.texture = nullptr;
  }

  if (!renderer_create_material(m)) {
    NS_ERROR("Failed to acquire render resources for material '%s'.",
             config.name);
    return false;
  }

  return true;
}

void destroy_material(Material *m) {
  NS_TRACE("Destroying material '%s'", m->name);

  if (m->diffuse_map.texture) {
    texture_system_release(m->diffuse_map.texture->name);
  }

  renderer_destroy_material(m);

  mem_zero(m, sizeof(Material));
  m->id = INVALID_ID;
  m->generation = INVALID_ID;
  m->internal_id = INVALID_ID;
}

bool create_default_material(material_system_state *state) {
  if (state == nullptr) {
    return true;
  }
  mem_zero(&state->default_material, sizeof(Material));
  state->default_material.id = INVALID_ID;
  state->default_material.generation = INVALID_ID;
  string_ncpy(state->default_material.name, DEFAULT_MATERIAL_NAME,
              Material::NAME_MAX_LENGTH);
  state->default_material.diffuse_color = vec4(1.0f);
  state->default_material.diffuse_map.use = TextureUse::MAP_DIFFUSE;
  state->default_material.diffuse_map.texture =
      texture_system_get_default_texture();

  if (!renderer_create_material(&state->default_material)) {
    NS_FATAL("Failed to acquire render resources for default material.");
    return false;
  }

  return true;
}

} // namespace ns
