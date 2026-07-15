#pragma once

#include <SeMath/Vector3.hpp>

namespace Se
{

/// Four-dimensional vector.
class Vector4
{
public:
    /// Construct a zero vector.
    Vector4() noexcept :
        x_(0.0f),
        y_(0.0f),
        z_(0.0f),
        w_(0.0f)
    {
    }

    /// Copy-construct from another vector.
    Vector4(const Vector4& vector) noexcept : 
        x_(vector.x_),
        y_(vector.y_),
        z_(vector.z_),
        w_(vector.w_)
    {
    };

    /// Construct from a 3-dimensional vector and the W coordinate.
    Vector4(const Vector3& vector, float w) noexcept :
        x_(vector.x_),
        y_(vector.y_),
        z_(vector.z_),
        w_(w)
    {
    }

    /// Construct from two 2-dimensional vectors.
    Vector4(const Vector2& v1, const Vector2& v2) noexcept :
        x_(v1.x_),
        y_(v1.y_),
        z_(v2.x_),
        w_(v2.y_)
    {
    }

    Vector4(float value) noexcept :
            x_(value),
            y_(value),
            z_(value),
            w_(value)
    {
    }

    /// Construct from coordinates.
    Vector4(float x, float y, float z, float w) noexcept :
        x_(x),
        y_(y),
        z_(z),
        w_(w)
    {
    }

    /// Construct from a float array.
    explicit Vector4(const float* data) noexcept :
        x_(data[0]),
        y_(data[1]),
        z_(data[2]),
        w_(data[3])
    {
    }

    /// Assign from another vector.
    Vector4& operator =(const Vector4& rhs) noexcept = default;

    /// Test for equality with another vector without epsilon.
    bool operator ==(const Vector4& rhs) const { return x_ == rhs.x_ && y_ == rhs.y_ && z_ == rhs.z_ && w_ == rhs.w_; }

    /// Test for inequality with another vector without epsilon.
    bool operator !=(const Vector4& rhs) const { return x_ != rhs.x_ || y_ != rhs.y_ || z_ != rhs.z_ || w_ != rhs.w_; }

    /// Add a vector.
    Vector4 operator +(const Vector4& rhs) const { return Vector4(x_ + rhs.x_, y_ + rhs.y_, z_ + rhs.z_, w_ + rhs.w_); }

    /// Return negation.
    Vector4 operator -() const { return Vector4(-x_, -y_, -z_, -w_); }

    /// Subtract a vector.
    Vector4 operator -(const Vector4& rhs) const { return Vector4(x_ - rhs.x_, y_ - rhs.y_, z_ - rhs.z_, w_ - rhs.w_); }

    /// Multiply with a scalar.
    Vector4 operator *(float rhs) const { return Vector4(x_ * rhs, y_ * rhs, z_ * rhs, w_ * rhs); }

    /// Multiply with a vector.
    Vector4 operator *(const Vector4& rhs) const { return Vector4(x_ * rhs.x_, y_ * rhs.y_, z_ * rhs.z_, w_ * rhs.w_); }

    /// Divide by a scalar.
    Vector4 operator /(float rhs) const { return Vector4(x_ / rhs, y_ / rhs, z_ / rhs, w_ / rhs); }

    /// Divide by a vector.
    Vector4 operator /(const Vector4& rhs) const { return Vector4(x_ / rhs.x_, y_ / rhs.y_, z_ / rhs.z_, w_ / rhs.w_); }

    /// Add-assign a vector.
    Vector4& operator +=(const Vector4& rhs)
    {
        x_ += rhs.x_;
        y_ += rhs.y_;
        z_ += rhs.z_;
        w_ += rhs.w_;
        return *this;
    }

    /// Subtract-assign a vector.
    Vector4& operator -=(const Vector4& rhs)
    {
        x_ -= rhs.x_;
        y_ -= rhs.y_;
        z_ -= rhs.z_;
        w_ -= rhs.w_;
        return *this;
    }

    /// Multiply-assign a scalar.
    Vector4& operator *=(float rhs)
    {
        x_ *= rhs;
        y_ *= rhs;
        z_ *= rhs;
        w_ *= rhs;
        return *this;
    }

    /// Multiply-assign a vector.
    Vector4& operator *=(const Vector4& rhs)
    {
        x_ *= rhs.x_;
        y_ *= rhs.y_;
        z_ *= rhs.z_;
        w_ *= rhs.w_;
        return *this;
    }

    /// Divide-assign a scalar.
    Vector4& operator /=(float rhs)
    {
        float invRhs = 1.0f / rhs;
        x_ *= invRhs;
        y_ *= invRhs;
        z_ *= invRhs;
        w_ *= invRhs;
        return *this;
    }

    /// Divide-assign a vector.
    Vector4& operator /=(const Vector4& rhs)
    {
        x_ /= rhs.x_;
        y_ /= rhs.y_;
        z_ /= rhs.z_;
        w_ /= rhs.w_;
        return *this;
    }

    Vector3 GetVector3() const { //TODO need Static cast
        return {x_, y_, z_};
    }

    /// Return const value by index.
    float operator[](unsigned index) const { return (&x_)[index]; }

    /// Return mutable value by index.
    float& operator[](unsigned index) { return (&x_)[index]; }

    /// Calculate dot product.
    float DotProduct(const Vector4& rhs) const { return x_ * rhs.x_ + y_ * rhs.y_ + z_ * rhs.z_ + w_ * rhs.w_; }
    float DotProduct(const Vector3& rhs) const { return x_ * rhs.x_ + y_ * rhs.y_ + z_ * rhs.z_ + w_; }

    /// Calculate absolute dot product.
    float AbsDotProduct(const Vector4& rhs) const
    {
        return Se::Abs(x_ * rhs.x_) + Se::Abs(y_ * rhs.y_) + Se::Abs(z_ * rhs.z_) + Se::Abs(w_ * rhs.w_);
    }

    /// Project vector onto axis.
    float ProjectOntoAxis(const Vector3& axis) const { return DotProduct(Vector4(axis.Normalized(), 0.0f)); }

    /// Return absolute vector.
    Vector4 Abs() const { return Vector4(Se::Abs(x_), Se::Abs(y_), Se::Abs(z_), Se::Abs(w_)); }

    /// Linear interpolation with another vector.
    Vector4 Lerp(const Vector4& rhs, float t) const { return *this * (1.0f - t) + rhs * t; }

    /// Test for equality with another vector with epsilon.
    bool Equals(const Vector4& rhs) const
    {
        return Se::Equals(x_, rhs.x_) && Se::Equals(y_, rhs.y_) && Se::Equals(z_, rhs.z_) && Se::Equals(w_, rhs.w_);
    }

    /// Return whether is NaN.
    bool IsNaN() const { return Se::IsNaN(x_) || Se::IsNaN(y_) || Se::IsNaN(z_) || Se::IsNaN(w_); }

    /// Return float data.
    const float* Data() const { return &x_; }

    /// Return as string.
    String ToString() const {
        return cformat("%g %g %g %g", x_, y_, z_, w_);
    }

    /// Return hash value for HashSet & HashMap.
    unsigned ToHash() const
    {
        unsigned hash = 37;
        hash = 37 * hash + FloatToRawIntBits(x_);
        hash = 37 * hash + FloatToRawIntBits(y_);
        hash = 37 * hash + FloatToRawIntBits(z_);
        hash = 37 * hash + FloatToRawIntBits(w_);

        return hash;
    }

    Vector4 &Normalize() {
        float ilength = 1/Sqrt(x_*x_ + y_*y_ + z_*z_ + w_*w_);
        x_ *= ilength; y_ *= ilength; z_ *= ilength; w_ *= ilength;
        return *this;
    }

    static Vector4 &Mad(Vector4 &ret, const Vector4 &v0, float v1, const Vector4 &v2) {
        ret.x_ = v0.x_ * v1 + v2.x_;
        ret.y_ = v0.y_ * v1 + v2.y_;
        ret.z_ = v0.z_ * v1 + v2.z_;
        ret.w_ = v0.w_ * v1 + v2.w_;
        return ret;
    }

    static Vector4 Cross(const Vector3 &v0, const Vector3 &v1) {
        return Vector4(v0.CrossProduct(v1), 0);
    }

    /// Return IntVector2 vector (z, w components are ignored).
    IntVector2 ToIntVector2() const { return {static_cast<int>(x_), static_cast<int>(y_)}; }

    /// Return Vector2 vector (z, w components are ignored).
    Vector2 ToVector2() const { return {x_, y_}; }

    /// Return Vector4 vector.
    IntVector3 ToIntVector3() const { 
        return { static_cast<int>(x_), static_cast<int>(y_), static_cast<int>(z_) };
    }

    /// Return Vector4 vector.
    Vector3 ToVector3() const { return { x_, y_, z_ }; }

    inline static Vector4 Min(const Vector4& lhs, const Vector4& rhs) { 
        return { Se::Min(lhs.x_, rhs.x_), Se::Min(lhs.y_, rhs.y_), Se::Min(lhs.z_, rhs.z_), Se::Min(lhs.w_, rhs.w_)}; 
    }


    inline static Vector4 Max(const Vector4& lhs, const Vector4& rhs) {
        return { Se::Max(lhs.x_, rhs.x_), Se::Max(lhs.y_, rhs.y_), Se::Max(lhs.z_, rhs.z_), Se::Max(lhs.w_, rhs.w_)};
    }


    union {
        struct {
            /// X coordinate.
            float x_;
            /// Y coordinate.
            float y_;
            /// Z coordinate.
            float z_;
            /// W coordinate.
            float w_;
        };
        float data[4];
    };

    /// Zero vector.
    static const Vector4 ZERO;
    /// (1,1,1) vector.
    static const Vector4 ONE;
};


inline const Vector4 Vector4::ZERO{};
inline const Vector4 Vector4::ONE{1.0f, 1.0f, 1.0f, 1.0f};

/// Multiply Vector4 with a scalar.
inline Vector4 operator *(float lhs, const Vector4& rhs) { return rhs * lhs; }

/// Per-component linear interpolation between two 4-vectors.
inline Vector4 VectorLerp(const Vector4& lhs, const Vector4& rhs, const Vector4& t) { return lhs + (rhs - lhs) * t; }

/// Per-component min of two 4-vectors.
inline Vector4 VectorMin(const Vector4& lhs, const Vector4& rhs) { return Vector4(Min(lhs.x_, rhs.x_), Min(lhs.y_, rhs.y_), Min(lhs.z_, rhs.z_), Min(lhs.w_, rhs.w_)); }

/// Per-component max of two 4-vectors.
inline Vector4 VectorMax(const Vector4& lhs, const Vector4& rhs) { return Vector4(Max(lhs.x_, rhs.x_), Max(lhs.y_, rhs.y_), Max(lhs.z_, rhs.z_), Max(lhs.w_, rhs.w_)); }

/// Per-component floor of 4-vector.
inline Vector4 VectorFloor(const Vector4& vec) { return Vector4(Floor(vec.x_), Floor(vec.y_), Floor(vec.z_), Floor(vec.w_)); }

/// Per-component round of 4-vector.
inline Vector4 VectorRound(const Vector4& vec) { return Vector4(Round(vec.x_), Round(vec.y_), Round(vec.z_), Round(vec.w_)); }

/// Per-component ceil of 4-vector.
inline Vector4 VectorCeil(const Vector4& vec) { return Vector4(Ceil(vec.x_), Ceil(vec.y_), Ceil(vec.z_), Ceil(vec.w_)); }

/// Four-dimensional vector.
class IntVector4
{
public:
    /// Construct a zero vector.
    IntVector4() noexcept :
            x_(0.0f),
            y_(0.0f),
            z_(0.0f),
            w_(0.0f)
    {
    }

    /// Copy-construct from another vector.
    IntVector4(const IntVector4& vector) noexcept = default;

    /// Copy-construct from another vector.
    IntVector4(const Vector4& vector) noexcept :
         x_(vector.x_),
         y_(vector.y_),
         z_(vector.z_),
         w_(vector.w_)
    {
    }

    /// Construct from a 3-dimensional vector and the W coordinate.
    IntVector4(const IntVector3& vector, float w) noexcept :
            x_(vector.x_),
            y_(vector.y_),
            z_(vector.z_),
            w_(int(w))
    {
    }

    /// Construct from coordinates.
    IntVector4(int x, int y, int z, int w) noexcept :
            x_(x),
            y_(y),
            z_(z),
            w_(w)
    {
    }

    /// Construct from a float array.
    explicit IntVector4(const int* data) noexcept :
            x_(data[0]),
            y_(data[1]),
            z_(data[2]),
            w_(data[3])
    {
    }

    /// Assign from another vector.
    IntVector4& operator =(const IntVector4& rhs) noexcept = default;

    /// Test for equality with another vector without epsilon.
    bool operator ==(const IntVector4& rhs) const { return x_ == rhs.x_ && y_ == rhs.y_ && z_ == rhs.z_ && w_ == rhs.w_; }

    /// Test for inequality with another vector without epsilon.
    bool operator !=(const IntVector4& rhs) const { return x_ != rhs.x_ || y_ != rhs.y_ || z_ != rhs.z_ || w_ != rhs.w_; }

    /// Add a vector.
    IntVector4 operator +(const IntVector4& rhs) const { return IntVector4(x_ + rhs.x_, y_ + rhs.y_, z_ + rhs.z_, w_ + rhs.w_); }

    /// Return negation.
    IntVector4 operator -() const { return IntVector4(-x_, -y_, -z_, -w_); }

    /// Subtract a vector.
    IntVector4 operator -(const IntVector4& rhs) const { return IntVector4(x_ - rhs.x_, y_ - rhs.y_, z_ - rhs.z_, w_ - rhs.w_); }
    IntVector4 operator -(const Vector4& rhs) const { return IntVector4(x_ - rhs.x_, y_ - rhs.y_, z_ - rhs.z_, w_ - rhs.w_); }

    /// Multiply with a scalar.
    IntVector4 operator *(int rhs) const { return IntVector4(x_ * rhs, y_ * rhs, z_ * rhs, w_ * rhs); }

    /// Multiply with a vector.
    IntVector4 operator *(const IntVector4& rhs) const { return IntVector4(x_ * rhs.x_, y_ * rhs.y_, z_ * rhs.z_, w_ * rhs.w_); }

    /// Divide by a scalar.
    IntVector4 operator /(int rhs) const { return IntVector4(x_ / rhs, y_ / rhs, z_ / rhs, w_ / rhs); }

    /// Divide by a vector.
    IntVector4 operator /(const IntVector4& rhs) const {
        return IntVector4(x_ / rhs.x_, y_ / rhs.y_, z_ / rhs.z_, w_ / rhs.w_);
    }

    IntVector4 operator /(const Vector4& rhs) const {
        return IntVector4(static_cast<int>(x_ / rhs.x_), static_cast<int>(y_ / rhs.y_), static_cast<int>(z_ / rhs.z_), static_cast<int>(w_ / rhs.w_));
    }

    /// Add-assign a vector.
    IntVector4& operator +=(const Vector4& rhs)
    {
        x_ += static_cast<int>(rhs.x_);
        y_ += static_cast<int>(rhs.y_);
        z_ += static_cast<int>(rhs.z_);
        w_ += static_cast<int>(rhs.w_);
        return *this;
    }

    /// Add-assign a vector.
    IntVector4& operator +=(const IntVector4& rhs)
    {
        x_ += rhs.x_;
        y_ += rhs.y_;
        z_ += rhs.z_;
        w_ += rhs.w_;
        return *this;
    }

    /// Subtract-assign a vector.
    IntVector4& operator -=(const IntVector4& rhs)
    {
        x_ -= rhs.x_;
        y_ -= rhs.y_;
        z_ -= rhs.z_;
        w_ -= rhs.w_;
        return *this;
    }

    /// Multiply-assign a scalar.
    IntVector4& operator *=(int rhs)
    {
        x_ *= rhs;
        y_ *= rhs;
        z_ *= rhs;
        w_ *= rhs;
        return *this;
    }

    /// Multiply-assign a vector.
    IntVector4& operator *=(const IntVector4& rhs)
    {
        x_ *= rhs.x_;
        y_ *= rhs.y_;
        z_ *= rhs.z_;
        w_ *= rhs.w_;
        return *this;
    }

    /// Divide-assign a scalar.
    IntVector4& operator /=(int rhs)
    {
        float invRhs = 1.0f / rhs;
        x_ *= invRhs;
        y_ *= invRhs;
        z_ *= invRhs;
        w_ *= invRhs;
        return *this;
    }

    /// Divide-assign a vector.
    IntVector4& operator /=(const IntVector4& rhs)
    {
        x_ /= rhs.x_;
        y_ /= rhs.y_;
        z_ /= rhs.z_;
        w_ /= rhs.w_;
        return *this;
    }

    const IntVector3 GetVector3() { //TODO need Static cast
        return IntVector3(x_, y_, z_);
    }

    /// Return const value by index.
    int operator[](unsigned index) const { return (&x_)[index]; }

    /// Return mutable value by index.
    int& operator[](unsigned index) { return (&x_)[index]; }

    /// Calculate dot product.
    int DotProduct(const IntVector4& rhs) const { return x_ * rhs.x_ + y_ * rhs.y_ + z_ * rhs.z_ + w_ * rhs.w_; }
    /// Calculate dot product.
    float DotProduct(const Vector4& rhs) const { return x_ * rhs.x_ + y_ * rhs.y_ + z_ * rhs.z_ + w_ * rhs.w_; }

    /// Calculate absolute dot product.
    int AbsDotProduct(const IntVector4& rhs) const
    {
        return Se::Abs(x_ * rhs.x_) + Se::Abs(y_ * rhs.y_) + Se::Abs(z_ * rhs.z_) + Se::Abs(w_ * rhs.w_);
    }

    /// Project vector onto axis.
    float ProjectOntoAxis(const Vector3& axis) const { return DotProduct(Vector4(axis.Normalized(), 0.0f)); }

    /// Return absolute vector.
    IntVector4 Abs() const { return IntVector4(Se::Abs(x_), Se::Abs(y_), Se::Abs(z_), Se::Abs(w_)); }

    /// Linear interpolation with another vector.
    IntVector4 Lerp(const IntVector4& rhs, float t) const { return *this * (1.0f - t) + rhs * t; }

    /// Test for equality with another vector with epsilon.
    bool Equals(const IntVector4& rhs) const
    {
        return Se::Equals(x_, rhs.x_) && Se::Equals(y_, rhs.y_) && Se::Equals(z_, rhs.z_) && Se::Equals(w_, rhs.w_);
    }

    /// Return whether is NaN.
    bool IsNaN() const { return Se::IsNaN(x_) || Se::IsNaN(y_) || Se::IsNaN(z_) || Se::IsNaN(w_); }

    /// Return float data.
    const int* Data() const { return &x_; }

    /// Return as string.
    String ToString() const {
        return cformat("%i %i %i %i", x_, y_, z_, w_);
    }

    /// Return hash value for HashSet & HashMap.
    unsigned ToHash() const
    {
        unsigned hash = 37;
        hash = 37 * hash + x_;
        hash = 37 * hash + y_;
        hash = 37 * hash + z_;
        hash = 37 * hash + w_;

        return hash;
    }

    /// Return IntVector2 vector (z, w components are ignored).
    Vector2 ToVector2() const { return {static_cast<float>(x_), static_cast<float>(y_)}; }

    /// Return Vector2 vector (z, w components are ignored).
    IntVector2 ToIntVector2() const { return {x_, y_}; }


    /// Return Vector4 vector.
    Vector3 ToVector3() const { 
        return { static_cast<float>(x_), static_cast<float>(y_), static_cast<float>(z_) };
    }

    /// Return Vector4 vector.
    IntVector3 ToIntVector3() const { return { x_, y_, z_ }; }


    union {
        struct {
            /// X coordinate.
            int x_;
            /// Y coordinate.
            int y_;
            /// Z coordinate.
            int z_;
            /// W coordinate.
            int w_;
        };
        int data[4];
    };


    /// Zero vector.
    inline static const IntVector4 ZERO();
    /// (1,1,1) vector.
    static const IntVector4 ONE;
};

inline const IntVector4 IntVector4::ONE{1, 1, 1, 1};

/// Multiply Vector4 with a scalar.
inline IntVector4 operator *(float lhs, const IntVector4& rhs) { return rhs * lhs; }

/// Per-component linear interpolation between two 4-vectors.
inline IntVector4 VectorLerp(const IntVector4& lhs, const IntVector4& rhs, const IntVector4& t) {
    return lhs + (rhs - lhs) * t;
}

inline Vector4 &Lerp(Vector4 &ret, const Vector4 &v0, const Vector4 &v1, double k) {
    ret.x_ = v0.x_ + (v1.x_ - v0.x_) * k;
    ret.y_ = v0.y_ + (v1.y_ - v0.y_) * k;
    ret.z_ = v0.z_ + (v1.z_ - v0.z_) * k;
    ret.w_ = v0.w_ + (v1.w_ - v0.w_) * k;
    return ret;
}

inline Vector4 Lerp(const Vector4 &v0, const Vector4 &v1, double k) {
    Vector4 ret;
    return Lerp(ret, v0, v1, k);
}

/// Per-component min of two 4-vectors.
inline IntVector4 VectorMin(const IntVector4& lhs, const IntVector4& rhs) {
    return IntVector4(Min(lhs.x_, rhs.x_), Min(lhs.y_, rhs.y_), Min(lhs.z_, rhs.z_), Min(lhs.w_, rhs.w_)); }

/// Per-component max of two 4-vectors.
inline IntVector4 VectorMax(const IntVector4& lhs, const IntVector4& rhs) {
    return IntVector4(Max(lhs.x_, rhs.x_), Max(lhs.y_, rhs.y_), Max(lhs.z_, rhs.z_), Max(lhs.w_, rhs.w_));
}

/// Per-component floor of 4-vector.
inline IntVector4 VectorFloor(const IntVector4& vec) {
    return IntVector4(Floor(vec.x_), Floor(vec.y_), Floor(vec.z_), Floor(vec.w_));
}

/// Per-component round of 4-vector.
inline IntVector4 VectorRound(const IntVector4& vec) {
    return IntVector4(Round(vec.x_), Round(vec.y_), Round(vec.z_), Round(vec.w_));
}

/// Per-component ceil of 4-vector.
inline IntVector4 VectorCeil(const IntVector4& vec) {
    return IntVector4(Ceil(vec.x_), Ceil(vec.y_), Ceil(vec.z_), Ceil(vec.w_));
}

inline int Compare(const Vector4 &v0,const Vector4 &v1) {
    return (v0 == v1);
}

inline int Compare(const Vector4 &v0,const Vector4 &v1, float epsilon) {
    return (v0 == v1);
}

using DVector4 = Vector4;

struct HalfVector4 {

    HalfVector4() { }
    HalfVector4(const HalfVector4 &v) : x(v.x), y(v.y), z(v.z), w(v.w) { }
    HalfVector4(Half x, Half y, Half z, Half w) : x(x), y(y), z(z), w(w) { }
    explicit HalfVector4(Half v) : x(v), y(v), z(v), w(v) { }
    explicit HalfVector4(float v) : x(v), y(v), z(v), w(v) { }
    explicit HalfVector4(const Vector4 &v) :
            x(v.x_), y(v.y_), z(v.z_), w(v.w_) { }
//    explicit HalfVector4(const DVector4 &v) :
//            x((float)v.x_), y((float)v.y_), z((float)v.z_), w((float)v.w_) { }

    inline HalfVector4 &operator=(const HalfVector4 &v) {
        x = v.x; y = v.y; z = v.z; w = v.w;
        return *this;
    }

    inline operator Half*() { return &x; }
    inline operator const Half*() const { return &x; }
    inline operator void*() { return &x; }
    inline operator const void*() const { return &x; }

    inline Half &operator[](int i) {
        assert(i >= 0 && i < 4 && "HalfVector4::operator[](): bad index");
        return (&x)[i];
    }
    inline Half operator[](int i) const {
        assert(i >= 0 && i < 4 && "HalfVector4::operator[](): bad index");
        return (&x)[i];
    }

    Half x,y,z,w;
};

template<class T>
T MinV4(const T &v0,const T &v1) {
    T ret;
    ret.x_ = Min(v0.x_, v1.x_);
    ret.y_ = Min(v0.y_, v1.y_);
    ret.z_ = Min(v0.z_, v1.z_);
    ret.w_ = Min(v0.w_, v1.w_);
    return ret;
}
inline Vector4 Min(const Vector4 &v0, const Vector4 &v1) { return MinV4<Vector4>(v0, v1); }
inline IntVector4 Min(const IntVector4 &v0, const IntVector4 &v1) { return MinV4<IntVector4>(v0, v1); };

template<class T>
T MaxV4(const T &v0, const T &v1) {
    T ret;
    ret.x_ = Max(v0.x_, v1.x_);
    ret.y_ = Max(v0.y_, v1.y_);
    ret.z_ = Max(v0.z_, v1.z_);
    ret.w_ = Max(v0.w_, v1.w_);
    return ret;
}

inline Vector4 Max(const Vector4 &v0, const Vector4 &v1) { return MaxV4<Vector4>(v0, v1); }
inline IntVector4 Max(const IntVector4 &v0, const IntVector4 &v1) { return MaxV4<IntVector4>(v0, v1); };

/// Return Vector4 vector.
inline Vector4 IntVector2::ToVector4(float z, float w) const { return { static_cast<float>(x_), static_cast<float>(y_), z, w }; }

/// Return Vector4 vector.
inline Vector4 Vector2::ToVector4(float z, float w) const { return { x_, y_, z, w }; }

/// Return Vector4 vector.
inline Vector4 IntVector3::ToVector4(float w) const { return { static_cast<float>(x_), static_cast<float>(y_), static_cast<float>(z_), w }; }

/// Return Vector4 vector.
inline Vector4 Vector3::ToVector4(float w) const { return { x_, y_, z_, w }; }

}
