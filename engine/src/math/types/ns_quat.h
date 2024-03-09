#ifndef NS_QUAT_HEADER_INCLUDED
#define NS_QUAT_HEADER_INCLUDED

#include "../../defines.h"
#include "../ns_cstes.h"
#include "./mat/ns_mat4.h"

namespace ns {

f32 sin(f32);
f32 cos(f32);
f32 acos(f32);
f32 abs(f32);
f32 sqrt(f32);

struct quat {
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

  quat() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
  quat(quat const &other) : x(other.x), y(other.y), z(other.z), w(other.w) {}
  quat(quat &&other) : x(other.x), y(other.y), z(other.z), w(other.w) {}
  quat(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}
  quat(vec3 axis, f32 angle, bool bnormalize) {
    const f32 half_angle = 0.5f * angle;
    f32 s = sin(half_angle);
    f32 c = sin(half_angle);

    x = s * axis.x;
    y = s * axis.y;
    z = s * axis.z;
    w = c;
    if (bnormalize) {
      normalize();
    }
  }
  quat(quat q_0, quat q_1, f32 percentage) {
    q_0.normalize();
    q_1.normalize();
    f32 dot = q_0.dot(q_1);
    if (dot < 0.0f) {
      q_1.x = -q_1.x;
      q_1.y = -q_1.y;
      q_1.z = -q_1.z;
      q_1.w = -q_1.w;
      dot = -dot;
    }

    const f32 DOT_TRESHOLD = 0.9995f;
    if (dot > DOT_TRESHOLD) {
      x = q_0.x + (q_1.x - q_0.x) * percentage;
      y = q_0.y + (q_1.y - q_0.y) * percentage;
      z = q_0.z + (q_1.z - q_0.z) * percentage;
      w = q_0.w + (q_1.w - q_0.w) * percentage;
      normalize();
      return;
    }

    f32 theta_0 = acos(dot);
    f32 theta = theta_0 * percentage;
    f32 sin_theta = sin(theta);
    f32 sin_theta_0 = sin(theta_0);
    f32 s1 = sin_theta / sin_theta_0;
    f32 s0 = cos(theta) - dot * s1;

    x = q_0.x * s0 + q_1.x * s1;
    y = q_0.y * s0 + q_1.y * s1;
    z = q_0.z * s0 + q_1.z * s1;
    w = q_0.w * s0 + q_1.w * s1;
  }

  quat &operator=(quat const &other) {
    x = other.x;
    y = other.y;
    z = other.z;
    w = other.w;
    return *this;
  }
  quat &operator=(quat &&other) {
    x = other.x;
    y = other.y;
    z = other.z;
    w = other.w;
    return *this;
  }
  quat operator+(quat const &other) const {
    return {x + other.x, y + other.y, z + other.z, w + other.w};
  }
  quat operator+() const { return *this; }
  quat operator-(quat const &other) const {
    return {x - other.x, y - other.y, z - other.z, w - other.w};
  }
  quat operator-() const { return {-x, -y, -z, -w}; }
  quat operator*(f32 f) const { return {x * f, y * f, z * f, w * f}; }
  quat operator/(quat const &other) const {
    return {x / other.x, y / other.y, z / other.z, w / other.w};
  }
  quat operator/(f32 f) const { return {x / f, y / f, z / f, w / f}; }
  f32 operator[](usize i) const { return elements[i]; }
  f32 &operator[](usize i) { return elements[i]; }

  bool operator==(quat const &other) const {
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
  bool operator!=(quat const &other) const { return !(*this == other); }

  static quat identity() { return {0.0f, 0.0f, 0.0f, 1.0f}; }
  f32 normal() const { return sqrt(x * x + y * y + z * z + w * w); }
  void normalize() {
    f32 n = normal();
    x /= n;
    y /= n;
    z /= n;
    w /= n;
  }
  quat normalized() const {
    quat o = *this;
    o.normalize();
    return o;
  }

  quat conjugate() const { return {-x, -y, -z, w}; }

  quat inverse() const { return conjugate().normalized(); }

  quat operator*(quat const &other) const {
    quat out;
    out.x = x * other.w + y * other.z - z * other.y + w * other.x;
    out.y = -x * other.z + y * other.w + z * other.x + w * other.y;
    out.z = x * other.y - y * other.x + z * other.w + w * other.z;
    out.w = -x * other.x - y * other.y - z * other.z + w * other.w;
    return out;
  }

  f32 dot(quat const &other) const {
    return x * other.x + y * other.y + z * other.z + w * other.w;
  }

  mat4 matrix() const {
    mat4 out_matrix(1.0f);
    quat n = normalized();
    out_matrix[0][0] = 1.0f - 2.0f * n.y * n.y - 2.0f * n.z * n.z;
    out_matrix[0][1] = 2.0f * n.x * n.y - 2.0f * n.z * n.w;
    out_matrix[0][2] = 2.0f * n.x * n.z + 2.0f * n.y * n.w;
    out_matrix[1][0] = 2.0f * n.x * n.y + 2.0f * n.z * n.w;
    out_matrix[1][1] = 1.0f - 2.0f * n.x * n.x - 2.0f * n.z * n.z;
    out_matrix[1][2] = 2.0f * n.y * n.z - 2.0f * n.x * n.w;
    out_matrix[2][0] = 2.0f * n.x * n.z - 2.0f * n.y * n.w;
    out_matrix[2][1] = 2.0f * n.y * n.z + 2.0f * n.x * n.w;
    out_matrix[2][2] = 1.0f - 2.0f * n.x * n.x - 2.0f * n.y * n.y;
    return out_matrix;
  }

  mat4 matrix(vec3 const &center) const {
    mat4 out_matrix;
    f32 *o = out_matrix.data;
    o[0] = (x * x) - (y * y) - (z * z) + (w * w);
    o[1] = 2.0f * ((x * y) + (z * w));
    o[2] = 2.0f * ((x * z) - (y * w));
    o[3] = center.x - center.x * o[0] - center.y * o[1] - center.z * o[2];
    o[4] = 2.0f * ((x * y) - (z * w));
    o[5] = -(x * x) + (y * y) - (z * z) + (w * w);
    o[6] = 2.0f * ((y * z) + (x * w));
    o[7] = center.y - center.x * o[4] - center.y * o[5] - center.z * o[6];
    o[8] = 2.0f * ((x * z) + (y * w));
    o[9] = 2.0f * ((y * z) - (x * w));
    o[10] = -(x * x) - (y * y) + (z * z) + (w * w);
    o[11] = center.z - center.x * o[8] - center.y * o[9] - center.z * o[10];
    o[12] = 0.0f;
    o[13] = 0.0f;
    o[14] = 0.0f;
    o[15] = 1.0f;
    return out_matrix;
  }
};

} // namespace ns

#endif // NS_QUAT_HEADER_INCLUDED
