//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Loong::Math {

using Vector2 = glm::vec2;
using Vector3 = glm::vec3;
using Vector4 = glm::vec4;

using IVector2 = glm::ivec2;
using IVector3 = glm::ivec3;
using IVector4 = glm::ivec4;

using Matrix2 = glm::mat2;
using Matrix3 = glm::mat3;
using Matrix4 = glm::mat4;

using Quat = glm::quat;

const struct {
    template <class T>
    operator T() const // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)
    {
        return glm::zero<T>();
    }
} Zero;

const struct {
    template <class T>
    operator T() const // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)
    {
        return glm::one<T>();
    }
} One;

const struct {
    template <class T>
    operator T() const // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)
    {
        return glm::identity<T>();
    }
} Identity;

const struct {
    template <class T>
    operator T() const // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)
    {
        return glm::pi<T>();
    }
} Pi;

const struct {
    template <class T>
    operator T() const // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)
    {
        return glm::half_pi<T>();
    }
} HalfPi;

const struct {
    template <class T>
    operator T() const // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)
    {
        return glm::two_pi<T>();
    }
} TwoPi;

// clang-format off
const Vector3 kForward { 0.0F,  0.0F, -1.0F }; // NOLINT(cert-err58-cpp)
const Vector3 kRight   { 1.0F,  0.0F,  0.0F }; // NOLINT(cert-err58-cpp)
const Vector3 kUp      { 0.0F,  1.0F,  0.0F }; // NOLINT(cert-err58-cpp)
// clang-format on

template <class T>
inline bool IsBetween(const T& v, const T& min, const T& max)
{
    return v >= min && v <= max;
}

template <>
inline bool IsBetween<Vector2>(const Vector2& v, const Vector2& min, const Vector2& max)
{
    return IsBetween(v.x, min.x, max.x) && IsBetween(v.y, min.y, max.y);
}

template <>
inline bool IsBetween<Vector3>(const Vector3& v, const Vector3& min, const Vector3& max)
{
    return IsBetween(v.x, min.x, max.x) && IsBetween(v.y, min.y, max.y) && IsBetween(v.z, min.z, max.z);
}

template <>
inline bool IsBetween<Vector4>(const Vector4& v, const Vector4& min, const Vector4& max)
{
    return IsBetween(v.x, min.x, max.x) && IsBetween(v.y, min.y, max.y) && IsBetween(v.z, min.z, max.z) && IsBetween(v.w, min.w, max.w);
}

template <class T>
inline T Min(const T& a, const T& b)
{
    return glm::min(a, b);
}

template <class T>
inline T Max(const T& a, const T& b)
{
    return glm::max(a, b);
}

template <class T>
inline T Cross(const T& a, const T& b)
{
    return glm::cross(a, b);
}

template <class T>
inline typename T::value_type Dot(const T& a, const T& b)
{
    return glm::dot(a, b);
}

template <class T>
inline T Transpose(const T& matrix)
{
    return glm::transpose(matrix);
}

template <class T>
inline T Inverse(const T& matrixOrQuat)
{
    return glm::inverse(matrixOrQuat);
}

template <class T>
inline T Conjugate(const T& q)
{
    return glm::conjugate(q);
}

template <class T>
inline T Normalize(const T& t)
{
    return glm::normalize(t);
}

inline Quat LookAtQuat(const Vector3& dir, const Vector3& up)
{
    return glm::quatLookAt(dir, up);
}

inline Matrix4 LookAt(const Vector3& eye, const Vector3& target, const Vector3& up)
{
    return glm::lookAt(eye, target, up);
}

inline Matrix4 Perspective(float fov, float aspect, float nearPlane, float farPlane)
{
    return glm::perspective(fov, aspect, nearPlane, farPlane);
}

inline Matrix4 Perspective(float fov, float width, float height, float nearPlane, float farPlane)
{
    return glm::perspectiveFov(fov, width, height, nearPlane, farPlane);
}

inline Matrix4 Translate(const Matrix4& mat, const Vector3& trans)
{
    return glm::translate(mat, trans);
}

inline Matrix4 Translate(const Vector3& trans)
{
    return glm::translate(Math::Matrix4(Identity), trans);
}

inline Quat Rotate(const Quat& quat, const Vector3& axis, float angle)
{
    return Normalize(glm::rotate(quat, angle, axis));
}

inline Matrix4 Rotate(const Matrix4& mat, const Vector3& axis, float angle)
{
    return glm::rotate(mat, angle, axis);
}

inline Matrix4 Rotate(const Vector3& axis, float angle)
{
    return glm::rotate(Matrix4(Identity), angle, axis);
}

inline Matrix4 Scale(const Matrix4& mat, const Vector3& scale)
{
    return glm::scale(mat, scale);
}

inline Matrix4 Scale(const Vector3& scale)
{
    return glm::scale(Matrix4(Identity), scale);
}

inline Matrix4 QuatToMatrix4(const Quat& quat)
{
    return glm::mat4_cast(quat);
}

template <class T>
inline Quat MatrixToQuat(const T& mat)
{
    return glm::quat_cast(mat);
}

inline Vector3 QuatToEuler(const Quat& q)
{
    Vector3 euler;
    glm::extractEulerAngleYXZ(QuatToMatrix4(q), euler.y, euler.x, euler.z);
    return euler;
}

inline Quat EulerToQuat(const Vector3& euler)
{
    return glm::eulerAngleYXZ(euler.y, euler.x, euler.z);
    return Rotate(Quat(Identity), kUp, euler.z) * Rotate(Quat(Identity), kUp, euler.x) * Rotate(Quat(Identity), kUp, euler.y);
    return glm::quat(euler);
}

template <class T>
inline float Distance(const T& a, const T& b)
{
    return glm::distance(a, b);
}

inline bool Decompose(const Matrix4& mat, Vector3& scale, Quat& rotation, Vector3& translation)
{
    Vector3 skew;
    Vector4 perspective;
    return glm::decompose(mat, scale, rotation, translation, skew, perspective);
}

inline bool DecomposeScale(const Matrix4& mat, Vector3& scale)
{
    Quat rotation;
    Vector3 translation;
    Vector3 skew;
    Vector4 perspective;
    return glm::decompose(mat, scale, rotation, translation, skew, perspective);
}

inline bool DecomposeRotation(const Matrix4& mat, Quat& rotation)
{
    Vector3 scale;
    Vector3 translation;
    Vector3 skew;
    Vector4 perspective;
    return glm::decompose(mat, scale, rotation, translation, skew, perspective);
}

inline bool DecomposeTranslation(const Matrix4& mat, Vector3& translation)
{
    Vector3 scale;
    Quat rotation;
    Vector3 skew;
    Vector4 perspective;
    return glm::decompose(mat, scale, rotation, translation, skew, perspective);
}

inline float Clamp(float x, float min, float max)
{
    return glm::clamp(x, min, max);
}

template <class T>
inline T RadToDegree(T rad) { return glm::degrees(rad); }

template <class T>
inline T DegreeToRad(T degree) { return glm::radians(degree); }

class AABB {
public:
    Vector3 min {};
    Vector3 max {};

    AABB Transformed(const Math::Matrix4& transform) const;
};

} // namespace Loong::Math
