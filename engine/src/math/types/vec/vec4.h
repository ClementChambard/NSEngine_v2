#ifndef NS_VEC4_HEADER_INCLUDED
#define NS_VEC4_HEADER_INCLUDED

#include "../../../core/string.h"
#include "../../../defines.h"
#include "../../cstes.h"
#include "./vec2.h"
#include "./vec3.h"

namespace ns {

struct vec4 {
  union {
#if defined(NS_USE_SIMD)
    alignas(16) __m128 data;
#endif
    alignas(16) f32 elements[4];
    struct {
      union {
        f32 x, r, s;
      };
      union {
        f32 y, g, t;
      };
      union {
        f32 z, b, p;
      };
      union {
        f32 w, a, q;
      };
    };
  };

  vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
  vec4(vec4 const &other) : x(other.x), y(other.y), z(other.z), w(other.w) {}
  vec4(vec4 &&other) : x(other.x), y(other.y), z(other.z), w(other.w) {}
  explicit vec4(vec2 const &other);
  explicit vec4(vec2 &&other);
  explicit vec4(vec3 const &other);
  explicit vec4(vec3 &&other);
  vec4(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}
  vec4(vec3 const &v, f32 w) : x(v.x), y(v.y), z(v.z), w(w) {}
  vec4(vec3 &&v, f32 w) : x(v.x), y(v.y), z(v.z), w(w) {}
  vec4(vec2 const &v, f32 z, f32 w) : x(v.x), y(v.y), z(z), w(w) {}
  vec4(vec2 &&v, f32 z, f32 w) : x(v.x), y(v.y), z(z), w(w) {}
  explicit vec4(f32 v) : x(v), y(v), z(v), w(v) {}

  vec4 &operator=(vec4 const &other) {
    x = other.x;
    y = other.y;
    z = other.z;
    w = other.w;
    return *this;
  }
  vec4 &operator=(vec4 &&other) {
    x = other.x;
    y = other.y;
    z = other.z;
    w = other.w;
    return *this;
  }
  vec4 operator+(vec4 const &other) const {
    return {x + other.x, y + other.y, z + other.z, w + other.w};
  }
  vec4 operator+() const { return *this; }
  vec4 operator-(vec4 const &other) const {
    return {x - other.x, y - other.y, z - other.z, w - other.w};
  }
  vec4 operator-() const { return {-x, -y, -z, -w}; }
  vec4 operator*(vec4 const &other) const {
    return {x * other.x, y * other.y, z * other.z, w * other.w};
  }
  vec4 operator*(f32 f) const { return {x * f, y * f, z * f, w * f}; }
  vec4 operator/(vec4 const &other) const {
    return {x / other.x, y / other.y, z / other.z, w / other.w};
  }
  vec4 operator/(f32 f) const { return {x / f, y / f, z / f, w / f}; }
  vec4 &operator+=(vec4 const &other) {
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;
    return *this;
  }
  vec4 &operator-=(vec4 const &other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;
    return *this;
  }
  vec4 &operator*=(vec4 const &other) {
    x *= other.x;
    y *= other.y;
    z *= other.z;
    w *= other.w;
    return *this;
  }
  vec4 &operator/=(vec4 const &other) {
    x /= other.x;
    y /= other.y;
    z /= other.z;
    w /= other.w;
    return *this;
  }
  vec4 &operator*=(f32 f) {
    x *= f;
    y *= f;
    z *= f;
    w *= f;
    return *this;
  }
  vec4 &operator/=(f32 f) {
    x /= f;
    y /= f;
    z /= f;
    w /= f;
    return *this;
  }
  f32 operator[](usize i) const { return elements[i]; }
  f32 &operator[](usize i) { return elements[i]; }

  bool operator==(vec4 const &other) const {
    if (abs(x - other.x) > NS_FEPS)
      return false;
    if (abs(y - other.y) > NS_FEPS)
      return false;
    if (abs(z - other.z) > NS_FEPS)
      return false;
    if (abs(w - other.w) > NS_FEPS)
      return false;
    return true;
  }
  bool operator!=(vec4 const &other) const { return !(*this == other); }

  f32 length_sq() const { return x * x + y * y + z * z + w * w; }
  f32 length() const { return sqrt(length_sq()); }
  void normalize() {
    const f32 len = length();
    x /= len;
    y /= len;
    z /= len;
    w /= len;
  }
  vec4 normalized() const {
    vec4 o = *this;
    o.normalize();
    return o;
  }
  f32 dist_sq(vec4 const &other) const { return (*this - other).length_sq(); }
  f32 dist(vec4 const &other) const { return (*this - other).length(); }
  f32 dot(vec4 const &other) const {
    return x * other.x + y * other.y + z * other.z + w * other.w;
  }

  static vec4 one() { return {1.0f, 1.0f, 1.0f, 1.0f}; }
  static vec4 zero() { return {0.0f, 0.0f, 0.0f, 0.0f}; }

  bool from(cstr s) {
    if (!s)
      return false;
    return string_scanf(s, "%f %f %f %f", &x, &y, &z, &w) != -1;
  }
};

} // namespace ns

#endif // NS_VEC4_HEADER_INCLUDED
