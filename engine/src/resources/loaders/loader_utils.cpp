#include "./loader_utils.h"

namespace ns {

void resource_unload(resource_loader *self, Resource *resource, MemTag tag) {
  if (!self || !resource) {
    NS_WARN("resource_unload - Loader or resource is null");
    return;
  }
  u32 path_len = string_length(resource->full_path);
  if (path_len) {
    ns::free(resource->full_path, sizeof(char) * path_len + 1, MemTag::STRING);
  }

  if (resource->data) {
    ns::free(resource->data, resource->data_size, tag);
    resource->data = nullptr;
    resource->data_size = 0;
    resource->loader_id = INVALID_ID;
  }
}

} // namespace ns
