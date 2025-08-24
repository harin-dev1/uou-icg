#pragma once

#include <array>
#include <cmath>

class Vec3f {
    public:
    float x, y, z;

    Vec3f() : x(0.0f), y(0.0f), z(0.0f) {}  // Default constructor
    Vec3f(float x, float y, float z) : x(x), y(y), z(z) {}
    Vec3f operator-(Vec3f const &other) const {
        return Vec3f(x - other.x, y - other.y, z - other.z);
    }
    Vec3f operator+(Vec3f const &other) const {
        return Vec3f(x + other.x, y + other.y, z + other.z);
    }
    Vec3f operator*(float scalar) const {
        return Vec3f(x * scalar, y * scalar, z * scalar);
    }
    Vec3f cross(Vec3f const &other) const {
        return Vec3f(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
    }
    float dot(Vec3f const &other) const {
        return x * other.x + y * other.y + z * other.z;
    }
    float length() const {
        return sqrt(x * x + y * y + z * z);
    }
    Vec3f normalize() const {
        float length = this->length();
        return Vec3f(x / length, y / length, z / length);
    }
    Vec3f operator/(float scalar) const {
        return Vec3f(x / scalar, y / scalar, z / scalar);
    }
    std::array<float, 3> to_array() const {
        return {x, y, z};
    }
    static Vec3f from_array(const std::array<float, 3>& array) {
        return Vec3f(array[0], array[1], array[2]);
    }
    static Vec3f zero() {
        return Vec3f(0.0f, 0.0f, 0.0f);
    }
    Vec3f(const Vec3f& other) : x(other.x), y(other.y), z(other.z) {}
    Vec3f& operator=(const Vec3f& other) {
        x = other.x;
        y = other.y;
        z = other.z;
        return *this;
    }
    Vec3f& operator+=(const Vec3f& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }
    Vec3f& operator-=(const Vec3f& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }
    Vec3f& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }
    Vec3f& operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }
    Vec3f operator-() const {
        return Vec3f(-x, -y, -z);
    }
    bool operator==(const Vec3f& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
    bool operator!=(const Vec3f& other) const {
        return !(*this == other);
    }
    static Vec3f min(const Vec3f& a, const Vec3f& b) {
        return Vec3f(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
    }
    static Vec3f max(const Vec3f& a, const Vec3f& b) {
        return Vec3f(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
    }
    const float* data() const {
        return &x;
    }
};