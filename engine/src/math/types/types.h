#ifndef MATH_TYPES_HEADER_INCLUDED
#define MATH_TYPES_HEADER_INCLUDED

#include "./mat/mat.h"
#include "./quat.h"
#include "./vec/vec.h"

namespace ns {

struct vertex_3d {
  vec3 position;
  vec2 texcoord;
};

struct vertex_2d {
  vec2 position;
  vec2 texcoord;
};

} // namespace ns

#endif // MATH_TYPES_HEADER_INCLUDED
