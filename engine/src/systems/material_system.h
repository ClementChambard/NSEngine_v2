
#ifndef MATERIAL_SYSTEM_HEADER_INCLUDED
#define MATERIAL_SYSTEM_HEADER_INCLUDED

#include "../defines.h"
#include "../resources/resource_types.h"

#define DEFAULT_MATERIAL_NAME "default"

namespace ns {

struct material_system_config {
  u32 max_material_count;
};

bool material_system_initialize(usize *memory_requirement, ptr state,
                                material_system_config config);

void material_system_shutdown(ptr state);

Material *material_system_acquire(cstr name);
Material *material_system_acquire(MaterialConfig config);
void material_system_release(cstr name);

Material *material_system_get_default();

} // namespace ns

#endif // MATERIAL_SYSTEM_HEADER_INCLUDED
