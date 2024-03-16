#ifndef NS_CSTES_HEADER_INCLUDED
#define NS_CSTES_HEADER_INCLUDED

#define NS_INF 1e30
#define NS_FEPS 1.192092896e-07f

namespace ns {

template <typename T> constexpr T PI = 3.14159265358979323846;
template <typename T> constexpr T PI_2 = 6.28318530717958647692;
template <typename T> constexpr T PI_1_2 = 1.57079632679489661923;
template <typename T> constexpr T PI_1_4 = 0.78539816339744830561;
template <typename T> constexpr T PI_INV = 0.318309886183791;
template <typename T> constexpr T PI_2_INV = 0.159154943091895;
template <typename T> constexpr T SQRT_2 = 1.41421356237309504880;
template <typename T> constexpr T SQRT_3 = 1.73205080756887729352;
template <typename T> constexpr T SQRT_1_2 = 0.70710678118654752440;
template <typename T> constexpr T SQRT_1_3 = 0.57735026918962576450;
template <typename T> constexpr T DEG_2_RAD = PI<T> / 180.0;
template <typename T> constexpr T RAD_2_DEG = 180.0 / PI<T>;

} // namespace ns

#endif // NS_CSTES_HEADER_INCLUDED
