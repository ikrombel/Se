#pragma once

#include <SeMath/!Precompiler.hpp>
#include <SeMath/MathDefs.hpp>

namespace Se
{
class Vector2;
class Vector3;
class Vector4;
class IntVector2;
class IntVector3;


/// Two-dimensional vector with integer values.
class IntVector2
{
public:
    /// Construct a zero vector.
    IntVector2() noexcept :
        x_(0),
        y_(0)
    {
    }

    /// Construct from coordinates.
    IntVector2(int x, int y) noexcept :
        x_(x),
        y_(y)
    {
    }

    /// Construct from an int array.
    explicit IntVector2(const int* data) noexcept :
        x_(data[0]),
        y_(data[1])
    {
    }

    /// Construct from an float array.
    explicit IntVector2(const float* data) :
        x_((int)data[0]),
        y_((int)data[1])
    {
    }
    /// Copy-construct from another vector.
    IntVector2(const IntVector2& rhs) noexcept = default;

    /// Assign from another vector.
    IntVector2& operator =(const IntVector2& rhs) noexcept = default;

    /// Test for equality with another vector.
    bool operator ==(const IntVector2& rhs) const { return x_ == rhs.x_ && y_ == rhs.y_; }

    /// Test for inequality with another vector.
    bool operator !=(const IntVector2& rhs) const { return x_ != rhs.x_ || y_ != rhs.y_; }

    /// Add a vector.
    IntVector2 operator +(const IntVector2& rhs) const { return IntVector2(x_ + rhs.x_, y_ + rhs.y_); }

    /// Return negation.
    IntVector2 operator -() const { return IntVector2(-x_, -y_); }

    /// Subtract a vector.
    IntVector2 operator -(const IntVector2& rhs) const { return IntVector2(x_ - rhs.x_, y_ - rhs.y_); }

    /// Multiply with a scalar.
    IntVector2 operator *(int rhs) const { return IntVector2(x_ * rhs, y_ * rhs); }

    /// Multiply with a vector.
    IntVector2 operator *(const IntVector2& rhs) const { return IntVector2(x_ * rhs.x_, y_ * rhs.y_); }

    /// Divide by a scalar.
    IntVector2 operator /(int rhs) const { return IntVector2(x_ / rhs, y_ / rhs); }

    /// Divide by a vector.
    IntVector2 operator /(const IntVector2& rhs) const { return IntVector2(x_ / rhs.x_, y_ / rhs.y_); }

    /// Add-assign a vector.
    IntVector2& operator +=(const IntVector2& rhs)
    {
        x_ += rhs.x_;
        y_ += rhs.y_;
        return *this;
    }

    /// Subtract-assign a vector.
    IntVector2& operator -=(const IntVector2& rhs)
    {
        x_ -= rhs.x_;
        y_ -= rhs.y_;
        return *this;
    }

    /// Multiply-assign a scalar.
    IntVector2& operator *=(int rhs)
    {
        x_ *= rhs;
        y_ *= rhs;
        return *this;
    }

    /// Multiply-assign a vector.
    IntVector2& operator *=(const IntVector2& rhs)
    {
        x_ *= rhs.x_;
        y_ *= rhs.y_;
        return *this;
    }

    /// Divide-assign a scalar.
    IntVector2& operator /=(int rhs)
    {
        x_ /= rhs;
        y_ /= rhs;
        return *this;
    }

    /// Divide-assign a vector.
    IntVector2& operator /=(const IntVector2& rhs)
    {
        x_ /= rhs.x_;
        y_ /= rhs.y_;
        return *this;
    }

    /// Return integer data.
    const int* Data() const { return &x_; }

    /// Return as string.
    String ToString() const {
        return cformat("%d %d", x_, y_);
    }

    /// Return hash value for HashSet & HashMap.
    unsigned ToHash() const { return (unsigned)x_ * 31 + (unsigned)y_; }

    /// Return length.
    float Length() const { return Sqrt<float>(x_ * x_ + y_ * y_); }

    /// Distance to point
    float Distance(const IntVector2& to) const {
        return std::sqrt(std::pow<int>(this->x_ - to.x_, 2) + std::pow<int>(this->y_ - to.y_, 2));
    }

    /// Return Vector2 vector.
    Vector2 ToVector2() const;

    /// Return IntVector3 vector.
    IntVector3 ToIntVector3(int z = 0) const;

    /// Return Vector3 vector.
    Vector3 ToVector3(float z = 0.0f) const;

    /// Return Vector4 vector.
    Vector4 ToVector4(float z = 0.0f, float w = 0.0f) const;

    /// X coordinate.
    int x_;
    /// Y coordinate.
    int y_;

    /// Zero vector.
    static const IntVector2 ZERO;
    /// (-1,0) vector.
    static const IntVector2 LEFT;
    /// (1,0) vector.
    static const IntVector2 RIGHT;
    /// (0,1) vector.
    static const IntVector2 UP;
    /// (0,-1) vector.
    static const IntVector2 DOWN;
    /// (1,1) vector.
    static const IntVector2 ONE;
};

inline const IntVector2 IntVector2::ZERO;
inline const IntVector2 IntVector2::LEFT(-1, 0);
inline const IntVector2 IntVector2::RIGHT(1, 0);
inline const IntVector2 IntVector2::UP(0, 1);
inline const IntVector2 IntVector2::DOWN(0, -1);
inline const IntVector2 IntVector2::ONE(1, 1);

/// Two-dimensional vector.
class Vector2
{
public:
    /// Construct a zero vector.
    Vector2() noexcept :
        x_(0.0f),
        y_(0.0f)
    {
    }

    /// Copy-construct from another vector.
    Vector2(const Vector2& vector) noexcept = default;

    /// Construct from an IntVector2.
    explicit Vector2(const IntVector2& vector) noexcept :
        x_((float)vector.x_),
        y_((float)vector.y_)
    {
    }

    /// Construct from coordinates.
    Vector2(float x, float y) noexcept :
        x_(x),
        y_(y)
    {
    }

    /// Construct from a float array.
    explicit Vector2(const float* data) noexcept :
        x_(data[0]),
        y_(data[1])
    {
    }

    /// Assign from another vector.
    Vector2& operator =(const Vector2& rhs) noexcept = default;

    /// Test for equality with another vector without epsilon.
    bool operator ==(const Vector2& rhs) const { return x_ == rhs.x_ && y_ == rhs.y_; }

    /// Test for inequality with another vector without epsilon.
    bool operator !=(const Vector2& rhs) const { return x_ != rhs.x_ || y_ != rhs.y_; }

    /// Add a vector.
    Vector2 operator +(const Vector2& rhs) const { return Vector2(x_ + rhs.x_, y_ + rhs.y_); }

    /// Return negation.
    Vector2 operator -() const { return Vector2(-x_, -y_); }

    /// Subtract a vector.
    Vector2 operator -(const Vector2& rhs) const { return Vector2(x_ - rhs.x_, y_ - rhs.y_); }

    /// Multiply with a scalar.
    Vector2 operator *(float rhs) const { return Vector2(x_ * rhs, y_ * rhs); }

    /// Multiply with a vector.
    Vector2 operator *(const Vector2& rhs) const { return Vector2(x_ * rhs.x_, y_ * rhs.y_); }

    /// Divide by a scalar.
    Vector2 operator /(float rhs) const { return Vector2(x_ / rhs, y_ / rhs); }

    /// Divide by a vector.
    Vector2 operator /(const Vector2& rhs) const { return Vector2(x_ / rhs.x_, y_ / rhs.y_); }

    /// Add-assign a vector.
    Vector2& operator +=(const Vector2& rhs)
    {
        x_ += rhs.x_;
        y_ += rhs.y_;
        return *this;
    }

    /// Subtract-assign a vector.
    Vector2& operator -=(const Vector2& rhs)
    {
        x_ -= rhs.x_;
        y_ -= rhs.y_;
        return *this;
    }

    /// Multiply-assign a scalar.
    Vector2& operator *=(float rhs)
    {
        x_ *= rhs;
        y_ *= rhs;
        return *this;
    }

    /// Multiply-assign a vector.
    Vector2& operator *=(const Vector2& rhs)
    {
        x_ *= rhs.x_;
        y_ *= rhs.y_;
        return *this;
    }

    /// Divide-assign a scalar.
    Vector2& operator /=(float rhs)
    {
        float invRhs = 1.0f / rhs;
        x_ *= invRhs;
        y_ *= invRhs;
        return *this;
    }

    /// Divide-assign a vector.
    Vector2& operator /=(const Vector2& rhs)
    {
        x_ /= rhs.x_;
        y_ /= rhs.y_;
        return *this;
    }

    /// Normalize to unit length.
    void Normalize()
    {
        float lenSquared = LengthSquared();
        if (!Se::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
        {
            float invLen = 1.0f / Sqrt<float>(lenSquared);
            x_ *= invLen;
            y_ *= invLen;
        }
    }

    /// Return length.
    float Length() const { return Sqrt<float>(x_ * x_ + y_ * y_); }

    /// Return squared length.
    float LengthSquared() const { return x_ * x_ + y_ * y_; }

    /// Calculate dot product.
    float DotProduct(const Vector2& rhs) const { return x_ * rhs.x_ + y_ * rhs.y_; }

    /// Calculate absolute dot product.
    float AbsDotProduct(const Vector2& rhs) const { return Se::Abs(x_ * rhs.x_) + Se::Abs(y_ * rhs.y_); }

    /// Project vector onto axis.
    float ProjectOntoAxis(const Vector2& axis) const { return DotProduct(axis.Normalized()); }

    /// Returns the angle between this vector and another vector in degrees.
    float Angle(const Vector2& rhs) const { return acos(DotProduct(rhs) / (Length() * rhs.Length())); }

    /// Return absolute vector.
    Vector2 Abs() const { return Vector2(Se::Abs(x_), Se::Abs(y_)); }

    /// Linear interpolation with another vector.
    Vector2 Lerp(const Vector2& rhs, float t) const { return *this * (1.0f - t) + rhs * t; }

    /// Test for equality with another vector with epsilon.
    bool Equals(const Vector2& rhs) const { return Se::Equals(x_, rhs.x_) && Se::Equals(y_, rhs.y_); }

    /// Return whether is NaN.
    bool IsNaN() const { return Se::IsNaN(x_) || Se::IsNaN(y_); }

    /// Return normalized to unit length.
    Vector2 Normalized() const
    {
        float lenSquared = LengthSquared();
        if (!Se::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
        {
            float invLen = 1.0f / Sqrt<float>(lenSquared);
            return *this * invLen;
        }
        else
            return *this;
    }

    /// Distance to point
    float Distance(const Vector2& to) const {
        return Sqrt<float>(Pow<float>(this->x_ - to.x_, 2) + Pow<float>(this->y_ - to.y_, 2));
    }

    /// Return float data.
    const float* Data() const { return &x_; }


    /// Return IntVector2 vector.
    IntVector2 ToIntVector2() const { return {static_cast<int>(x_), static_cast<int>(y_)}; }

    /// Return IntVector3 vector.
    IntVector3 ToIntVector3(int z = 0) const;

    /// Return Vector3 vector.
    Vector3 ToVector3(float z = 0.0f) const;

    /// Return Vector4 vector.
    Vector4 ToVector4(float z = 0.0f, float w = 0.0f) const;

    /// Return as string.
    String ToString() const {
        return cformat("%g %g", x_, y_);
    }

    unsigned ToHash() const
    {
        unsigned hash = 37;
        hash = 37 * hash + FloatToRawIntBits(x_);
        hash = 37 * hash + FloatToRawIntBits(y_);

        return hash;
    }

    union {
        struct {
            /// X coordinate.
            float x_;
            /// Y coordinate.
            float y_;
        };
        float data[2];
    };

    /// Zero vector.
    static const Vector2 ZERO;
    /// (-1,0) vector.
    static const Vector2 LEFT;
    /// (1,0) vector.
    static const Vector2 RIGHT;
    /// (0,1) vector.
    static const Vector2 UP;
    /// (0,-1) vector.
    static const Vector2 DOWN;
    /// (1,1) vector.
    static const Vector2 ONE;
};

inline const Vector2 Vector2::ZERO;
inline const Vector2 Vector2::LEFT(-1.0f, 0.0f);
inline const Vector2 Vector2::RIGHT(1.0f, 0.0f);
inline const Vector2 Vector2::UP(0.0f, 1.0f);
inline const Vector2 Vector2::DOWN(0.0f, -1.0f);
inline const Vector2 Vector2::ONE(1.0f, 1.0f);

/// Multiply Vector2 with a scalar
inline Vector2 operator *(float lhs, const Vector2& rhs) { return rhs * lhs; }

/// Multiply IntVector2 with a scalar.
inline IntVector2 operator *(int lhs, const IntVector2& rhs) { return rhs * lhs; }

/// Per-component linear interpolation between two 2-vectors.
inline Vector2 VectorLerp(const Vector2& lhs, const Vector2& rhs, const Vector2& t) { return lhs + (rhs - lhs) * t; }

/// Per-component min of two 2-vectors.
inline Vector2 VectorMin(const Vector2& lhs, const Vector2& rhs) { return Vector2(Min(lhs.x_, rhs.x_), Min(lhs.y_, rhs.y_)); }

/// Per-component max of two 2-vectors.
inline Vector2 VectorMax(const Vector2& lhs, const Vector2& rhs) { return Vector2(Max(lhs.x_, rhs.x_), Max(lhs.y_, rhs.y_)); }

/// Per-component floor of 2-vector.
inline Vector2 VectorFloor(const Vector2& vec) { return Vector2(Floor(vec.x_), Floor(vec.y_)); }

/// Per-component round of 2-vector.
inline Vector2 VectorRound(const Vector2& vec) { return Vector2(Round(vec.x_), Round(vec.y_)); }

/// Per-component ceil of 2-vector.
inline Vector2 VectorCeil(const Vector2& vec) { return Vector2(Ceil(vec.x_), Ceil(vec.y_)); }

/// Per-component absolute value of 2-vector.
inline Vector2 VectorAbs(const Vector2& vec) { return Vector2(Abs(vec.x_), Abs(vec.y_)); }

/// Per-component square root of 2-vector.
inline Vector2 VectorSqrt(const Vector2& vec) { return Vector2(Sqrt(vec.x_), Sqrt(vec.y_)); }

/// Per-component floor of 2-vector. Returns IntVector2.
inline IntVector2 VectorFloorToInt(const Vector2& vec) { return IntVector2(FloorToInt(vec.x_), FloorToInt(vec.y_)); }

/// Per-component round of 2-vector. Returns IntVector2.
inline IntVector2 VectorRoundToInt(const Vector2& vec) { return IntVector2(RoundToInt(vec.x_), RoundToInt(vec.y_)); }

/// Per-component ceil of 2-vector. Returns IntVector2.
inline IntVector2 VectorCeilToInt(const Vector2& vec) { return IntVector2(CeilToInt(vec.x_), CeilToInt(vec.y_)); }

/// Per-component min of two 2-vectors.
inline IntVector2 VectorMin(const IntVector2& lhs, const IntVector2& rhs) { return IntVector2(Min(lhs.x_, rhs.x_), Min(lhs.y_, rhs.y_)); }

/// Per-component max of two 2-vectors.
inline IntVector2 VectorMax(const IntVector2& lhs, const IntVector2& rhs) { return IntVector2(Max(lhs.x_, rhs.x_), Max(lhs.y_, rhs.y_)); }

/// Return a random value from [0, 1) from 2-vector seed.
/// http://stackoverflow.com/questions/12964279/whats-the-origin-of-this-glsl-rand-one-liner
inline float StableRandom(const Vector2& seed) { return Fract(Sin(seed.DotProduct(Vector2(12.9898f, 78.233f)) * M_RADTODEG) * 43758.5453f); }

/// Return a random value from [0, 1) from scalar seed.
inline float StableRandom(float seed) { return StableRandom(Vector2(seed, seed)); }

inline int Compare(const Vector2 &v0, const Vector2 &v1) {
    return (Compare(v0.x_,v1.x_) && Compare(v0.y_,v1.y_));
}

inline int Compare(const Vector2 &v0, const Vector2 &v1, float epsilon) {
    return (Compare(v0.x_, v1.x_, epsilon) && Compare(v0.y_, v1.y_, epsilon));
}

inline int Compare(const IntVector2 &v0, const IntVector2 &v1) {
    return (v0.x_ == v1.x_) && (v0.y_ == v1.y_);
}

/// Return Vector2 vector.
inline Vector2 IntVector2::ToVector2() const { return { static_cast<float>(x_), static_cast<float>(y_) }; }

// https://stackoverflow.com/a/2259502/486974
inline Vector2 RotateAroundPoint(Vector2& point, const Vector2& center, float angle) 
{
    float sin = Sin(angle);
    float cos = Cos(angle);

    point -= center;
    Vector2 rotated = Vector2(point.x_ * cos - point.y_ * sin, point.x_ * sin + point.y_ * cos);
    return rotated + center;
}

}