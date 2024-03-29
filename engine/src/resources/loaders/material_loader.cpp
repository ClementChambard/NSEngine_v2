#include "./material_loader.h"

#include "../../core/logger.h"
#include "../../core/memory.h"
#include "../../core/string.h"
#include "../../math/math.h"
#include "../../systems/resource_system.h"
#include "../resource_types.h"
#include "./loader_utils.h"

#include "../../platform/filesystem.h"

namespace ns {

bool material_loader_load(resource_loader *self, cstr name,
                          Resource *out_resource) {
  if (!self || !name || !out_resource) {
    return false;
  }

  cstr format_str = "%s/%s/%s%s";
  char full_file_path[512];

  string_fmt(full_file_path, sizeof(full_file_path), format_str,
             resource_system_base_path(), self->type_path, name, ".nsmt");

  fs::File f;
  if (!fs::open(full_file_path, fs::Mode::READ, false, &f)) {
    NS_ERROR("material_loader_load - Failed to open material file '%s'",
             full_file_path);
    return false;
  }

  out_resource->full_path = string_dup(full_file_path);

  MaterialConfig *resource_data = reinterpret_cast<MaterialConfig *>(
      ns::alloc(sizeof(MaterialConfig), MemTag::MATERIAL_INSTANCE));

  resource_data->auto_release = true;
  resource_data->diffuse_color = vec4(1.0f);
  resource_data->diffuse_map_name[0] = '\0';
  string_ncpy(resource_data->name, name, Material::NAME_MAX_LENGTH);

  char linebuf[512] = "";
  pstr p = &linebuf[0];
  usize linelen = 0;
  u32 linenum = 1;
  while (fs::read_line(&f, 511, &p, &linelen)) {
    pstr line = string_trim(p);
    linelen = string_length(line);

    if (linelen < 1 || line[0] == '#') {
      linenum++;
      continue;
    }

    isize equal_index = string_indexof(line, '=');
    if (equal_index == -1) {
      NS_WARN("Potential formatting issue found in '%s:%d': '=' not found.",
              full_file_path, linenum);
      linenum++;
      continue;
    }

    char raw_var_name[64];
    mem_zero(raw_var_name, sizeof(raw_var_name));
    string_sub(raw_var_name, line, 0, equal_index);
    pstr var_name = string_trim(raw_var_name);

    char raw_var_value[446];
    mem_zero(raw_var_value, sizeof(raw_var_value));
    string_sub(raw_var_value, line, equal_index + 1, -1);
    pstr var_value = string_trim(raw_var_value);

    if (string_EQ(var_name, "version")) {
      // TODO(ClementChambard)
    } else if (string_EQ(var_name, "name")) {
      string_ncpy(resource_data->name, var_value, Material::NAME_MAX_LENGTH);
    } else if (string_EQ(var_name, "diffuse_color")) {
      if (!resource_data->diffuse_color.from(var_value)) {
        NS_WARN(
            "Error parsing diffuse_color in file '%s'. Using white instead.",
            full_file_path);
      }
    } else if (string_EQ(var_name, "diffuse_map_name")) {
      string_ncpy(resource_data->diffuse_map_name, var_value,
                  Texture::NAME_MAX_LENGTH);
    } else if (string_EQ(var_name, "type")) {
      if (string_EQ(var_value, "ui")) {
        resource_data->type = MaterialType::UI;
      }
    }
    // TODO(ClementChambard): more

    mem_zero(linebuf, sizeof(linebuf));
    linenum++;
  }

  fs::close(&f);

  out_resource->data = resource_data;
  out_resource->data_size = sizeof(MaterialConfig);
  out_resource->name = name;

  return true;
}

void material_loader_unload(resource_loader *self, Resource *resource) {
  resource_unload(self, resource, MemTag::MATERIAL_INSTANCE);
}

resource_loader material_resource_loader_create() {
  resource_loader l;
  l.type = ResourceType::MATERIAL;
  l.custom_type = nullptr;
  l.type_path = "materials";
  l.load = material_loader_load;
  l.unload = material_loader_unload;
  return l;
}

} // namespace ns
