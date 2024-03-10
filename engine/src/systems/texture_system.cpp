#include "./texture_system.h"

#include "../containers/hashtable.h"
#include "../core/logger.h"
#include "../core/ns_memory.h"
#include "../core/ns_string.h"

#include "../renderer/renderer_frontend.h"

#include <new>

#include "./resource_system.h"

namespace ns {

struct texture_system_state {
  texture_system_config config;
  Texture default_texture;

  Texture *registered_textures;

  chashtable registered_textures_table;
};

struct texture_reference {
  u64 reference_count;
  u32 handle;
  bool auto_release;
};

static texture_system_state *state_ptr = nullptr;

bool create_default_textures(texture_system_state *state);
void destroy_default_textures(texture_system_state *state);
bool load_texture(cstr texture_name, Texture *t);
void destroy_texture(Texture *t);

bool texture_system_initialize(usize *memory_requirement, ptr state,
                               texture_system_config config) {
  if (config.max_texture_count == 0) {
    NS_FATAL(
        "texture_system_initialize - Max texture count must be greater than 0");
    return false;
  }
  u64 struct_requirement = sizeof(texture_system_state);
  u64 array_requirement = sizeof(Texture) * config.max_texture_count;
  u64 hashtable_requirement =
      sizeof(texture_reference) * config.max_texture_count;
  *memory_requirement =
      struct_requirement + array_requirement + hashtable_requirement;
  if (state == nullptr) {
    return true;
  }
  state_ptr = reinterpret_cast<texture_system_state *>(state);
  state_ptr->config = config;

  ptr array_block = AS_BYTES(state) + struct_requirement;
  state_ptr->registered_textures = reinterpret_cast<Texture *>(array_block);

  ptr hashtable_block = AS_BYTES(array_block) + array_requirement;
  new (&state_ptr->registered_textures_table) chashtable(
      sizeof(texture_reference), config.max_texture_count, hashtable_block);

  texture_reference invalid_ref;
  invalid_ref.auto_release = false;
  invalid_ref.handle = INVALID_ID;
  invalid_ref.reference_count = 0;
  state_ptr->registered_textures_table.fill(&invalid_ref);

  u32 count = state_ptr->config.max_texture_count;
  for (u32 i = 0; i < count; ++i) {
    state_ptr->registered_textures[i].id = INVALID_ID;
    state_ptr->registered_textures[i].generation = INVALID_ID;
  }

  create_default_textures(state_ptr);

  return true;
}

void texture_system_shutdown(ptr /*state*/) {
  if (state_ptr == nullptr) {
    return;
  }
  for (u32 i = 0; i < state_ptr->config.max_texture_count; i++) {
    Texture *t = &state_ptr->registered_textures[i];
    if (t->generation != INVALID_ID) {
      renderer_destroy_texture(t);
    }
  }
  destroy_default_textures(state_ptr);
  state_ptr = nullptr;
}

Texture *texture_system_acquire(cstr name, bool auto_release) {
  if (state_ptr == nullptr) {
    NS_FATAL("texture_system_acquire - State pointer is null");
    return nullptr;
  }
  if (string_EQ(name, DEFAULT_TEXTURE_NAME)) {
    NS_WARN("texture_system_acquire called for default texture. Use "
            "texture_system_get_default_texture instead.");
    return &state_ptr->default_texture;
  }

  texture_reference ref{};
  if (!state_ptr->registered_textures_table.get(name, &ref)) {
    NS_ERROR("texture_system_acquire failed to acquire texture '%s'. Null "
             "pointer will be returned.",
             name);
    return nullptr;
  }
  if (ref.reference_count == 0) {
    ref.auto_release = auto_release;
  }
  ref.reference_count++;
  if (ref.handle == INVALID_ID) {
    // no texture here: create it
    u32 count = state_ptr->config.max_texture_count;
    Texture *t = nullptr;
    for (u32 i = 0; i < count; i++) {
      if (state_ptr->registered_textures[i].id == INVALID_ID) {
        ref.handle = i;
        t = &state_ptr->registered_textures[i];
        break;
      }
    }
    if (t == nullptr || ref.handle == INVALID_ID) {
      NS_FATAL("texture_system_acquire - Max texture count reached. Adjust "
               "config to allow more.");
      return nullptr;
    }
    if (!load_texture(name, t)) {
      NS_ERROR("Failed to load texture '%s'.", name);
      return nullptr;
    }
    t->id = ref.handle;
    NS_TRACE("Texture '%s' created, and ref_count is now %i.", name,
             ref.reference_count);
  } else {
    NS_TRACE("Texture '%s' exists. ref_count increased to %i.", name,
             ref.reference_count);
  }

  state_ptr->registered_textures_table.set(name, &ref);

  return &state_ptr->registered_textures[ref.handle];
}

void texture_system_release(cstr name) {
  if (state_ptr == nullptr) {
    NS_FATAL("texture_system_release - State pointer is null");
    return;
  }
  if (string_EQ(name, DEFAULT_TEXTURE_NAME)) {
    return;
  }
  texture_reference ref{};
  if (!state_ptr->registered_textures_table.get(name, &ref)) {
    NS_ERROR("texture_system_release failed to release texture '%s'.", name);
    return;
  }
  if (ref.reference_count == 0) {
    NS_WARN("Tried to release non-existent texture: '%s'.", name);
    return;
  }
  char name_copy[Texture::NAME_MAX_LENGTH];
  string_ncpy(name_copy, name, Texture::NAME_MAX_LENGTH);

  ref.reference_count--;
  if (ref.reference_count == 0 && ref.auto_release) {
    Texture *t = &state_ptr->registered_textures[ref.handle];

    destroy_texture(t);

    ref.handle = INVALID_ID;
    ref.auto_release = false;
    NS_TRACE(
        "Released texture '%s'. Texture unloaded because reference count = 0.",
        name_copy);
  } else {
    NS_TRACE("Released texture '%s'. Reference count decreased to %i.",
             name_copy, ref.reference_count);
  }

  state_ptr->registered_textures_table.set(name_copy, &ref);
}

Texture *texture_system_get_default_texture() {
  if (state_ptr == nullptr) {
    NS_ERROR("texture_system_get_default_texture - State pointer is null");
    return nullptr;
  }
  return &state_ptr->default_texture;
}

bool create_default_textures(texture_system_state *state) {
  NS_TRACE("Creating default texture...");
  const u32 tex_dimension = 256;
  const u32 channels = 4;
  const u32 pixel_count = tex_dimension * tex_dimension;
  u8 pixels[pixel_count * channels];
  mem_set(pixels, 255, sizeof(u8) * pixel_count * channels);
  for (usize row = 0; row < tex_dimension; row++) {
    for (usize col = 0; col < tex_dimension; col++) {
      usize index = row * tex_dimension + col;
      usize index_bpp = index * channels;
      if (row % 2) {
        if (col % 2) {
          pixels[index_bpp + 0] = 0;
          pixels[index_bpp + 1] = 0;
        }
      } else {
        if (!(col % 2)) {
          pixels[index_bpp + 0] = 0;
          pixels[index_bpp + 1] = 0;
        }
      }
    }
  }

  string_ncpy(state->default_texture.name, DEFAULT_TEXTURE_NAME,
              Texture::NAME_MAX_LENGTH);
  state->default_texture.width = tex_dimension;
  state->default_texture.height = tex_dimension;
  state->default_texture.channel_count = channels;
  state->default_texture.has_transparency = false;
  state->default_texture.generation = INVALID_ID;
  renderer_create_texture(pixels, &state->default_texture);

  state->default_texture.generation = INVALID_ID;

  return true;
}

void destroy_default_textures(texture_system_state *state) {
  if (state) {
    destroy_texture(&state->default_texture);
  }
}

bool load_texture(cstr texture_name, Texture *t) {
  Resource img_resource;
  if (!resource_system_load(texture_name, ResourceType::IMAGE, &img_resource)) {
    NS_ERROR("Failed to load image resource for texture '%s'.", texture_name);
    return false;
  }

  ImageResourceData *data =
      reinterpret_cast<ImageResourceData *>(img_resource.data);

  // temp texture to load into
  Texture temp_texture;
  temp_texture.width = data->width;
  temp_texture.height = data->height;
  temp_texture.channel_count = data->channel_count;

  u32 current_generation = t->generation;
  t->generation = INVALID_ID;
  usize total_size =
      temp_texture.width * temp_texture.height * temp_texture.channel_count;
  for (usize i = 0; i < total_size; i += temp_texture.channel_count) {
    if (data->pixels[i + 3] < 255) {
      temp_texture.has_transparency = true;
      break;
    }
  }

  string_ncpy(temp_texture.name, texture_name, Texture::NAME_MAX_LENGTH);
  temp_texture.generation = INVALID_ID;

  renderer_create_texture(data->pixels, &temp_texture);

  Texture old = *t;
  *t = temp_texture;

  renderer_destroy_texture(&old);

  if (current_generation == INVALID_ID) {
    t->generation = 0;
  } else {
    t->generation = current_generation + 1;
  }

  resource_system_unload(&img_resource);
  return true;
}

void destroy_texture(Texture *t) {
  renderer_destroy_texture(t);

  mem_zero(t, sizeof(Texture));
  t->id = INVALID_ID;
  t->generation = INVALID_ID;
}

} // namespace ns
