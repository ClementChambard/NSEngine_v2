#ifndef RESOURCE_TYPES_HEADER_INCLUDED
#define RESOURCE_TYPES_HEADER_INCLUDED

#include "../math/types/math_types.h"

namespace ns {

struct Texture {
  static constexpr usize NAME_MAX_LENGTH = 512;
  NSID id;
  u32 width;
  u32 height;
  u8 channel_count;
  bool has_transparency;
  u32 generation;
  char name[NAME_MAX_LENGTH];
  ptr internal_data;
};

enum class TextureUse {
  UNKNOWN = 0x00,
  MAP_DIFFUSE = 0x01,
};

struct TextureMap {
  Texture *texture;
  TextureUse use;
};

struct Material {
  static constexpr usize NAME_MAX_LENGTH = 256;
  NSID id;
  NSID internal_id;
  u32 generation;
  char name[NAME_MAX_LENGTH];
  vec4 diffuse_color;
  TextureMap diffuse_map;
};

struct Geometry {
  static constexpr usize NAME_MAX_LENGTH = 256;
  NSID id;
  NSID internal_id;
  u32 generation;
  char name[NAME_MAX_LENGTH];
  Material *material;
};

} // namespace ns

#endif // RESOURCE_TYPES_HEADER_INCLUDED
