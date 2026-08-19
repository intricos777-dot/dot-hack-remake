#pragma once
#include <cmath>
#include <numbers>

namespace mine {

struct Vec2 {
    float x, y;
    Vec2() : x(0), y(0) {}
    Vec2(float x, float y) : x(x), y(y) {}
    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
};

struct Vec3 {
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    Vec3 operator/(float s) const { return {x / s, y / s, z / s}; }
    Vec3 operator-() const { return {-x, -y, -z}; }
    float dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    Vec3 cross(const Vec3& o) const {
        return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x};
    }
    float length() const { return std::sqrt(x * x + y * y + z * z); }
    float lengthSq() const { return x * x + y * y + z * z; }
    Vec3 normalized() const {
        float len = length();
        if (len < 0.0001f) return {0, 0, 0};
        return {x / len, y / len, z / len};
    }
    Vec3& normalize() {
        float len = length();
        if (len > 0.0001f) { x /= len; y /= len; z /= len; }
        return *this;
    }
    static Vec3 lerp(const Vec3& a, const Vec3& b, float t) {
        return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t};
    }
    static Vec3 cross(const Vec3& a, const Vec3& b) {
        return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
    }
    static float dot(const Vec3& a, const Vec3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }
};

struct Vec4 {
    float x, y, z, w;
    Vec4() : x(0), y(0), z(0), w(1) {}
    Vec4(float x, float y, float z, float w = 1.0f) : x(x), y(y), z(z), w(w) {}
    Vec4(const Vec3& v, float w = 1.0f) : x(v.x), y(v.y), z(v.z), w(w) {}
    Vec3 xyz() const { return {x, y, z}; }
    Vec4 operator+(const Vec4& o) const { return {x + o.x, y + o.y, z + o.z, w + o.w}; }
    Vec4 operator*(float s) const { return {x * s, y * s, z * s, w * s}; }
};

struct Mat4 {
    float m[16];
    Mat4(bool identity = true) {
        for (int i = 0; i < 16; i++) m[i] = (identity && i % 5 == 0) ? 1.0f : 0.0f;
    }
    static Mat4 identity() { return Mat4(true); }
    static Mat4 perspective(float fovDeg, float aspect, float nearPlane, float farPlane) {
        Mat4 result(false);
        float fovRad = fovDeg * (float)std::numbers::pi / 180.0f;
        float f = 1.0f / std::tan(fovRad / 2.0f);
        result.m[0] = f / aspect;
        result.m[5] = f;
        result.m[10] = (farPlane + nearPlane) / (nearPlane - farPlane);
        result.m[11] = -1.0f;
        result.m[14] = (2.0f * farPlane * nearPlane) / (nearPlane - farPlane);
        return result;
    }
    static Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
        Vec3 f = (target - eye).normalized();
        Vec3 r = Vec3::cross(f, up).normalized();
        Vec3 u = Vec3::cross(r, f);
        Mat4 result;
        result.m[0] = r.x; result.m[1] = u.x; result.m[2] = -f.x;
        result.m[4] = r.y; result.m[5] = u.y; result.m[6] = -f.y;
        result.m[8] = r.z; result.m[9] = u.z; result.m[10] = -f.z;
        result.m[12] = -Vec3::dot(r, eye);
        result.m[13] = -Vec3::dot(u, eye);
        result.m[14] = Vec3::dot(f, eye);
        result.m[15] = 1.0f;
        return result;
    }
    static Mat4 translate(const Vec3& t) {
        Mat4 result;
        result.m[12] = t.x; result.m[13] = t.y; result.m[14] = t.z;
        return result;
    }
    static Mat4 scale(const Vec3& s) {
        Mat4 result;
        result.m[0] = s.x; result.m[5] = s.y; result.m[10] = s.z;
        return result;
    }
    static Mat4 rotateY(float angleDeg) {
        Mat4 result;
        float rad = angleDeg * (float)std::numbers::pi / 180.0f;
        float c = std::cos(rad), s = std::sin(rad);
        result.m[0] = c; result.m[2] = s;
        result.m[8] = -s; result.m[10] = c;
        return result;
    }
    static Mat4 rotateX(float angleDeg) {
        Mat4 result;
        float rad = angleDeg * (float)std::numbers::pi / 180.0f;
        float c = std::cos(rad), s = std::sin(rad);
        result.m[5] = c; result.m[6] = -s;
        result.m[9] = s; result.m[10] = c;
        return result;
    }
    Mat4 operator*(const Mat4& o) const {
        Mat4 result(false);
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                result.m[i * 4 + j] = 0;
                for (int k = 0; k < 4; k++) {
                    result.m[i * 4 + j] += m[i * 4 + k] * o.m[k * 4 + j];
                }
            }
        }
        return result;
    }
    Vec4 operator*(const Vec4& v) const {
        return {
            m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12] * v.w,
            m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13] * v.w,
            m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14] * v.w,
            m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15] * v.w
        };
    }
    Vec3 transformPoint(const Vec3& p) const {
        Vec4 v = *this * Vec4(p, 1.0f);
        return {v.x / v.w, v.y / v.w, v.z / v.w};
    }
    Vec3 transformDirection(const Vec3& d) const {
        Vec4 v = *this * Vec4(d, 0.0f);
        return {v.x, v.y, v.z};
    }
};

struct Color {
    float r, g, b, a;
    Color() : r(1), g(1), b(1), a(1) {}
    Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
    Color operator*(float s) const { return {r * s, g * s, b * s, a}; }
    Color operator+(const Color& o) const { return {r + o.r, g + o.g, b + o.b, a + o.a}; }
    Color operator*(const Color& o) const { return {r * o.r, g * o.g, b * o.b, a * o.a}; }
    static Color lerp(const Color& a, const Color& b, float t) {
        return {a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t, a.a + (b.a - a.a) * t};
    }
};

struct Vertex {
    Vec3 position;
    Vec3 normal;
    Vec2 uv;
    Color color;
    Vertex() {}
    Vertex(const Vec3& p, const Vec3& n = {0, 1, 0}, const Vec2& uv = {0, 0}, const Color& c = {1, 1, 1, 1})
        : position(p), normal(n), uv(uv), color(c) {}
};

struct Triangle {
    int v0, v1, v2;
    Triangle(int v0 = 0, int v1 = 0, int v2 = 0) : v0(v0), v1(v1), v2(v2) {}
};

} // namespace mine
