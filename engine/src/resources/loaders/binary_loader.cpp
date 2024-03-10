#include "./binary_loader.h"

#include "../../core/logger.h"
#include "../../core/ns_memory.h"
#include "../../core/ns_string.h"
#include "../../systems/resource_system.h"
#include "../resource_types.h"

#include "../../platform/filesystem.h"

namespace ns {

bool binary_loader_load(resource_loader *self, cstr name,
                        Resource *out_resource) {
  if (!self || !name || !out_resource) {
    return false;
  }

  cstr format_str = "%s/%s/%s%s";
  char full_file_path[512];

  string_fmt(full_file_path, sizeof(full_file_path), format_str,
             resource_system_base_path(), self->type_path, name, "");

  out_resource->full_path = string_dup(full_file_path);

  fs::File f;
  if (!fs::open(full_file_path, fs::Mode::READ, true, &f)) {
    NS_ERROR("binary_loader_load - Failed to open binary file '%s'",
             full_file_path);
    return false;
  }

  usize file_size = 0;
  if (!fs::fsize(&f, &file_size)) {
    NS_ERROR("binary_loader_load - Failed to get size of binary file '%s'",
             full_file_path);
    return false;
  }

  bytes resource_data =
      reinterpret_cast<bytes>(ns::alloc(file_size, mem_tag::ARRAY));
  usize read_size = 0;
  if (!fs::read_all_bytes(&f, resource_data, &read_size)) {
    NS_ERROR("binary_loader_load - Failed to read binary file '%s'",
             full_file_path);
    fs::close(&f);
    return false;
  }

  fs::close(&f);

  out_resource->data = resource_data;
  out_resource->data_size = file_size;
  out_resource->name = name;

  return true;
}

void binary_loader_unload(resource_loader *self, Resource *resource) {
  if (!self || !resource) {
    NS_WARN("binary_loader_unload - Loader or resource is null");
    return;
  }

  i32 path_length = string_length(resource->full_path);
  if (path_length) {
    ns::free(resource->full_path, sizeof(char) * path_length + 1,
             mem_tag::STRING);
  }

  if (resource->data) {
    ns::free(resource->data, resource->data_size, mem_tag::ARRAY);
    resource->data = nullptr;
    resource->data_size = 0;
    resource->loader_id = INVALID_ID;
  }
}

resource_loader binary_resource_loader_create() {
  resource_loader l;
  l.type = ResourceType::BINARY;
  l.custom_type = nullptr;
  l.type_path = "";
  l.load = binary_loader_load;
  l.unload = binary_loader_unload;
  return l;
}

} // namespace ns
