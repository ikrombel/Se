#pragma once

#include <SeMath/Vector2.hpp>

namespace Se
{

class IntVector3;
class Vector3;

/// Interface of three-dimensional vector.
template<class T>
class IVector3
{
    //using Vector3 = IVector3<float>;
public:

    /// Construct a zero vector.
    IVector3() noexcept :
            x_(0), y_(0), z_(0) {}

    /// Construct from a two-dimensional vector and the Z coordinate.
    IVector3(T xyz) noexcept :
            x_(xyz), y_(xyz), z_(xyz) {}

    /// Construct from coordinates.
    IVector3(T x, T y, T z) noexcept :
            x_(x), y_(y), z_(z) {}

    /// Construct from an array.
    explicit IVector3(const T* data) noexcept :
            x_(data[0]), y_(data[1]), z_(data[2]) {}

    /// Construct from two-dimensional coordinates (for 2D).
    IVector3(T x, T y) noexcept :
            x_(x), y_(y), z_(0) {}

    /// Construct from a two-dimensional vector and the Z coordinate.
    IVector3(const Vector2& vector, T z) noexcept :
            x_(vector.x_), y_(vector.y_), z_(z) {}

    /// Copy-construct from another vector.
    IVector3(const IVector3<T>& rhs) noexcept = default;

    /// Return mutable value by index.
    T& operator[](unsigned index) { return (&x_)[index]; }

    /// Assign from another vector.
    template<typename U>
    IVector3<T>& operator =(const IVector3<U>& rhs) noexcept {
        x_ = (T)rhs.x_;
        y_ = (T)rhs.y_;
        z_ = (T)rhs.z_;
    };

    /// Test for equality with another vector.
    bool operator ==(const IVector3<T>& rhs) const { return x_ == rhs.x_ && y_ == rhs.y_ && z_ == rhs.z_; }

    /// Test for inequality with another vector.
    bool operator !=(const IVector3<T>& rhs) const { return x_ != rhs.x_ || y_ != rhs.y_ || z_ != rhs.z_; }

    /// Add a vector.
    IVector3<T> operator +(const IVector3<T>& rhs) const {
        return IVector3<T>(x_ + rhs.x_, y_ + rhs.y_, z_ + rhs.z_);
    }

    /// Return negation.
    IVector3<T> operator -() const { return IVector3<T>(-x_, -y_, -z_); }

    /// Subtract a vector.
    IVector3<T> operator -(const IVector3<T>& rhs) const {
        return IVector3<T>(x_ - rhs.x_, y_ - rhs.y_, z_ - rhs.z_);
    }

    /// Multiply with a scalar.
    IVector3<T> operator *(T rhs) const {
        return IVector3<T>(x_ * rhs, y_ * rhs, z_ * rhs); }

    /// Multiply with a vector.
    IVector3<T> operator *(const IVector3<T>& rhs) const {
        return IVector3<T>(x_ * rhs.x_, y_ * rhs.y_, z_ * rhs.z_);
    }

    /// Divide by a scalar.
    IVector3<T> operator /(T rhs) const {
        return IVector3<T>(x_/rhs, y_/rhs, z_/rhs); }

    /// Divide by a vector.
    IVector3<T> operator /(const IVector3<T>& rhs) const {
        return IVector3<T>(x_/rhs.x_, y_/rhs.y_, z_/rhs.z_);
    }

    /// Add-assign a vector.
    IVector3<T>& operator +=(const IVector3<T>& rhs)
    {
        x_ += rhs.x_;
        y_ += rhs.y_;
        z_ += rhs.z_;
        return *this;
    }

    /// Subtract-assign a vector.
    IVector3<T>& operator -=(const IVector3<T>& rhs)
    {
        x_ -= rhs.x_;
        y_ -= rhs.y_;
        z_ -= rhs.z_;
        return *this;
    }

    /// Multiply-assign a scalar.
    IVector3<T>& operator *=(T rhs)
    {
        x_ *= rhs;
        y_ *= rhs;
        z_ *= rhs;
        return *this;
    }

    /// Multiply-assign a vector.
    IVector3<T>& operator *=(const IVector3<T>& rhs)
    {
        x_ *= rhs.x_;
        y_ *= rhs.y_;
        z_ *= rhs.z_;
        return *this;
    }

    /// Divide-assign a scalar.
    IVector3<T>& operator /=(T rhs)
    {
        x_ /= rhs;
        y_ /= rhs;
        z_ /= rhs;
        return *this;
    }

    /// Divide-assign a vector.
    IVector3<T>& operator /=(const IVector3<T>& rhs)
    {
        x_ /= rhs.x_;
        y_ /= rhs.y_;
        z_ /= rhs.z_;
        return *this;
    }

    /// Return integer data.
    const T* Data() const { return data; }

    /// Return as string.
//    virtual String ToString() const = 0;


    /// Return length.
    float Length() const { return sqrtf((float)(x_ * x_ + y_ * y_ + z_ * z_)); }

    float Length2() const { return (float)(x_ * x_ + y_ * y_ + z_ * z_); }

    /// Distance to point
    float Distance(const IVector3<T>& to) const {
        return Sqrt<float>((float)Pow<T>(this->x_ - to.x_, 2)
                       + (float)Pow<T>(this->y_ - to.y_, 2)
                       + (float)Pow<T>(this->z_ - to.z_, 2));
    }

    /// Distance squared to point
    float DistanceSquared(const IVector3<T>& to) const {
        return Pow<T>(this->x_ - to.x_, 2)
             + Pow<T>(this->y_ - to.y_, 2)
             + Pow<T>(this->z_ - to.z_, 2);
    }

    /// Returns the angle between this vector and another vector in degrees.
    float Angle(const IVector3<T>& rhs) const { return Se::Acos<float>(DotProduct(rhs) / (Length() * rhs.Length())); }

    /// Return whether is NaN.
    bool IsNaN() const { return Se::IsNaN(x_) || Se::IsNaN(y_) || Se::IsNaN(z_); }

    /// Return normalized to unit length.
    IVector3<T> Normalized() const
    {
        float lenSquared = LengthSquared();
        if (!Se::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
        {
            float invLen = 1.0f / sqrtf(lenSquared);
            return *this * invLen;
        }
        else
            return *this;
    }


    /// Return normalized to unit length or zero if length is too small.
    IVector3<float> NormalizedOrDefault(const IVector3<float>& defaultValue = 0, float eps = M_LARGE_EPSILON) const
    {
        const float lenSquared = LengthSquared();
        if (lenSquared < eps * eps)
            return defaultValue;
        return *this / sqrtf(lenSquared);
    }

    /// Return normalized vector with length in given range.
    IVector3<float> ReNormalized(float minLength, float maxLength, const IVector3<float>& defaultValue = 0, float eps = M_LARGE_EPSILON) const
    {
        const float lenSquared = LengthSquared();
        if (lenSquared < eps * eps)
            return defaultValue;

        const float len = sqrtf(lenSquared);
        const float newLen = Clamp(len, minLength, maxLength);
        return *this * (newLen / len);
    }

    /// Return absolute vector.
    IVector3<T> Abs() const { return IVector3<T>(Se::Abs(x_), Se::Abs(y_), Se::Abs(z_)); }

    /// Linear interpolation with another vector.
    IVector3<T> Lerp(const IVector3<T>& rhs, float t) const { return *this * (1.0f - t) + rhs * t; }

    /// Test for equality with another vector with epsilon.
    bool Equals(const IVector3<float>& rhs, float eps = M_EPSILON) const
    {
        return Se::Equals(x_, rhs.x_, eps) && Se::Equals(y_, rhs.y_, eps) && Se::Equals(z_, rhs.z_, eps);
    }

    /// Return squared length.
    float LengthSquared() const { return x_ * x_ + y_ * y_ + z_ * z_; }

    /// Normalize to unit length.
    void Normalize()
    {
        float lenSquared = LengthSquared();
        if (!Se::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
        {
            float invLen = 1.0f / sqrtf(lenSquared);
            x_ *= invLen;
            y_ *= invLen;
            z_ *= invLen;
        }
    }

    /// Calculate dot product.
    float DotProduct(const IVector3<T>& rhs) const { return x_ * rhs.x_ + y_ * rhs.y_ + z_ * rhs.z_; }

    /// Calculate absolute dot product.
    float AbsDotProduct(const IVector3<T>& rhs) const
    {
        return Se::Abs(x_ * rhs.x_) + Se::Abs(y_ * rhs.y_) + Se::Abs(z_ * rhs.z_);
    }

//        float DotProduct(const IVector3<T> &v1) {
//            return x_ * v1.x + y_ * v1.y + z_ * v1.z;
//        }


    /// Project direction vector onto axis.
    float ProjectOntoAxis(const IVector3<T>& axis) const { return DotProduct(axis.Normalized()); }

    /// Project position vector onto plane with given origin and normal.
    IVector3<T> ProjectOntoPlane(const IVector3<T>& origin, const IVector3<T>& normal) const
    {
        const IVector3<T> delta = *this - origin;
        return *this - normal.Normalized() * delta.ProjectOntoAxis(normal);
    }

    /// Project position vector onto line segment.
    IVector3<T> ProjectOntoLine(const IVector3<T>& from, const IVector3<T>& to, bool clamped = false) const
    {
        const IVector3<T> direction = to - from;
        const float lengthSquared = direction.LengthSquared();
        float factor = IVector3<T>(*this - from).DotProduct(direction) / lengthSquared;

        if (clamped)
            factor = Clamp(factor, 0.0f, 1.0f);

        return from + direction * factor;
    }

    /// Calculate distance to another position vector.
    float DistanceToPoint(const IVector3<T>& point) const { return (*this - point).Length(); }

    /// Calculate distance to the plane with given origin and normal.
    float DistanceToPlane(const IVector3<T>& origin, const IVector3<T>& normal) const {
        return IVector3<T>(*this - origin).ProjectOntoAxis(normal);
    }

    /// Make vector orthogonal to the axis.
    IVector3<T> Orthogonalize(const IVector3<T>& axis) const {
        return axis.CrossProduct(*this).CrossProduct(axis).Normalized();
    }

    /// Calculate cross product.
    IVector3<T> CrossProduct(const IVector3<T>& rhs) const
    {
        return IVector3<T>(
                y_ * rhs.z_ - z_ * rhs.y_,
                z_ * rhs.x_ - x_ * rhs.z_,
                x_ * rhs.y_ - y_ * rhs.x_
        );
    }

    static void Mad(IVector3<T> &ret, const IVector3<T> &v0, T v1, const IVector3<T> &v2) {
        ret.x_ = v0.x_ * v1 + v2.x_;
        ret.y_ = v0.y_ * v1 + v2.y_;
        ret.z_ = v0.z_ * v1 + v2.z_;
    }

    static IVector3<T> &Cross(IVector3<T> &ret, const IVector3<T> &v0, const IVector3<T> &v1) {
        ret.x_ = v0.y_ * v1.z_ - v0.z_ * v1.y_;
        ret.y_ = v0.z_ * v1.x_ - v0.x_ * v1.z_;
        ret.z_ = v0.x_ * v1.y_ - v0.y_ * v1.x_;
        return ret;
    }

    static void PointTriangleCoordinates(const IVector3<T> &point, const IVector3<T> &v0, const IVector3<T> &v1,
                                  const IVector3<T> &v2, float &a, float &b) {
        IVector3<T> v20, v10, v0p, area;
        v20 = v2 - v0;
        v10 = v1 - v0;
        float iarea = 1.0f/Cross(area, v20, v10).Length();
        v20 = v2 - point;
        v10 = v1 - point;
        v0p = v0 - point;
        a = Cross(area, v20, v0p).Length() * iarea;
        b = Cross(area, v10, v0p).Length() * iarea;
    }

    union {
        struct {
            T x_;
            T y_;
            T z_;
        };
        T data[3];
    };
};

// Three-dimensional vector with integer values.
class IntVector3 : public IVector3<int>
{
public:
    /// Construct a zero vector.
    IntVector3() noexcept : IVector3<int>(0, 0, 0) {}
    /// Construct from coordinates.
    IntVector3(int x, int y, int z) noexcept : IVector3<int>(x, y, z) {}
    /// Construct from an int array.
    explicit IntVector3(const int* data) noexcept :
            IVector3<int>(data[0], data[1], data[2]) {}

    /// Copy-construct from another vector.
    template<class T>
    IntVector3(const IVector3<T>& rhs) noexcept :
            IVector3<int>((T)rhs.x_, (T)rhs.y_, (T)rhs.z_) {}

    /// Copy-construct from another vector.
    IntVector3(const IntVector3& rhs) noexcept :
            IVector3<int>(rhs.x_, rhs.y_, rhs.z_) {}

    /// Return as string.
    String ToString() const {
        return cformat("%d %d %d", x_, y_, z_);
    }

    /// Return IntVector2 vector (z component is ignored).
    IntVector2 ToIntVector2() const { return {x_, y_}; }

    /// Return Vector2 vector (z component is ignored).
    Vector2 ToVector2() const { return {static_cast<float>(x_), static_cast<float>(y_)}; }

    /// Return Vector3 vector.
    Vector3 ToVector3() const;

    /// Return Vector4 vector.
    Vector4 ToVector4(float w = 0.0f) const;

    /// Return hash value for HashSet & HashMap.
    unsigned ToHash() const { return (unsigned)x_ * 31 * 31 + (unsigned)y_ * 31 + (unsigned)z_; }

    /// Per-component min of two 3-vectors.
    static Vector3 Min(const Vector3& lhs, const Vector3& rhs);

    /// Per-component max of two 3-vectors.
    static Vector3 Max(const Vector3& lhs, const Vector3& rhs);


    /// Zero vector.
    static const IntVector3 ZERO;
    /// (-1,0,0) vector.
    static const IntVector3 LEFT;
    /// (1,0,0) vector.
    static const IntVector3 RIGHT;
    /// (0,1,0) vector.
    static const IntVector3 UP;
    /// (0,-1,0) vector.
    static const IntVector3 DOWN;
    /// (0,0,1) vector.
    static const IntVector3 FORWARD;
    /// (0,0,-1) vector.
    static const IntVector3 BACK;
    /// (1,1,1) vector.
    static const IntVector3 ONE;
};

inline const IntVector3 IntVector3::ZERO;
inline const IntVector3 IntVector3::LEFT(-1, 0, 0);
inline const IntVector3 IntVector3::RIGHT(1, 0, 0);
inline const IntVector3 IntVector3::UP(0, 1, 0);
inline const IntVector3 IntVector3::DOWN(0, -1, 0);
inline const IntVector3 IntVector3::FORWARD(0, 0, 1);
inline const IntVector3 IntVector3::BACK(0, 0, -1);
inline const IntVector3 IntVector3::ONE(1, 1, 1);

// Three-dimensional vector with integer values.
class DVector3 : public IVector3<double>
{
public:
    /// Construct a zero vector.
    DVector3() noexcept : IVector3<double>(0, 0, 0) {}
    /// Construct from coordinates.
    DVector3(double x, double y, double z) noexcept : IVector3<double>(x, y, z) {}
    /// Construct from an int array.
    explicit DVector3(const double* data) noexcept :
            IVector3<double>(data[0], data[1], data[2]) {}

    /// Copy-construct from another vector.
    template<class T>
    DVector3(const IVector3<T>& rhs) noexcept :
            IVector3<double>((T)rhs.x_, (T)rhs.y_, (T)rhs.z_) {}

    /// Copy-construct from another vector.
    DVector3(const IntVector3& rhs) noexcept :
            IVector3<double>(rhs.x_, rhs.y_, rhs.z_) {}

    /// Construct from a two-dimensional vector and the Z coordinate.
    DVector3(double xyz) noexcept :
            IVector3<double>(xyz) {}

    /// Return as string.
    String ToString() const {
        return cformat("%g %g %g", x_, y_, z_);
    }

    /// Return hash value for HashSet & HashMap.
    unsigned ToHash() const
    {
        unsigned hash = 37;
        hash = 37 * hash + FloatToRawIntBits(x_);
        hash = 37 * hash + FloatToRawIntBits(y_);
        hash = 37 * hash + FloatToRawIntBits(z_);

        return hash;
    }

    /// Zero vector.
    static const DVector3 ZERO;
    /// (-1,0,0) vector.
    static const DVector3 LEFT;
    /// (1,0,0) vector.
    static const DVector3 RIGHT;
    /// (0,1,0) vector.
    static const DVector3 UP;
    /// (0,-1,0) vector.
    static const DVector3 DOWN;
    /// (0,0,1) vector.
    static const DVector3 FORWARD;
    /// (0,0,-1) vector.
    static const DVector3 BACK;
    /// (1,1,1) vector.
    static const DVector3 ONE;
};

inline const DVector3 DVector3::ZERO;
inline const DVector3 DVector3::LEFT(-1.0f, 0.0f, 0.0f);
inline const DVector3 DVector3::RIGHT(1.0f, 0.0f, 0.0f);
inline const DVector3 DVector3::UP(0.0f, 1.0f, 0.0f);
inline const DVector3 DVector3::DOWN(0.0f, -1.0f, 0.0f);
inline const DVector3 DVector3::FORWARD(0.0f, 0.0f, 1.0f);
inline const DVector3 DVector3::BACK(0.0f, 0.0f, -1.0f);
inline const DVector3 DVector3::ONE(1.0f, 1.0f, 1.0f);

// Three-dimensional vector with integer values.
class Vector3 : public IVector3<float>
{
public:
    /// Construct a zero vector.
    Vector3() noexcept : IVector3<float>(0, 0, 0) {}
    /// Construct from 3D coordinates.
    Vector3(float xyz) noexcept : IVector3<float>(xyz, xyz, xyz) {}
    /// Construct from 3D coordinates.
    Vector3(float x, float y, float z) noexcept : IVector3<float>(x, y, z) {}

    /// Construct from an float array.
    explicit Vector3(const float* data) noexcept :
            IVector3<float>(data[0], data[1], data[2]) {}

    /// Construct from 2D coordinates.
    Vector3(float x, float y) noexcept : IVector3<float>(x, y, 0) {}
    Vector3(const Vector2& vector, float z = 0.0f) noexcept :
            IVector3(vector, z) {}

    /// Copy-construct from another vector.
    template<class T>
    Vector3(const Vector3& rhs) noexcept :
            IVector3<float>(rhs.x_, rhs.y_, rhs.z_) {}

    /// Copy-construct from another vector.
    template<class T>
    Vector3(const IVector3<T>& rhs) noexcept :
            IVector3<float>(rhs.x_, rhs.y_, rhs.z_) {}

    Vector3(const IntVector3& rhs) noexcept :
            IVector3<float>(rhs.x_, rhs.y_, rhs.z_) {}

    /// Add a vector.
    Vector3 operator +(const IVector3<float>& rhs) const {
        return Vector3(x_ + rhs.x_, y_ + rhs.y_, z_ + rhs.z_);
    }
    
    /// Return as string.
    String ToString() const {
        return cformat("%g %g %g", x_, y_, z_);
    }

    /// Return hash value for HashSet & HashMap.
    unsigned ToHash() const
    {
        unsigned hash = 37;
        hash = 37 * hash + FloatToRawIntBits(x_);
        hash = 37 * hash + FloatToRawIntBits(y_);
        hash = 37 * hash + FloatToRawIntBits(z_);

        return hash;
    }

    /// Return IntVector2 vector (z component is ignored).
    IntVector2 ToIntVector2() const { return {static_cast<int>(x_), static_cast<int>(y_)}; }

    /// Return Vector2 vector (z component is ignored).
    Vector2 ToVector2() const { return {x_, y_}; }

    /// Return IntVector3 vector.
    IntVector3 ToIntVector3() const { return {static_cast<int>(x_), static_cast<int>(y_), static_cast<int>(z_)}; }

    /// Return Vector4 vector.
    Vector4 ToVector4(float w = 0.0f) const;

    /// Return x and z components as 2D vector (y component is ignored).
    Vector2 ToXZ() const { return { x_, z_ }; }

    inline static Vector3 Min(const Vector3& lhs, const Vector3& rhs) { 
        return { Se::Min(lhs.x_, rhs.x_), Se::Min(lhs.y_, rhs.y_), Se::Min(lhs.z_, rhs.z_)}; 
    }


    inline static Vector3 Max(const Vector3& lhs, const Vector3& rhs) {
        return { Se::Max(lhs.x_, rhs.x_), Se::Max(lhs.y_, rhs.y_), Se::Max(lhs.z_, rhs.z_)};
    }


    /// Zero vector.
    static const Vector3 ZERO;
    /// (-1,0,0) vector.
    static const Vector3 LEFT;
    /// (1,0,0) vector.
    static const Vector3 RIGHT;
    /// (0,1,0) vector.
    static const Vector3 UP;
    /// (0,-1,0) vector.
    static const Vector3 DOWN;
    /// (0,0,1) vector.
    static const Vector3 FORWARD;
    /// (0,0,-1) vector.
    static const Vector3 BACK;
    /// (1,1,1) vector.
    static const Vector3 ONE;
};

inline const Vector3 Vector3::ZERO;
inline const Vector3 Vector3::LEFT(-1.0f, 0.0f, 0.0f);
inline const Vector3 Vector3::RIGHT(1.0f, 0.0f, 0.0f);
inline const Vector3 Vector3::UP(0.0f, 1.0f, 0.0f);
inline const Vector3 Vector3::DOWN(0.0f, -1.0f, 0.0f);
inline const Vector3 Vector3::FORWARD(0.0f, 0.0f, 1.0f);
inline const Vector3 Vector3::BACK(0.0f, 0.0f, -1.0f);
inline const Vector3 Vector3::ONE(1.0f, 1.0f, 1.0f);

/// Multiply Vector3 with a scalar.
inline Vector3 operator *(float lhs, const Vector3& rhs) { return rhs * lhs; }
/// Multiply IntVector3 with a scalar.
inline IntVector3 operator *(int lhs, const IntVector3& rhs) { return rhs * lhs; }

/// Per-component linear interpolation between two 3-vectors.
inline Vector3 VectorLerp(const Vector3& lhs, const Vector3& rhs, const Vector3& t) { return lhs + (rhs - lhs) * t; }

/// Per-component min of two 3-vectors.
inline Vector3 VectorMin(const Vector3& lhs, const Vector3& rhs) { return Vector3(Min(lhs.x_, rhs.x_), Min(lhs.y_, rhs.y_), Min(lhs.z_, rhs.z_)); }

/// Per-component max of two 3-vectors.
inline Vector3 VectorMax(const Vector3& lhs, const Vector3& rhs) { return Vector3(Max(lhs.x_, rhs.x_), Max(lhs.y_, rhs.y_), Max(lhs.z_, rhs.z_)); }

/// Per-component floor of 3-vector.
inline Vector3 VectorFloor(const Vector3& vec) { return Vector3(Floor(vec.x_), Floor(vec.y_), Floor(vec.z_)); }

/// Per-component round of 3-vector.
inline Vector3 VectorRound(const Vector3& vec) { return Vector3(Round(vec.x_), Round(vec.y_), Round(vec.z_)); }

/// Per-component ceil of 3-vector.
inline Vector3 VectorCeil(const Vector3& vec) { return Vector3(Ceil(vec.x_), Ceil(vec.y_), Ceil(vec.z_)); }

/// Per-component absolute value of 3-vector.
inline Vector3 VectorAbs(const Vector3& vec) { return Vector3(Abs(vec.x_), Abs(vec.y_), Abs(vec.z_)); }

/// Per-component square root of 3-vector.
inline Vector3 VectorSqrt(const Vector3& vec) { return Vector3(Sqrt(vec.x_), Sqrt(vec.y_), Sqrt(vec.z_)); }

/// Per-component floor of 3-vector. Returns IntVector3.
inline IntVector3 VectorFloorToInt(const Vector3& vec) { return IntVector3(FloorToInt(vec.x_), FloorToInt(vec.y_), FloorToInt(vec.z_)); }

/// Per-component round of 3-vector. Returns IntVector3.
inline IntVector3 VectorRoundToInt(const Vector3& vec) { return IntVector3(RoundToInt(vec.x_), RoundToInt(vec.y_), RoundToInt(vec.z_)); }

/// Per-component ceil of 3-vector. Returns IntVector3.
inline IntVector3 VectorCeilToInt(const Vector3& vec) { return IntVector3(CeilToInt(vec.x_), CeilToInt(vec.y_), CeilToInt(vec.z_)); }


/// Return a random value from [0, 1) from 3-vector seed.
inline float StableRandom(const Vector3& seed) { return StableRandom(Vector2(StableRandom(Vector2(seed.x_, seed.y_)), seed.z_)); }

inline int Compare(const Vector3 &v0,const Vector3 &v1) {
    return (Compare(v0.x_, v1.x_, M_EPSILON) && Compare(v0.y_,v1.y_, M_EPSILON) && Compare(v0.z_,v1.z_, M_EPSILON));
}

inline int Compare(const Vector3 &v0,const Vector3 &v1, float epsilon) {
    return (Compare(v0.x_, v1.x_, epsilon) && Compare(v0.y_, v1.y_, epsilon) && Compare(v0.z_, v1.z_, epsilon));
}

inline int Compare(const IntVector3 &v0,const IntVector3 &v1) {
    return (v0.x_ == v1.x_) && (v0.y_ == v1.y_) && (v0.z_ == v1.z_);
}


inline Vector3 Clamp(const Vector3 &v, const Vector3 &v0, const Vector3 &v1) {
    Vector3 ret;
    ret.x_ = Clamp(v.x_,v0.x_,v1.x_);
    ret.y_ = Clamp(v.y_,v0.y_,v1.y_);
    ret.z_ = Clamp(v.z_,v0.z_,v1.z_);
    return ret;
}

// inline Vector3 VectorMin(const Vector3 &v0,const Vector3 &v1) {
//     Vector3 ret;
//     ret.x_ = Min(v0.x_,v1.x_);
//     ret.y_ = Min(v0.y_,v1.y_);
//     ret.z_ = Min(v0.z_,v1.z_);
//     return ret;
// }

// inline Vector3 VectorMax(const Vector3 &v0,const Vector3 &v1) {
//     Vector3 ret;
//     ret.x_ = Max(v0.x_,v1.x_);
//     ret.y_ = Max(v0.y_,v1.y_);
//     ret.z_ = Max(v0.z_,v1.z_);
//     return ret;
// }

// ///
// int rayBoundBoxIntersection(const Vector3 &point,const Vector3 &dir, const Vector3 &min, const Vector3 &max);
// ///
// int irayBoundBoxIntersection(const Vector3 &point, const Vector3 &idir,const Vector3 &min, const Vector3 &max);
// ///
// int rayTriangleIntersection(const Vector3 &point, const Vector3 &dir,
//                             const Vector3 &v0, const Vector3 &v1, const Vector3 &v2);

/// Return IntVector3 vector.
inline IntVector3 IntVector2::ToIntVector3(int z) const { return { x_, y_, z }; }

/// Return IntVector3 vector.
inline IntVector3 Vector2::ToIntVector3(int z) const { return { static_cast<int>(x_), static_cast<int>(y_), z }; }

/// Return Vector3 vector.
inline Vector3 IntVector2::ToVector3(float z) const { return { static_cast<float>(x_), static_cast<float>(y_), z }; }

/// Return Vector3 vector.
inline Vector3 Vector2::ToVector3(float z) const { return { x_, y_, z }; }

/// Return Vector3 vector.
inline Vector3 IntVector3::ToVector3() const { return { static_cast<float>(x_), static_cast<float>(y_), static_cast<float>(z_) }; }

}
