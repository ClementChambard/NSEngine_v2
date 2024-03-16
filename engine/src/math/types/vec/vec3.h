#ifndef NS_VEC3_HEADER_INCLUDED
#define NS_VEC3_HEADER_INCLUDED

#include "../../../core/string/cstring.h"
#include "../../../defines.h"
#include "../../cstes.h"
#include "./vec2.h"

namespace ns {

struct vec3 {
  union {
    f32 elements[3];
    struct {
      union {
        f32 x, r, s, u;
      };
      union {
        f32 y, g, t, v;
      };
      union {
        f32 z, b, p, w;
      };
    };
  };

  vec3() : x(0.0f), y(0.0f), z(0.0f) {}
  vec3(vec3 const &other) : x(other.x), y(other.y), z(other.z) {}
  vec3(vec3 &&other) : x(other.x), y(other.y), z(other.z) {}
  explicit vec3(vec2 const &other);
  explicit vec3(vec2 &&other);
  explicit vec3(vec4 const &other);
  explicit vec3(vec4 &&other);
  vec3(f32 x, f32 y, f32 z) : x(x), y(y), z(z) {}
  vec3(vec2 const &v, f32 z) : x(v.x), y(v.y), z(z) {}
  vec3(vec2 &&v, f32 z) : x(v.x), y(v.y), z(z) {}
  explicit vec3(f32 v) : x(v), y(v), z(v) {}

  vec3 &operator=(vec3 const &other) {
    x = other.x;
    y = other.y;
    z = other.z;
    return *this;
  }
  vec3 &operator=(vec3 &&other) {
    x = other.x;
    y = other.y;
    z = other.z;
    return *this;
  }
  vec3 operator+(vec3 const &other) const {
    return {x + other.x, y + other.y, z + other.z};
  }
  vec3 operator+() const { return *this; }
  vec3 operator-(vec3 const &other) const {
    return {x - other.x, y - other.y, z - other.z};
  }
  vec3 operator-() const { return {-x, -y, -z}; }
  vec3 operator*(vec3 const &other) const {
    return {x * other.x, y * other.y, z * other.z};
  }
  vec3 operator*(f32 f) const { return {x * f, y * f, z * f}; }
  vec3 operator/(vec3 const &other) const {
    return {x / other.x, y / other.y, z / other.z};
  }
  vec3 operator/(f32 f) const { return {x / f, y / f, z / f}; }
  vec3 &operator+=(vec3 const &other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
  }
  vec3 &operator-=(vec3 const &other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
  }
  vec3 &operator*=(vec3 const &other) {
    x *= other.x;
    y *= other.y;
    z *= other.z;
    return *this;
  }
  vec3 &operator/=(vec3 const &other) {
    x /= other.x;
    y /= other.y;
    z /= other.z;
    return *this;
  }
  vec3 &operator*=(f32 f) {
    x *= f;
    y *= f;
    z *= f;
    return *this;
  }
  vec3 &operator/=(f32 f) {
    x /= f;
    y /= f;
    z /= f;
    return *this;
  }
  f32 operator[](usize i) const { return elements[i]; }
  f32 &operator[](usize i) { return elements[i]; }

  bool operator==(vec3 const &other) const {
    if (abs(x - other.x) > NS_FEPS)
      return false;
    if (abs(y - other.y) > NS_FEPS)
      return false;
    if (abs(z - other.z) > NS_FEPS)
      return false;
    return true;
  }
  bool operator!=(vec3 const &other) const { return !(*this == other); }

  f32 length_sq() const { return x * x + y * y + z * z; }
  f32 length() const { return sqrt(length_sq()); }
  void normalize() {
    const f32 len = length();
    x /= len;
    y /= len;
    z /= len;
  }
  vec3 normalized() const {
    vec3 o = *this;
    o.normalize();
    return o;
  }
  f32 dist_sq(vec3 const &other) const { return (*this - other).length_sq(); }
  f32 dist(vec3 const &other) const { return (*this - other).length(); }
  f32 dot(vec3 const &other) const {
    return x * other.x + y * other.y + z * other.z;
  }
  vec3 cross(vec3 const &other) const {
    return {
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x,
    };
  }

  static vec3 zero() { return {0.0f, 0.0f, 0.0f}; }
  static vec3 one() { return {1.0f, 1.0f, 1.0f}; }
  static vec3 up() { return {0.0f, 1.0f, 0.0f}; }
  static vec3 down() { return {0.0f, -1.0f, 0.0f}; }
  static vec3 left() { return {-1.0f, 0.0f, 0.0f}; }
  static vec3 right() { return {1.0f, 0.0f, 0.0f}; }
  static vec3 back() { return {0.0f, 0.0f, -1.0f}; }
  static vec3 forward() { return {0.0f, 0.0f, 1.0f}; }

  bool from(cstr s) {
    if (!s)
      return false;
    return string_scanf(s, "%f %f %f", &x, &y, &z) != -1;
  }
};

} // namespace ns

#endif // NS_VEC3_HEADER_INCLUDED
