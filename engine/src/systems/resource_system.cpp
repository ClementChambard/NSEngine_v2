#include "./resource_system.h"

#include "../core/ns_memory.h"
#include "../core/ns_string.h"

#include "../resources/loaders/binary_loader.h"
#include "../resources/loaders/image_loader.h"
#include "../resources/loaders/material_loader.h"
#include "../resources/loaders/text_loader.h"

namespace ns {

struct resource_system_state {
  resource_system_config config;
  resource_loader *registered_loaders;
};

static resource_system_state *state_ptr = nullptr;

bool load(cstr name, resource_loader *loader, Resource *out_resource);

bool resource_system_initialize(usize *memory_requirement, ptr state,
                                resource_system_config config) {
  if (config.max_loader_count == 0) {
    NS_FATAL("resource_system_initialize - Max loader count must be greater "
             "than 0");
    return false;
  }
  u64 struct_requirement = sizeof(resource_system_state);
  u64 array_requirement = sizeof(resource_loader) * config.max_loader_count;
  *memory_requirement = struct_requirement + array_requirement;
  if (state == nullptr) {
    return true;
  }
  state_ptr = reinterpret_cast<resource_system_state *>(state);
  state_ptr->config = config;

  ptr array_block = AS_BYTES(state) + struct_requirement;
  state_ptr->registered_loaders =
      reinterpret_cast<resource_loader *>(array_block);

  u32 count = config.max_loader_count;
  for (u32 i = 0; i < count; i++) {
    state_ptr->registered_loaders[i].id = INVALID_ID;
  }

  resource_system_register_loader(image_resource_loader_create());
  resource_system_register_loader(material_resource_loader_create());
  resource_system_register_loader(binary_resource_loader_create());
  resource_system_register_loader(text_resource_loader_create());

  NS_INFO("Resource system initialized with base path '%s'.",
          config.asset_base_path);

  return true;
}

void resource_system_shutdown(ptr /*state*/) {
  if (state_ptr) {
    state_ptr = nullptr;
  }
}

bool resource_system_register_loader(resource_loader loader) {
  if (!state_ptr) {
    return false;
  }
  u32 count = state_ptr->config.max_loader_count;
  for (u32 i = 0; i < count; i++) {
    resource_loader *l = &state_ptr->registered_loaders[i];
    if (l->id != INVALID_ID) {
      if (l->type == loader.type) {
        NS_ERROR("resource_system_register_loader - Loader of type %d already "
                 "exists and will not be registered.",
                 loader.type);
        return false;
      } else if (loader.custom_type && string_length(loader.custom_type) > 0 &&
                 string_EQ(l->custom_type, loader.custom_type)) {
        NS_ERROR("resource_system_register_loader - Loader of custom type %s "
                 "already "
                 "exists and will not be registered.",
                 loader.custom_type);
        return false;
      }
    }
  }
  for (u32 i = 0; i < count; i++) {
    if (state_ptr->registered_loaders[i].id == INVALID_ID) {
      state_ptr->registered_loaders[i] = loader;
      state_ptr->registered_loaders[i].id = i;
      NS_TRACE("Loader registered with id %d", i);
      return true;
    }
  }
  return false;
}

bool resource_system_load(cstr name, ResourceType type,
                          Resource *out_resource) {
  if (!state_ptr || type == ResourceType::CUSTOM) {
    out_resource->loader_id = INVALID_ID;
    NS_ERROR("resource_system_load - State pointer is null or invalid type ");
    return false;
  }
  for (u32 i = 0; i < state_ptr->config.max_loader_count; i++) {
    resource_loader *l = &state_ptr->registered_loaders[i];
    if (l->id != INVALID_ID && l->type == type) {
      return load(name, l, out_resource);
    }
  }
  NS_ERROR("resource_system_load - No loader found for type %d", type);
  return false;
}

bool resource_system_load(cstr name, cstr custom_type, Resource *out_resource) {
  if (!state_ptr || !custom_type || string_length(custom_type) < 1) {
    out_resource->loader_id = INVALID_ID;
    NS_ERROR("resource_system_load - State pointer is null or invalid type ");
    return false;
  }
  for (u32 i = 0; i < state_ptr->config.max_loader_count; i++) {
    resource_loader *l = &state_ptr->registered_loaders[i];
    if (l->id != INVALID_ID && l->type == ResourceType::CUSTOM &&
        string_EQ(l->custom_type, custom_type)) {
      return load(name, l, out_resource);
    }
  }
  NS_ERROR("resource_system_load - No loader found for custom type '%s'",
           custom_type);
  return false;
}

void resource_system_unload(Resource *resource) {
  if (!state_ptr || !resource || resource->loader_id == INVALID_ID) {
    return;
  }
  resource_loader *l = &state_ptr->registered_loaders[resource->loader_id];
  if (l->id != INVALID_ID && l->unload) {
    l->unload(l, resource);
  }
}

cstr resource_system_base_path() {
  if (!state_ptr) {
    NS_ERROR("resource_system_base_path - State pointer is null");
    return "";
  }
  return state_ptr->config.asset_base_path;
}

bool load(cstr name, resource_loader *loader, Resource *out_resource) {
  if (!state_ptr || !loader || !out_resource) {
    out_resource->loader_id = INVALID_ID;
    return false;
  }
  out_resource->loader_id = loader->id;
  return loader->load(loader, name, out_resource);
}

} // namespace ns
