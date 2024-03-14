#include "./text_loader.h"

#include "../../core/logger.h"
#include "../../core/ns_memory.h"
#include "../../core/ns_string.h"
#include "../../systems/resource_system.h"
#include "../resource_types.h"
#include "./loader_utils.h"

#include "../../platform/filesystem.h"

namespace ns {

bool text_loader_load(resource_loader *self, cstr name,
                      Resource *out_resource) {
  if (!self || !name || !out_resource) {
    return false;
  }

  cstr format_str = "%s/%s/%s%s";
  char full_file_path[512];

  string_fmt(full_file_path, sizeof(full_file_path), format_str,
             resource_system_base_path(), self->type_path, name, "");

  fs::File f;
  if (!fs::open(full_file_path, fs::Mode::READ, false, &f)) {
    NS_ERROR("text_loader_load - Failed to open text file '%s'",
             full_file_path);
    return false;
  }

  out_resource->full_path = string_dup(full_file_path);

  usize file_size = 0;
  if (!fs::fsize(&f, &file_size)) {
    NS_ERROR("text_loader_load - Failed to get size of text file '%s'",
             full_file_path);
    return false;
  }

  str resource_data = reinterpret_cast<str>(
      ns::alloc(file_size * sizeof(char), mem_tag::ARRAY));
  usize read_size = 0;
  if (!fs::read_all_text(&f, resource_data, &read_size)) {
    NS_ERROR("text_loader_load - Failed to read text file '%s'",
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

void text_loader_unload(resource_loader *self, Resource *resource) {
  resource_unload(self, resource, mem_tag::ARRAY);
}

resource_loader text_resource_loader_create() {
  resource_loader l;
  l.type = ResourceType::TEXT;
  l.custom_type = nullptr;
  l.type_path = "";
  l.load = text_loader_load;
  l.unload = text_loader_unload;
  return l;
}

} // namespace ns
