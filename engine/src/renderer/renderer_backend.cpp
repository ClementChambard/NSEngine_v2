#include "./renderer_backend.h"

#include "./vulkan/vulkan_backend.h"

namespace ns {

bool renderer_backend_create(renderer_backend_type type,
                             renderer_backend *out_renderer_backend) {
  if (type == RENDERER_BACKEND_TYPE_VULKAN) {
    out_renderer_backend->initialize = vulkan::backend_initialize;
    out_renderer_backend->shutdown = vulkan::backend_shutdown;
    out_renderer_backend->begin_frame = vulkan::backend_begin_frame;
    out_renderer_backend->update_global_world_state =
        vulkan::backend_update_global_world_state;
    out_renderer_backend->update_global_ui_state =
        vulkan::backend_update_global_ui_state;
    out_renderer_backend->end_frame = vulkan::backend_end_frame;
    out_renderer_backend->begin_renderpass = vulkan::backend_begin_renderpass;
    out_renderer_backend->end_renderpass = vulkan::backend_end_renderpass;
    out_renderer_backend->resized = vulkan::backend_on_resized;
    out_renderer_backend->create_texture = vulkan::backend_create_texture;
    out_renderer_backend->destroy_texture = vulkan::backend_destroy_texture;
    out_renderer_backend->create_material = vulkan::backend_create_material;
    out_renderer_backend->destroy_material = vulkan::backend_destroy_material;
    out_renderer_backend->create_geometry = vulkan::backend_create_geometry;
    out_renderer_backend->destroy_geometry = vulkan::backend_destroy_geometry;
    out_renderer_backend->draw_geometry = vulkan::backend_draw_geometry;
    return true;
  }

  return false;
}

void renderer_backend_destroy(renderer_backend *renderer_backend) {
  renderer_backend->initialize = nullptr;
  renderer_backend->shutdown = nullptr;
  renderer_backend->begin_frame = nullptr;
  renderer_backend->update_global_world_state = nullptr;
  renderer_backend->update_global_ui_state = nullptr;
  renderer_backend->end_frame = nullptr;
  renderer_backend->begin_renderpass = nullptr;
  renderer_backend->end_renderpass = nullptr;
  renderer_backend->resized = nullptr;
  renderer_backend->create_texture = nullptr;
  renderer_backend->destroy_texture = nullptr;
  renderer_backend->create_material = nullptr;
  renderer_backend->destroy_material = nullptr;
  renderer_backend->create_geometry = nullptr;
  renderer_backend->destroy_geometry = nullptr;
  renderer_backend->draw_geometry = nullptr;
}

} // namespace ns
