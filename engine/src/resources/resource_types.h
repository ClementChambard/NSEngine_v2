#ifndef RESOURCE_TYPES_HEADER_INCLUDED
#define RESOURCE_TYPES_HEADER_INCLUDED

#include "../math/types/math_types.h"

namespace ns {

struct Texture {
  NSID id;
  u32 width;
  u32 height;
  u8 channel_count;
  bool has_transparency;
  u32 generation;
  ptr internal_data;
};

} // namespace ns

#endif // RESOURCE_TYPES_HEADER_INCLUDED
