#include "./image_loader.h"

#include "../../core/logger.h"
#include "../../core/memory.h"
#include "../../core/string.h"
#include "../../systems/resource_system.h"
#include "../resource_types.h"
#include "./loader_utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace ns {

bool image_loader_load(resource_loader *self, cstr name,
                       Resource *out_resource) {
  if (!self || !name || !out_resource) {
    return false;
  }

  cstr format_str = "%s/%s/%s%s";
  const i32 required_channel_count = 4;
  stbi_set_flip_vertically_on_load(true);
  char full_file_path[512];

  string_fmt(full_file_path, sizeof(full_file_path), format_str,
             resource_system_base_path(), self->type_path, name, ".png");

  i32 width;
  i32 height;
  i32 channel_count;

  u8 *data = stbi_load(full_file_path, &width, &height, &channel_count,
                       required_channel_count);

  cstr fail_reason = stbi_failure_reason();
  if (fail_reason) {
    NS_ERROR("image_loader_load - Failed to load image '%s': %s",
             full_file_path, fail_reason);
    stbi__err(0, 0);

    if (data) {
      stbi_image_free(data);
    }
    return false;
  }

  if (!data) {
    NS_ERROR("image_loader_load - Failed to load image '%s'", full_file_path);
    return false;
  }

  out_resource->full_path = string_dup(full_file_path);

  ImageResourceData *resource_data = reinterpret_cast<ImageResourceData *>(
      ns::alloc(sizeof(ImageResourceData), MemTag::TEXTURE));
  resource_data->width = width;
  resource_data->height = height;
  resource_data->channel_count = channel_count;
  resource_data->pixels = data;

  out_resource->data = resource_data;
  out_resource->data_size = sizeof(ImageResourceData);
  out_resource->name = name;

  return true;
}

void image_loader_unload(resource_loader *self, Resource *resource) {
  if (!self || !resource) {
    NS_WARN("image_loader_unload - Loader or resource is null");
    return;
  }

  i32 path_length = string_length(resource->full_path);
  if (path_length) {
    ns::free(resource->full_path, sizeof(char) * path_length + 1,
             MemTag::STRING);
  }

  if (resource->data) {
    stbi_image_free(
        reinterpret_cast<ImageResourceData *>(resource->data)->pixels);
    ns::free(resource->data, resource->data_size, MemTag::TEXTURE);
    resource->data = nullptr;
    resource->data_size = 0;
    resource->loader_id = INVALID_ID;
  }
}

resource_loader image_resource_loader_create() {
  resource_loader loader;
  loader.type = ResourceType::IMAGE;
  loader.type_path = "textures";
  loader.custom_type = nullptr;
  loader.load = image_loader_load;
  loader.unload = image_loader_unload;
  return loader;
}

} // namespace ns
