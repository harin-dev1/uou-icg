#include "Matrix4x4.h"

Matrix4x4::Matrix4x4() : m_elements{1, 0, 0, 0,
     0, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1} {}

Matrix4x4::Matrix4x4(float diagonal) : m_elements{diagonal, 0, 0, 0,
     0, diagonal, 0, 0,
      0, 0, diagonal, 0,
      0, 0, 0, 1} {}

Matrix4x4::Matrix4x4(float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24, float m31, float m32, float m33, float m34, float m41, float m42, float m43, float m44) : 
m_elements{m11, m12, m13, m14, m21, m22, m23, m24, m31, m32, m33, m34, m41, m42, m43, m44} {}

Matrix4x4::Matrix4x4(const Matrix4x4& other) : m_elements(other.m_elements) {}

Matrix4x4& Matrix4x4::operator=(const Matrix4x4& other) {
    m_elements = other.m_elements;
    return *this;
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other) const {
    Matrix4x4 result;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.m_elements[i * 4 + j] = 0;
            for (int k = 0; k < 4; k++) {
                result.m_elements[i * 4 + j] += m_elements[i * 4 + k] * other.m_elements[k * 4 + j];
            }
        }
    }
    return result;
}

Vec3f Matrix4x4::operator*(const Vec3f& vector) const {
    Vec3f result;
    result.x = m_elements[0] * vector.x + m_elements[1] * vector.y + m_elements[2] * vector.z + m_elements[3];
    result.y = m_elements[4] * vector.x + m_elements[5] * vector.y + m_elements[6] * vector.z + m_elements[7];
    result.z = m_elements[8] * vector.x + m_elements[9] * vector.y + m_elements[10] * vector.z + m_elements[11];
    return result;
}

float* Matrix4x4::data() {
    return m_elements.data();
}

const float* Matrix4x4::data() const {
    return m_elements.data();
}

float& Matrix4x4::at(int row, int col) {
    return m_elements[row * 4 + col];
}

float Matrix4x4::at(int row, int col) const {
    return m_elements[row * 4 + col];
}

void Matrix4x4::set_translation(float x, float y, float z) {
    at(0, 3) = x;
    at(1, 3) = y;
    at(2, 3) = z;
}

void Matrix4x4::set_rotation_x(float angle) {
    float sin_theta = sin(angle);
    float cos_theta = cos(angle);
    at(1, 1) = cos_theta;
    at(1, 2) = -sin_theta;
    at(2, 1) = sin_theta;
    at(2, 2) = cos_theta;
}

void Matrix4x4::set_rotation_y(float angle) {
    float sin_theta = sin(angle);
    float cos_theta = cos(angle);
    at(0, 0) = cos_theta;
    at(0, 2) = sin_theta;
    at(2, 0) = -sin_theta;
    at(2, 2) = cos_theta;
}

void Matrix4x4::set_rotation_z(float angle) {
    float sin_theta = sin(angle);
    float cos_theta = cos(angle);
    at(0, 0) = cos_theta;
    at(0, 1) = -sin_theta;
    at(1, 0) = sin_theta;
    at(1, 1) = cos_theta;
}

void Matrix4x4::set_scale(float x, float y, float z) {
    at(0, 0) = x;
    at(1, 1) = y;
    at(2, 2) = z;
}

void Matrix4x4::set_perspective(float fov, float aspect_ratio, float near_plane, float far_plane) {
    float angle = fov * M_PI / 180.0f;
    float f = 1.0f / tan(angle / 2.0f);  // fov is in degrees, convert to radians
    float range = far_plane - near_plane;
    at(0, 0) = f / aspect_ratio;
    at(1, 1) = f;
    at(2, 2) = -(near_plane + far_plane) / range;
    at(2, 3) = (-2.0f * near_plane * far_plane) / range;
    at(3, 2) = -1.0f;
}

void Matrix4x4::set_orthographic(float left, float right, float bottom, float top, float near_plane, float far_plane) {
    at(0, 0) = 2.0f / (right - left);
    at(1, 1) = 2.0f / (top - bottom);
    at(2, 2) = 2.0f / (far_plane - near_plane);
    at(3, 0) = -(right + left) / (right - left);
    at(3, 1) = -(top + bottom) / (top - bottom);
    at(3, 2) = -(far_plane + near_plane) / (far_plane - near_plane);
}

void Matrix4x4::set_view(Vec3f const &pos, Vec3f const &target, Vec3f const &up) {
    Vec3f f = -(target - pos).normalize();
    Vec3f s = up.cross(f).normalize();
    Vec3f u = f.cross(s);
    at(0, 0) = s.x;
    at(1, 0) = u.x;
    at(2, 0) = f.x;
    at(0, 1) = s.y;
    at(1, 1) = u.y;
    at(2, 1) = f.y;
    at(0, 2) = s.z;
    at(1, 2) = u.z;
    at(2, 2) = f.z;
    at(0, 3) = -pos.dot(s);
    at(1, 3) = -pos.dot(u);
    at(2, 3) = -pos.dot(f);
    at(3, 0) = 0.0f;
    at(3, 1) = 0.0f;
    at(3, 2) = 0.0f;
    at(3, 3) = 1.0f;
}

void Matrix4x4::set_view(float distance, float yaw, float pitch) {
    float x = distance * sin(yaw) * cos(pitch);
    float y = distance * sin(pitch);
    float z = distance * cos(yaw) * cos(pitch);
    set_view(Vec3f(x, y, z), Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 1.0f, 0.0f));
}