#pragma once
#include <array>
#include "Vector.h"

class Matrix4x4 {
    public:
        Matrix4x4();
        Matrix4x4(float diagonal);
        Matrix4x4(float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24, float m31, float m32, float m33, float m34, float m41, float m42, float m43, float m44);
        Matrix4x4(const Matrix4x4& other);
        Matrix4x4& operator=(const Matrix4x4& other);
        Matrix4x4 operator*(const Matrix4x4& other) const;
        float* data();
        const float* data() const;
        float& at(int row, int col);
        float at(int row, int col) const;
        void set_translation(float x, float y, float z);
        void set_rotation_x(float angle);
        void set_rotation_y(float angle);
        void set_rotation_z(float angle);
        void set_scale(float x, float y, float z);
        void set_perspective(float fov, float aspect_ratio, float near_plane, float far_plane);
        void set_orthographic(float left, float right, float bottom, float top, float near_plane, float far_plane);
        void set_view(Vec3f const &pos, Vec3f const &target, Vec3f const &up);
        void set_view(float distance, float yaw, float pitch);
    private:
        std::array<float, 16> m_elements;
};