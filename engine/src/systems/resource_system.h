#ifndef RESOURCE_SYSTEM_HEADER_INCLUDED
#define RESOURCE_SYSTEM_HEADER_INCLUDED

#include "../resources/resource_types.h"

namespace ns {

struct resource_system_config {
  u32 max_loader_count;
  cstr asset_base_path;
};

struct resource_loader {
  u32 id;
  ResourceType type;
  cstr custom_type;
  cstr type_path;

  bool (*load)(resource_loader *self, cstr name, Resource *out_resource);
  void (*unload)(resource_loader *self, Resource *resource);
};

bool resource_system_initialize(usize *memory_requirement, ptr state,
                                resource_system_config config);

void resource_system_shutdown(ptr state);

NS_API bool resource_system_register_loader(resource_loader loader);

NS_API bool resource_system_load(cstr name, ResourceType type,
                                 Resource *out_resource);

NS_API bool resource_system_load(cstr name, cstr custom_type,
                                 Resource *out_resource);

NS_API void resource_system_unload(Resource *resource);

NS_API cstr resource_system_base_path();

} // namespace ns

#endif // RESOURCE_SYSTEM_HEADER_INCLUDED
