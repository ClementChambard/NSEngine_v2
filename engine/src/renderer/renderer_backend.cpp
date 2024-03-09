#include "./renderer_backend.h"

#include "./vulkan/vulkan_backend.h"

namespace ns {

bool renderer_backend_create(renderer_backend_type type,
                             renderer_backend *out_renderer_backend) {
  if (type == RENDERER_BACKEND_TYPE_VULKAN) {
    out_renderer_backend->initialize = vulkan::renderer_backend_initialize;
    out_renderer_backend->shutdown = vulkan::renderer_backend_shutdown;
    out_renderer_backend->begin_frame = vulkan::renderer_backend_begin_frame;
    out_renderer_backend->update_global_state =
        vulkan::renderer_update_global_state;
    out_renderer_backend->end_frame = vulkan::renderer_backend_end_frame;
    out_renderer_backend->resized = vulkan::renderer_backend_on_resized;
    out_renderer_backend->create_texture = vulkan::renderer_create_texture;
    out_renderer_backend->destroy_texture = vulkan::renderer_destroy_texture;
    // temp
    out_renderer_backend->update_object = vulkan::renderer_update_object;
    return true;
  }

  return false;
}

void renderer_backend_destroy(renderer_backend *renderer_backend) {
  renderer_backend->initialize = nullptr;
  renderer_backend->shutdown = nullptr;
  renderer_backend->begin_frame = nullptr;
  renderer_backend->update_global_state = nullptr;
  renderer_backend->end_frame = nullptr;
  renderer_backend->resized = nullptr;
  renderer_backend->create_texture = nullptr;
  renderer_backend->destroy_texture = nullptr;
}

} // namespace ns
