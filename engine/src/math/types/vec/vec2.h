#ifndef NS_VEC2_HEADER_INCLUDED
#define NS_VEC2_HEADER_INCLUDED

#include "../../../core/string.h"
#include "../../../defines.h"
#include "../../cstes.h"

namespace ns {

struct vec2;
struct vec3;
struct vec4;
f32 sqrt(f32);
f32 abs(f32);

struct vec2 {
  union {
    f32 elements[2];
    struct {
      union {
        f32 x, r, s, u;
      };
      union {
        f32 y, g, t, v;
      };
    };
  };

  vec2() : x(0.0f), y(0.0f) {}
  vec2(vec2 const &other) : x(other.x), y(other.y) {}
  vec2(vec2 &&other) : x(other.x), y(other.y) {}
  explicit vec2(vec3 const &other);
  explicit vec2(vec3 &&other);
  explicit vec2(vec4 const &other);
  explicit vec2(vec4 &&other);
  vec2(f32 x, f32 y) : x(x), y(y) {}
  explicit vec2(f32 v) : x(v), y(v) {}

  vec2 &operator=(vec2 const &other) {
    x = other.x;
    y = other.y;
    return *this;
  }
  vec2 &operator=(vec2 &&other) {
    x = other.x;
    y = other.y;
    return *this;
  }
  vec2 operator+(vec2 const &other) const { return {x + other.x, y + other.y}; }
  vec2 operator+() const { return *this; }
  vec2 operator-(vec2 const &other) const { return {x - other.x, y - other.y}; }
  vec2 operator-() const { return {-x, -y}; }
  vec2 operator*(vec2 const &other) const { return {x * other.x, y * other.y}; }
  vec2 operator*(f32 f) const { return {x * f, y * f}; }
  vec2 operator/(vec2 const &other) const { return {x / other.x, y / other.y}; }
  vec2 operator/(f32 f) const { return {x / f, y / f}; }
  vec2 &operator+=(vec2 const &other) {
    x += other.x;
    y += other.y;
    return *this;
  }
  vec2 &operator-=(vec2 const &other) {
    x -= other.x;
    y -= other.y;
    return *this;
  }
  vec2 &operator*=(vec2 const &other) {
    x *= other.x;
    y *= other.y;
    return *this;
  }
  vec2 &operator/=(vec2 const &other) {
    x /= other.x;
    y /= other.y;
    return *this;
  }
  vec2 &operator*=(f32 f) {
    x *= f;
    y *= f;
    return *this;
  }
  vec2 &operator/=(f32 f) {
    x /= f;
    y /= f;
    return *this;
  }
  f32 operator[](usize i) const { return elements[i]; }
  f32 &operator[](usize i) { return elements[i]; }

  bool operator==(vec2 const &other) const {
    if (abs(x - other.x) > NS_FEPS)
      return false;
    if (abs(y - other.y) > NS_FEPS)
      return false;
    return true;
  }
  bool operator!=(vec2 const &other) const { return !(*this == other); }

  f32 length_sq() const { return x * x + y * y; }
  f32 length() const { return sqrt(length_sq()); }
  void normalize() {
    const f32 len = length();
    x /= len;
    y /= len;
  }
  vec2 normalized() const {
    vec2 o = *this;
    o.normalize();
    return o;
  }
  f32 dist_sq(vec2 const &other) const { return (*this - other).length_sq(); }
  f32 dist(vec2 const &other) const { return (*this - other).length(); }
  f32 dot(vec2 const &other) const { return x * other.x + y * other.y; }

  static vec2 zero() { return {0.0f, 0.0f}; }
  static vec2 one() { return {1.0f, 1.0f}; }
  static vec2 up() { return {0.0f, 1.0f}; }
  static vec2 down() { return {0.0f, -1.0f}; }
  static vec2 left() { return {-1.0f, 0.0f}; }
  static vec2 right() { return {1.0f, 0.0f}; }

  bool from(cstr s) {
    if (!s)
      return false;
    return string_scanf(s, "%f %f", &x, &y) != -1;
  }
};

} // namespace ns

#endif // NS_VEC2_HEADER_INCLUDED
