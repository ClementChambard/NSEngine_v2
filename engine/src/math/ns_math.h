#ifndef NS_MATH_HEADER_INCLUDED
#define NS_MATH_HEADER_INCLUDED

#include "../defines.h"
#include "./ns_cstes.h"
#include "./types/math_types.h"

namespace ns {

NS_API f32 sin(f32 x);
NS_API f32 cos(f32 x);
NS_API f32 tan(f32 x);
NS_API f32 acos(f32 x);
NS_API f32 sqrt(f32 x);
NS_API f32 abs(f32 x);

NS_INLINE bool is_power_of_2(u64 value) {
  return (value != 0) && ((value & (value - 1)) == 0);
}

NS_API i32 rand();
NS_API i32 randrange(i32 min, i32 max);
NS_API f32 frand();
NS_API f32 frandrange(f32 min, f32 max);

NS_INLINE f32 deg_2_rad(f32 degrees) { return degrees * DEG_2_RAD<f32>; }
NS_INLINE f32 rad_2_deg(f32 degrees) { return degrees * RAD_2_DEG<f32>; }

} // namespace ns

#endif // NS_MATH_HEADER_INCLUDED
