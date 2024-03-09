#ifndef NS_MAT4_HEADER_INCLUDED
#define NS_MAT4_HEADER_INCLUDED

#include "../../../core/logger.h"
#include "../../../core/ns_memory.h"
#include "../../../defines.h"
#include "../vec/ns_vec3.h"
#include "../vec/ns_vec4.h"

namespace ns {

f32 sin(f32);
f32 cos(f32);
f32 tan(f32);

struct mat4 {
  union {
    f32 data[16];
    vec4 rows[4];
    f32 d[4][4];
  };
  mat4() { mem_zero(data, 16 * sizeof(f32)); }
  mat4(mat4 const &other) { mem_copy(data, other.data, 16 * sizeof(f32)); }
  mat4(mat4 &&other) { mem_copy(data, other.data, 16 * sizeof(f32)); }
  mat4 &operator=(mat4 const &other) {
    mem_copy(data, other.data, 16 * sizeof(f32));
    return *this;
  }
  mat4 &operator=(mat4 &&other) {
    mem_copy(data, other.data, 16 * sizeof(f32));
    return *this;
  }
  explicit mat4(f32 f) {
    mem_zero(data, 16 * sizeof(f32));
    d[0][0] = d[1][1] = d[2][2] = f;
    d[3][3] = 1.0f;
  }

  mat4 operator*(mat4 const &other) {
    mat4 out_matrix{};
    const f32 *m1_ptr = data;
    const f32 *m2_ptr = other.data;
    f32 *dst_ptr = out_matrix.data;
    for (i32 i = 0; i < 4; i++) {
      for (i32 j = 0; j < 4; j++) {
        *dst_ptr = m1_ptr[0] * m2_ptr[0 + j] + m1_ptr[1] * m2_ptr[4 + j] +
                   m1_ptr[2] * m2_ptr[8 + j] + m1_ptr[3] * m2_ptr[12 + j];
        dst_ptr++;
      }
      m1_ptr += 4;
    }
    return out_matrix;
  }

  vec4 operator*(vec4 const &v) {
    vec4 out_vec{};
    for (i32 i = 0; i < 4; i++) {
      out_vec[i] =
          v.x * d[i][0] + v.y * d[i][1] + v.z * d[i][2] + v.w * d[i][3];
    }
    return out_vec;
  }

  f32 *operator[](usize i) { return d[i]; }
  const f32 *operator[](usize i) const { return d[i]; }

  static mat4 identity() { return mat4(1.0f); }

  static mat4 orthographic(f32 left, f32 right, f32 bottom, f32 top,
                           f32 near_clip, f32 far_clip) {
    mat4 out_matrix(1.0f);

    f32 lr = 1.0f / (left - right);
    f32 bt = 1.0f / (bottom - top);
    f32 nf = 1.0f / (near_clip - far_clip);

    out_matrix[0][0] = -2.0f * lr;
    out_matrix[1][1] = -2.0f * bt;
    out_matrix[2][2] = -2.0f * nf;
    out_matrix[3][0] = (left + right) * lr;
    out_matrix[3][1] = (top + bottom) * bt;
    out_matrix[3][2] = (far_clip + near_clip) * nf;
    return out_matrix;
  }

  static mat4 perspective(f32 fov_rad, f32 aspect_ratio, f32 near_clip,
                          f32 far_clip) {
    f32 half_tan_fov = tan(fov_rad * 0.5f);
    mat4 out_matrix;
    f32 fn = 1.0f / (far_clip - near_clip);
    mem_zero(out_matrix.data, 16 * sizeof(f32));
    out_matrix[0][0] = 1.0f / (aspect_ratio * half_tan_fov);
    out_matrix[1][1] = 1.0f / half_tan_fov;
    out_matrix[2][2] = -(far_clip + near_clip) * fn;
    out_matrix[2][3] = -1.0f;
    out_matrix[3][2] = -2.0 * far_clip * near_clip * fn;
    return out_matrix;
  }

  static mat4 lookat(vec3 const &position, vec3 const &target, vec3 const &up) {
    mat4 out_matrix;
    vec3 z_axis = target - position;
    z_axis.normalize();
    vec3 x_axis = z_axis.cross(up).normalized();
    vec3 y_axis = x_axis.cross(z_axis);

    out_matrix[0][0] = x_axis.x;
    out_matrix[0][1] = y_axis.x;
    out_matrix[0][2] = -z_axis.x;
    out_matrix[0][3] = 0.0f;
    out_matrix[1][0] = x_axis.y;
    out_matrix[1][1] = y_axis.y;
    out_matrix[1][2] = -z_axis.y;
    out_matrix[1][3] = 0.0f;
    out_matrix[2][0] = x_axis.z;
    out_matrix[2][1] = y_axis.z;
    out_matrix[2][2] = -z_axis.z;
    out_matrix[2][3] = 0.0f;
    out_matrix[3][0] = -x_axis.dot(position);
    out_matrix[3][1] = -y_axis.dot(position);
    out_matrix[3][2] = z_axis.dot(position);
    out_matrix[3][3] = 1.0f;
    return out_matrix;
  }

  mat4 inverse() const {
    const f32 *m = data;

    f32 t0 = m[10] * m[15];
    f32 t1 = m[14] * m[11];
    f32 t2 = m[6] * m[15];
    f32 t3 = m[14] * m[7];
    f32 t4 = m[6] * m[11];
    f32 t5 = m[10] * m[7];
    f32 t6 = m[2] * m[15];
    f32 t7 = m[14] * m[3];
    f32 t8 = m[2] * m[11];
    f32 t9 = m[10] * m[3];
    f32 t10 = m[2] * m[7];
    f32 t11 = m[6] * m[3];
    f32 t12 = m[8] * m[13];
    f32 t13 = m[12] * m[9];
    f32 t14 = m[4] * m[13];
    f32 t15 = m[12] * m[5];
    f32 t16 = m[4] * m[9];
    f32 t17 = m[8] * m[5];
    f32 t18 = m[0] * m[13];
    f32 t19 = m[12] * m[1];
    f32 t20 = m[0] * m[9];
    f32 t21 = m[8] * m[1];
    f32 t22 = m[0] * m[5];
    f32 t23 = m[4] * m[1];

    mat4 out_matrix;
    f32 *o = out_matrix.data;

    o[0] = (t0 * m[5] + t3 * m[9] + t4 * m[13]) -
           (t1 * m[5] + t2 * m[9] + t5 * m[13]);
    o[1] = (t1 * m[1] + t6 * m[9] + t9 * m[13]) -
           (t0 * m[1] + t7 * m[9] + t8 * m[13]);
    o[2] = (t2 * m[1] + t7 * m[5] + t10 * m[13]) -
           (t3 * m[1] + t6 * m[5] + t11 * m[13]);
    o[3] = (t5 * m[1] + t8 * m[5] + t11 * m[9]) -
           (t4 * m[1] + t9 * m[5] + t10 * m[9]);
    f32 d = 1.0f / (m[0] * o[0] + m[4] * o[1] + m[8] * o[2] + m[12] * o[3]);
    o[0] = d * o[0];
    o[1] = d * o[1];
    o[2] = d * o[2];
    o[3] = d * o[3];
    o[4] = d * ((t1 * m[4] + t2 * m[8] + t5 * m[12]) -
                (t0 * m[4] + t3 * m[8] + t4 * m[12]));
    o[5] = d * ((t0 * m[0] + t7 * m[8] + t8 * m[12]) -
                (t1 * m[0] + t6 * m[8] + t9 * m[12]));
    o[6] = d * ((t3 * m[0] + t8 * m[4] + t11 * m[12]) -
                (t2 * m[0] + t7 * m[4] + t10 * m[12]));
    o[7] = d * ((t4 * m[0] + t9 * m[4] + t10 * m[8]) -
                (t5 * m[0] + t8 * m[4] + t11 * m[8]));
    o[8] = d * ((t12 * m[7] + t15 * m[11] + t16 * m[15]) -
                (t13 * m[7] + t14 * m[11] + t17 * m[15]));
    o[9] = d * ((t13 * m[3] + t18 * m[11] + t21 * m[15]) -
                (t12 * m[3] + t19 * m[11] + t20 * m[15]));
    o[10] = d * ((t14 * m[3] + t19 * m[7] + t22 * m[15]) -
                 (t15 * m[3] + t18 * m[7] + t23 * m[15]));
    o[11] = d * ((t17 * m[3] + t20 * m[7] + t23 * m[11]) -
                 (t16 * m[3] + t21 * m[7] + t22 * m[11]));
    o[12] = d * ((t14 * m[10] + t17 * m[14] + t13 * m[6]) -
                 (t16 * m[14] + t12 * m[6] + t15 * m[10]));
    o[13] = d * ((t20 * m[14] + t12 * m[2] + t19 * m[10]) -
                 (t18 * m[10] + t21 * m[14] + t13 * m[2]));
    o[14] = d * ((t18 * m[6] + t23 * m[14] + t15 * m[2]) -
                 (t22 * m[14] + t14 * m[2] + t19 * m[6]));
    o[15] = d * ((t22 * m[10] + t16 * m[2] + t21 * m[6]) -
                 (t20 * m[6] + t23 * m[10] + t17 * m[2]));

    return out_matrix;
  }

  static mat4 mk_translate(vec3 const &pos) {
    mat4 out_matrix(1.0f);
    out_matrix[3][0] = pos.x;
    out_matrix[3][1] = pos.y;
    out_matrix[3][2] = pos.z;
    return out_matrix;
  }

  mat4 pre_translate(vec3 const &pos) const {
    mat4 out_matrix = *this;
    out_matrix.rows[3] =
        rows[3] + rows[0] * pos.x + rows[1] * pos.y + rows[2] * pos.z;
    return out_matrix;
  }

  mat4 translate(vec3 const &pos) const {
    mat4 out_matrix = *this;

    out_matrix[0][0] += out_matrix[0][3] * pos.x;
    out_matrix[0][1] += out_matrix[0][3] * pos.y;
    out_matrix[0][2] += out_matrix[0][3] * pos.z;
    out_matrix[1][0] += out_matrix[1][3] * pos.x;
    out_matrix[1][1] += out_matrix[1][3] * pos.y;
    out_matrix[1][2] += out_matrix[1][3] * pos.z;
    out_matrix[2][0] += out_matrix[2][3] * pos.x;
    out_matrix[2][1] += out_matrix[2][3] * pos.y;
    out_matrix[2][2] += out_matrix[2][3] * pos.z;
    out_matrix[3][0] += out_matrix[3][3] * pos.x;
    out_matrix[3][1] += out_matrix[3][3] * pos.y;
    out_matrix[3][2] += out_matrix[3][3] * pos.z;
    return out_matrix;
  }

  static mat4 mk_scale(vec3 const &scale) {
    mat4 out_matrix(1.0f);
    out_matrix[0][0] = scale.x;
    out_matrix[1][1] = scale.y;
    out_matrix[2][2] = scale.z;
    return out_matrix;
  }

  mat4 scale(vec3 const &scale) const {
    mat4 out_matrix;
    for (u8 i = 0; i < 4; i++) {
      out_matrix[i][0] = d[i][0] * scale.x;
      out_matrix[i][1] = d[i][1] * scale.x;
      out_matrix[i][2] = d[i][2] * scale.x;
      out_matrix[i][3] = d[i][3] * scale.x;
    }
    return out_matrix;
  }

  mat4 pre_scale(vec3 const &scale) const {
    mat4 out_matrix;
    out_matrix.rows[0] = rows[0] * scale.x;
    out_matrix.rows[1] = rows[1] * scale.y;
    out_matrix.rows[2] = rows[2] * scale.z;
    out_matrix.rows[3] = rows[3];
    return out_matrix;
  }

  static mat4 mk_rotate_x(f32 angle_rad) {
    mat4 out_matrix(1.0f);
    f32 c = cos(angle_rad);
    f32 s = sin(angle_rad);

    out_matrix[1][1] = c;
    out_matrix[1][2] = s;
    out_matrix[2][1] = -s;
    out_matrix[2][2] = c;
    return out_matrix;
  }

  mat4 rotate_x(f32 angle_rad) const {
    mat4 out_matrix = *this;
    f32 c = cos(angle_rad);
    f32 s = sin(angle_rad);
    for (u8 i = 0; i < 4; i++) {
      out_matrix[i][1] = d[i][1] * c - d[i][2] * s;
      out_matrix[i][2] = d[i][1] * s + d[i][2] * c;
    }
    return out_matrix;
  }

  mat4 pre_rotate_x(f32 angle_rad) const {
    mat4 out_matrix = *this;
    f32 c = cos(angle_rad);
    f32 s = sin(angle_rad);
    for (u8 i = 0; i < 4; i++) {
      out_matrix[1][i] = d[1][i] * c + d[2][i] * s;
      out_matrix[2][i] = d[1][i] * s - d[2][i] * c;
    }
    return out_matrix;
  }

  static mat4 mk_rotate_y(f32 angle_rad) {
    mat4 out_matrix(1.0f);
    f32 c = cos(angle_rad);
    f32 s = sin(angle_rad);

    out_matrix[0][0] = c;
    out_matrix[0][2] = -s;
    out_matrix[2][0] = s;
    out_matrix[2][2] = c;
    return out_matrix;
  }

  static mat4 mk_rotate_z(f32 angle_rad) {
    mat4 out_matrix(1.0f);
    f32 c = cos(angle_rad);
    f32 s = sin(angle_rad);

    out_matrix[0][0] = c;
    out_matrix[0][1] = s;
    out_matrix[1][0] = -s;
    out_matrix[1][1] = c;
    return out_matrix;
  }

  static mat4 mk_euler_xyz(f32 x_rad, f32 y_rad, f32 z_rad) {
    return mat4::mk_rotate_x(x_rad) * mat4::mk_rotate_y(y_rad) *
           mat4::mk_rotate_z(z_rad);
  }

  static mat4 mk_euler_xyz(vec3 angles) {
    return mk_euler_xyz(angles.x, angles.y, angles.z);
  }

  mat4 transposed() const {
    mat4 out_matrix;
    out_matrix[0][0] = d[0][0];
    out_matrix[0][1] = d[1][0];
    out_matrix[0][2] = d[2][0];
    out_matrix[0][3] = d[3][0];
    out_matrix[1][0] = d[0][1];
    out_matrix[1][1] = d[1][1];
    out_matrix[1][2] = d[2][1];
    out_matrix[1][3] = d[3][1];
    out_matrix[2][0] = d[0][2];
    out_matrix[2][1] = d[1][2];
    out_matrix[2][2] = d[2][2];
    out_matrix[2][3] = d[3][2];
    out_matrix[3][0] = d[0][3];
    out_matrix[3][1] = d[1][3];
    out_matrix[3][2] = d[2][3];
    out_matrix[3][3] = d[3][3];
    return out_matrix;
  }

  vec3 forward() const {
    vec3 fwd;
    fwd.x = -d[0][2];
    fwd.y = -d[1][2];
    fwd.z = -d[2][2];
    fwd.normalize();
    return fwd;
  }

  vec3 backward() const {
    vec3 bwd;
    bwd.x = d[0][2];
    bwd.y = d[1][2];
    bwd.z = d[2][2];
    bwd.normalize();
    return bwd;
  }

  vec3 up() const {
    vec3 v;
    v.x = d[0][1];
    v.y = d[1][1];
    v.z = d[2][1];
    v.normalize();
    return v;
  }

  vec3 down() const {
    vec3 v;
    v.x = -d[0][1];
    v.y = -d[1][1];
    v.z = -d[2][1];
    v.normalize();
    return v;
  }

  vec3 left() const {
    vec3 v;
    v.x = -d[0][0];
    v.y = -d[1][0];
    v.z = -d[2][0];
    v.normalize();
    return v;
  }

  vec3 right() const {
    vec3 v;
    v.x = d[0][0];
    v.y = d[1][0];
    v.z = d[2][0];
    v.normalize();
    return v;
  }

  void dump(log_level level = log_level::TRACE) const {
    for (usize i = 0; i < 4; i++) {
      log_output(level, "%f %f %f %f", d[i][0], d[i][1], d[i][2], d[i][3]);
    }
  }
};

} // namespace ns

#endif // NS_MAT4_HEADER_INCLUDED
