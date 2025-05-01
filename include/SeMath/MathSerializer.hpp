#pragma once

#include <Se/IO/Serializer.hpp>
#include <Se/IO/Deserializer.hpp>

#include <SeMath/BoundingBox.hpp>
#include <SeMath/Color.hpp>
#include <SeMath/Rect.hpp>
#include <SeMath/Quaternion.hpp>

#include <SeMath/Matrix3x4.hpp>

// /// Read an IntRect.
// IntRect ReadIntRect();
// /// Read an IntVector2.
// IntVector2 ReadIntVector2();
// /// Read an IntVector3.
// IntVector3 ReadIntVector3();
// /// Read a Rect.
// Rect ReadRect();
// /// Read a Vector2.
// Vector2 ReadVector2();
// /// Read a Vector3.
// Vector3 ReadVector3();
// /// Read a Vector3 packed into 3 x 16 bits with the specified maximum absolute range.
// Vector3 ReadPackedVector3(float maxAbsCoord);
// /// Read a Vector4.
// Vector4 ReadVector4();
// /// Read a quaternion.
// Quaternion ReadQuaternion();
// /// Read a quaternion with each component packed in 16 bits.
// Quaternion ReadPackedQuaternion();
// /// Read a Matrix3.
// Matrix3 ReadMatrix3();
// /// Read a Matrix3x4.
// Matrix3x4 ReadMatrix3x4();
// /// Read a Matrix4.
// Matrix4 ReadMatrix4();
// /// Read a color.
// Color ReadColor();
// /// Read a bounding box.
// BoundingBox ReadBoundingBox();

namespace Se
{

// ---------------------------------------------------------------------------------
// Deserializer::Read --------------------------------------------------------------

template<> inline Rect Deserializer::Read() { 
    float data[4]; Read(data, sizeof data);
    return Rect(data); }

template<> inline IntRect Deserializer::Read() {
    int data[4]; Read(data, sizeof data);
    return IntRect(data); }

template<> inline Vector2 Deserializer::Read() {
    float data[2]; Read(data, sizeof data);
    return Vector2(data); }

template<> inline IntVector2 Deserializer::Read() {
    int data[2]; Read(data, sizeof data);
    return IntVector2(data); }

template<> inline Vector3 Deserializer::Read() {
    float data[3]; Read(data, sizeof data);
    return Vector3(data); }

template<> inline IntVector3 Deserializer::Read() {
    int data[3]; Read(data, sizeof data);
    return IntVector3(data); }

template<> inline Vector4 Deserializer::Read() {
    float data[4]; Read(data, sizeof data);
    return Vector4(data); }

template<> inline Quaternion Deserializer::Read() {
    float data[4]; Read(data, sizeof data);
    return Quaternion(data); }

template<> inline Matrix3 Deserializer::Read() {
    float data[9]; Read(data, sizeof data);
    return Matrix3(data); }

template<> inline Matrix3x4 Deserializer::Read() {
    float data[12];
    Read(data, sizeof data);
    return Matrix3x4(data); }

template<> inline Matrix4 Deserializer::Read() {
    float data[16]; Read(data, sizeof data);
    return Matrix4(data); }

template<> inline Color Deserializer::Read() {
    float data[4]; Read(data, sizeof data);
    return Color(data); }

template<> inline BoundingBox Deserializer::Read() {
    float data[6]; Read(data, sizeof data);
    return BoundingBox(Vector3(&data[0]), Vector3(&data[3]));
}

// ---------------------------------------------------------------------------------
// Deserializer::ReadPacked --------------------------------------------------------

template<> inline Quaternion Deserializer::ReadPacked(float maxAbsCoord)
{
    float invQ = maxAbsCoord / 32767.0f;
    short coords[4];
    Read(coords, sizeof coords);
    Quaternion ret(coords[0] * invQ, coords[1] * invQ, coords[2] * invQ, coords[3] * invQ);
    ret.Normalize();
    return ret;
}


template<> inline Vector3 Deserializer::ReadPacked(float maxAbsCoord)
{

    float invV = maxAbsCoord / 32767.0f;
    short coords[3];
    Read(coords, sizeof coords);
    Vector3 ret(coords[0] * invV, coords[1] * invV, coords[2] * invV);
    return ret;
}

}