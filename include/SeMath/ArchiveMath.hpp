#pragma once
#include <SeMath/!Precompiler.hpp>
#include <SeArc/ArchiveSerializationBasic.hpp>

#include <SeMath/Color.hpp>
#include <SeMath/Matrix3x4.hpp>
#include <SeMath/Rect.hpp>

#include <vector>

namespace Se
{

inline Color ToColor(const String& source)
{
    Color ret;

    auto elements = source.split(' ');
    if (elements.size() < 3)
        return ret;

    ret.r_ = ToFloat(elements[0]);
    ret.g_ = ToFloat(elements[1]);
    ret.b_ = ToFloat(elements[2]);
    if (elements.size() > 3)
        ret.a_ = ToFloat(elements[3]);

    return ret;
}

inline IntVector2 ToIntVector2(const String& source)
{
    IntVector2 ret(IntVector2::ZERO);

    auto elements = source.split(' ');
    
    if (elements.size() < 2)
        return ret;

    ret.x_ = ToInt(elements[0]);
    ret.y_ = ToInt(elements[1]);

    return ret;
}

inline Rect ToRect(const String& source)
{
    Rect ret(Rect::ZERO);

    auto elements = source.split(' ');
    if (elements.size() < 4)
        return ret;

    ret.min_.x_ = ToFloat(elements[0]);
    ret.min_.y_ = ToFloat(elements[1]);
    ret.max_.x_ = ToFloat(elements[2]);
    ret.max_.y_ = ToFloat(elements[3]);

    return ret;
}

inline IntRect ToIntRect(const String& source)
{
    IntRect ret(IntRect::ZERO);

    auto elements = source.split(' ');

    if (elements.size() < 4)
        return ret;

    ret.left_ = ToInt(elements[0]);
    ret.top_ = ToInt(elements[1]);
    ret.right_ = ToInt(elements[2]);
    ret.bottom_ = ToInt(elements[3]);

    return ret;
}

inline Quaternion ToQuaternion(const String& source)
{
    auto elements = source.split(' ');

    if (elements.size() < 3)
        return Quaternion::IDENTITY;
    else if (elements.size() < 4)
    {
        // 3 coords specified: conversion from Euler angles
        float x, y, z;
        x = ToFloat(elements[0]);
        y = ToFloat(elements[1]);
        z = ToFloat(elements[2]);

        return Quaternion(x, y, z);
    }
    else
    {
        // 4 coords specified: full quaternion
        Quaternion ret;
        ret.w_ = ToFloat(elements[0]);
        ret.x_ = ToFloat(elements[1]);
        ret.y_ = ToFloat(elements[2]);
        ret.z_ = ToFloat(elements[3]);

        return ret;
    }
}

inline Vector2 ToVector2(const String& source)
{
    Vector2 ret(Vector2::ZERO);

    auto elements = source.split(' ');
    if (elements.size() < 2)
        return ret;

    ret.x_ = ToFloat(elements[0]);
    ret.y_ = ToFloat(elements[1]);

    return ret;
}

inline Vector3 ToVector3(const String& source)
{
    Vector3 ret(Vector3::ZERO);

    auto elements = source.split(' ');
    if (elements.size() < 3)
        return ret;
    
    ret.x_ = ToFloat(elements[0]);
    ret.y_ = ToFloat(elements[1]);
    ret.z_ = ToFloat(elements[2]);

    return ret;
}

inline IntVector3 ToIntVector3(const String& source)
{
    IntVector3 ret(IntVector3::ZERO);

    auto elements = source.split(' ');
    if (elements.size() < 3)
        return ret;

    ret.x_ = ToInt(elements[0]);
    ret.y_ = ToInt(elements[1]);
    ret.z_ = ToInt(elements[2]);

    return ret;
}

inline Vector4 ToVector4(const String& source, bool allowMissingCoords = false)
{
    Vector4 ret;

    auto elements = source.split(' ');

    if (!allowMissingCoords)
    {
        if (elements.size() < 4)
            return ret;

        ret.x_ = ToFloat(elements[0]);
        ret.y_ = ToFloat(elements[1]);
        ret.z_ = ToFloat(elements[2]);
        ret.w_ = ToFloat(elements[3]);

        return ret;
    }
    else
    {
        if (elements.size() > 0)
            ret.x_ = ToFloat(elements[0]);
        if (elements.size() > 1)
            ret.y_ = ToFloat(elements[1]);
        if (elements.size() > 2)
            ret.z_ = ToFloat(elements[2]);
        if (elements.size() > 3)
            ret.w_ = ToFloat(elements[3]);

        return ret;
    }
}

inline Matrix3 ToMatrix3(const String& source)
{
    Matrix3 ret(Matrix3::ZERO);

    auto elements = source.split(' ');
    if (elements.size() < 9)
        return ret;

    ret.m00_ = ToFloat(elements[0]);
    ret.m01_ = ToFloat(elements[1]);
    ret.m02_ = ToFloat(elements[2]);
    ret.m10_ = ToFloat(elements[3]);
    ret.m11_ = ToFloat(elements[4]);
    ret.m12_ = ToFloat(elements[5]);
    ret.m20_ = ToFloat(elements[6]);
    ret.m21_ = ToFloat(elements[7]);
    ret.m22_ = ToFloat(elements[8]);

    return ret;
}

inline Matrix3x4 ToMatrix3x4(const String& source)
{
    Matrix3x4 ret(Matrix3x4::ZERO);

    auto elements = source.split(' ');
    if (elements.size() < 12)
        return ret;

    ret.m00_ = ToFloat(elements[0]);
    ret.m01_ = ToFloat(elements[1]);
    ret.m02_ = ToFloat(elements[2]);
    ret.m03_ = ToFloat(elements[3]);
    ret.m10_ = ToFloat(elements[4]);
    ret.m11_ = ToFloat(elements[5]);
    ret.m12_ = ToFloat(elements[6]);
    ret.m13_ = ToFloat(elements[7]);
    ret.m20_ = ToFloat(elements[8]);
    ret.m21_ = ToFloat(elements[9]);
    ret.m22_ = ToFloat(elements[10]);
    ret.m23_ = ToFloat(elements[11]);

    return ret;
}

inline Matrix4 ToMatrix4(const String& source)
{
    Matrix4 ret(Matrix4::ZERO);

    auto elements = source.split(' ');
    if (elements.size() < 16)
        return ret;

    ret.m00_ = ToFloat(elements[0]);
    ret.m01_ = ToFloat(elements[1]);
    ret.m02_ = ToFloat(elements[2]);
    ret.m03_ = ToFloat(elements[3]);
    ret.m10_ = ToFloat(elements[4]);
    ret.m11_ = ToFloat(elements[5]);
    ret.m12_ = ToFloat(elements[6]);
    ret.m13_ = ToFloat(elements[7]);
    ret.m20_ = ToFloat(elements[8]);
    ret.m21_ = ToFloat(elements[9]);
    ret.m22_ = ToFloat(elements[10]);
    ret.m23_ = ToFloat(elements[11]);
    ret.m30_ = ToFloat(elements[12]);
    ret.m31_ = ToFloat(elements[13]);
    ret.m32_ = ToFloat(elements[14]);
    ret.m33_ = ToFloat(elements[15]);

    return ret;
}


template <> inline Color FromString<Color>(const char* source) { return ToColor(source); }
template <> inline IntRect FromString<IntRect>(const char* source) { return ToIntRect(source); }
template <> inline IntVector2 FromString<IntVector2>(const char* source) { return ToIntVector2(source); }
template <> inline IntVector3 FromString<IntVector3>(const char* source) { return ToIntVector3(source); }
template <> inline Quaternion FromString<Quaternion>(const char* source) { return ToQuaternion(source); }
template <> inline Rect FromString<Rect>(const char* source) { return ToRect(source); }
template <> inline Vector2 FromString<Vector2>(const char* source) { return ToVector2(source); }
template <> inline Vector3 FromString<Vector3>(const char* source) { return ToVector3(source); }
template <> inline Vector4 FromString<Vector4>(const char* source) { return ToVector4(source); }
//template <> inline Variant FromString<Variant>(const char* source) { return ToVectorVariant(source); }
template <> inline Matrix3 FromString<Matrix3>(const char* source) { return ToMatrix3(source); }
template <> inline Matrix3x4 FromString<Matrix3x4>(const char* source) { return ToMatrix3x4(source); }
template <> inline Matrix4 FromString<Matrix4>(const char* source) { return ToMatrix4(source); }

/// @name Serialize primitive array types
/// @{
inline void SerializeValue(Archive& archive, const char* name, Vector2& value) { Detail::SerializeAsString<Vector2>(archive, name, value); }
inline void SerializeValue(Archive& archive, const char* name, Vector3& value) { Detail::SerializeAsString<Vector3>(archive, name, value); }
inline void SerializeValue(Archive& archive, const char* name, Vector4& value) { Detail::SerializeAsString<Vector4>(archive, name, value); }
inline void SerializeValue(Archive& archive, const char* name, Matrix3& value) { Detail::SerializeAsString<Matrix3>(archive, name, value); }
inline void SerializeValue(Archive& archive, const char* name, Matrix3x4& value) { Detail::SerializeAsString<Matrix3x4>(archive, name, value); }
inline void SerializeValue(Archive& archive, const char* name, Matrix4& value) { Detail::SerializeAsString<Matrix4>(archive, name, value); }
inline void SerializeValue(Archive& archive, const char* name, Rect& value) { Detail::SerializeAsString<Rect>(archive, name, value); }
inline void SerializeValue(Archive& archive, const char* name, Quaternion& value) { Detail::SerializeAsString<Quaternion>(archive, name, value); }
inline void SerializeValue(Archive& archive, const char* name, Color& value) { Detail::SerializeAsString<Color>(archive, name, value); }
inline void SerializeValue(Archive& archive, const char* name, IntVector2& value) { Detail::SerializeAsString<IntVector2>(archive, name, value); }
inline void SerializeValue(Archive& archive, const char* name, IntVector3& value) { Detail::SerializeAsString<IntVector3>(archive, name, value); }
inline void SerializeValue(Archive& archive, const char* name, IntRect& value) { Detail::SerializeAsString<IntRect>(archive, name, value); }
/// @}

} // namespace Se