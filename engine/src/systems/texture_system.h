#ifndef TEXTURE_SYSTEM_HEADER_INCLUDED
#define TEXTURE_SYSTEM_HEADER_INCLUDED

#include "../renderer/renderer_types.inl"

namespace ns {

struct texture_system_config {
  u32 max_texture_count;
};

#define DEFAULT_TEXTURE_NAME "default"

bool texture_system_initialize(usize *memory_requirement, ptr state,
                               texture_system_config config);

void texture_system_shutdown(ptr state);

Texture *texture_system_acquire(cstr name, bool auto_release);

void texture_system_release(cstr name);

Texture *texture_system_get_default_texture();

} // namespace ns

#endif // TEXTURE_SYSTEM_HEADER_INCLUDED
