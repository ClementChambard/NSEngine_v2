#include "./ns_math.h"
#include "../platform/platform.h"

#include <math.h>
#include <stdlib.h>

static bool rand_seeded = false;

namespace ns {
f32 sin(f32 x) { return sinf(x); }
f32 cos(f32 x) { return cosf(x); }
f32 tan(f32 x) { return tanf(x); }
f32 acos(f32 x) { return acosf(x); }
f32 sqrt(f32 x) { return sqrtf(x); }
f32 abs(f32 x) { return fabsf(x); }

i32 rand() {
  if (!rand_seeded) {
    srand(static_cast<u32>(platform_get_absolute_time()));
    rand_seeded = true;
  }
  return ::rand();
}

i32 randrange(i32 min, i32 max) {
  if (!rand_seeded) {
    srand(static_cast<u32>(platform_get_absolute_time()));
    rand_seeded = true;
  }
  return (ns::rand() % (max - min + 1)) + min;
}

f32 frand() {
  return static_cast<f32>(ns::rand()) / static_cast<f32>(RAND_MAX);
}

f32 frandrange(f32 min, f32 max) {
  return min + static_cast<f32>(ns::rand()) /
                   (static_cast<float>(RAND_MAX) / (max - min));
}

} // namespace ns
