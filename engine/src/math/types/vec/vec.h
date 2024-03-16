#ifndef NS_VEC_HEADER_INCLUDED
#define NS_VEC_HEADER_INCLUDED

#include "./vec2.h"
#include "./vec3.h"
#include "./vec4.h"

namespace ns {

inline vec2::vec2(vec3 const &other) : x(other.x), y(other.y) {}
inline vec2::vec2(vec3 &&other) : x(other.x), y(other.y) {}
inline vec2::vec2(vec4 const &other) : x(other.x), y(other.y) {}
inline vec2::vec2(vec4 &&other) : x(other.x), y(other.y) {}
inline vec3::vec3(vec2 const &other) : x(other.x), y(other.y), z(0.0f) {}
inline vec3::vec3(vec2 &&other) : x(other.x), y(other.y), z(0.0f) {}
inline vec3::vec3(vec4 const &other) : x(other.x), y(other.y), z(other.z) {}
inline vec3::vec3(vec4 &&other) : x(other.x), y(other.y), z(other.z) {}
inline vec4::vec4(vec2 const &other)
    : x(other.x), y(other.y), z(0.0f), w(0.0f) {}
inline vec4::vec4(vec2 &&other) : x(other.x), y(other.y), z(0.0f), w(0.0f) {}
inline vec4::vec4(vec3 const &other)
    : x(other.x), y(other.y), z(other.z), w(0.0f) {}
inline vec4::vec4(vec3 &&other) : x(other.x), y(other.y), z(other.z), w(0.0f) {}

NS_INLINE vec2 operator*(f32 f, vec2 const &v) { return {v.x * f, v.y * f}; }
NS_INLINE vec3 operator*(f32 f, vec3 const &v) {
  return {v.x * f, v.y * f, v.z * f};
}
NS_INLINE vec4 operator*(f32 f, vec4 const &v) {
  return {v.x * f, v.y * f, v.z * f, v.w * f};
}

} // namespace ns

#endif // NS_VEC_HEADER_INCLUDED
