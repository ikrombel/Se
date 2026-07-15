#pragma once

#include <SeMath/Matrix3.hpp>
//#include <SeMath/Matrix4.hpp>

#ifdef USE_SSE
#include <emmintrin.h>
#endif

namespace Se
{

class Matrix4;

/// Rotation represented as a four-dimensional normalized vector.
class Quaternion
{
public:
    /// Construct an identity quaternion.
    Quaternion() noexcept
#ifndef USE_SSE
       :w_(1.0f),
        x_(0.0f),
        y_(0.0f),
        z_(0.0f)
#endif
    {
#ifdef USE_SSE
        _mm_storeu_ps(&w_, _mm_set_ps(0.f, 0.f, 0.f, 1.f));
#endif
    }

    /// Copy-construct from another quaternion.
    Quaternion(const Quaternion& quat) noexcept
#if defined(USE_SSE) && (!defined(_MSC_VER) || _MSC_VER >= 1700) /* Visual Studio 2012 and newer. VS2010 has a bug with these, see https://github.com/urho3d/Se/issues/1044 */
    {
        _mm_storeu_ps(&w_, _mm_loadu_ps(&quat.w_));
    }
#else
       :w_(quat.w_),
        x_(quat.x_),
        y_(quat.y_),
        z_(quat.z_)
    {
    }
#endif

    /// Construct from values.
    Quaternion(float w, float x, float y, float z) noexcept
#ifndef USE_SSE
       :w_(w),
        x_(x),
        y_(y),
        z_(z)
#endif
    {
#ifdef USE_SSE
        _mm_storeu_ps(&w_, _mm_set_ps(z, y, x, w));
#endif
    }

    /// Construct from a float array.
    explicit Quaternion(const float* data) noexcept
#ifndef USE_SSE
       :w_(data[0]),
        x_(data[1]),
        y_(data[2]),
        z_(data[3])
#endif
    {
#ifdef USE_SSE
        _mm_storeu_ps(&w_, _mm_loadu_ps(data));
#endif
    }

    /// Construct from an angle (in degrees) and axis.
    Quaternion(float angle, const Vector3& axis) noexcept
    {
        FromAngleAxis(angle, axis);
    }

    /// Construct from Euler angles (in degrees).
    explicit Quaternion(const Vector3& angles) noexcept
    {
        FromEulerAngles(angles.x_, angles.y_, angles.z_);
    }

    /// Construct from an angle (in degrees, for GFD).
    explicit Quaternion(float angle) noexcept
    {
        FromAngleAxis(angle, Vector3::FORWARD);
    }

    /// Construct from Euler angles (in degrees.) Equivalent to Y*X*Z.
    Quaternion(float x, float y, float z) noexcept
    {
        FromEulerAngles(x, y, z);
    }

    /// Construct from the rotation difference between two direction vectors.
    Quaternion(const Vector3& start, const Vector3& end) noexcept
    {
        FromRotationTo(start, end);
    }

    /// Construct from orthonormal axes.
    Quaternion(const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis) noexcept
    {
        FromAxes(xAxis, yAxis, zAxis);
    }

    /// Construct from a rotation matrix.
    explicit Quaternion(const Matrix3& matrix) noexcept
    {
        FromRotationMatrix(matrix);
    }

    /// Construct from angular velocity assuming unit time.
    /// Note: Absolute value of angular velocity is measured in radians.
    static Quaternion FromAngularVelocity(const Vector3& angularVelocity);


#ifdef USE_SSE
    explicit Quaternion(__m128 wxyz) noexcept
    {
        _mm_storeu_ps(&w_, wxyz);
    }
#endif

    /// Return mutable value by index.
    float& operator[](unsigned index) { return (&x_)[index]; }

    /// Assign from another quaternion.
    Quaternion& operator =(const Quaternion& rhs) noexcept
    {
#if defined(USE_SSE) && (!defined(_MSC_VER) || _MSC_VER >= 1700) /* Visual Studio 2012 and newer. VS2010 has a bug with these, see https://github.com/urho3d/Se/issues/1044 */
        _mm_storeu_ps(&w_, _mm_loadu_ps(&rhs.w_));
#else
        w_ = rhs.w_;
        x_ = rhs.x_;
        y_ = rhs.y_;
        z_ = rhs.z_;
#endif
        return *this;
    }

    /// Add-assign a quaternion.
    Quaternion& operator +=(const Quaternion& rhs)
    {
#ifdef USE_SSE
        _mm_storeu_ps(&w_, _mm_add_ps(_mm_loadu_ps(&w_), _mm_loadu_ps(&rhs.w_)));
#else
        w_ += rhs.w_;
        x_ += rhs.x_;
        y_ += rhs.y_;
        z_ += rhs.z_;
#endif
        return *this;
    }

    /// Multiply-assign a scalar.
    Quaternion& operator *=(float rhs)
    {
#ifdef USE_SSE
        _mm_storeu_ps(&w_, _mm_mul_ps(_mm_loadu_ps(&w_), _mm_set1_ps(rhs)));
#else
        w_ *= rhs;
        x_ *= rhs;
        y_ *= rhs;
        z_ *= rhs;
#endif
        return *this;
    }

    /// Test for equality with another quaternion without epsilon.
    bool operator ==(const Quaternion& rhs) const
    {
#ifdef USE_SSE
        __m128 c = _mm_cmpeq_ps(_mm_loadu_ps(&w_), _mm_loadu_ps(&rhs.w_));
        c = _mm_and_ps(c, _mm_movehl_ps(c, c));
        c = _mm_and_ps(c, _mm_shuffle_ps(c, c, _MM_SHUFFLE(1, 1, 1, 1)));
        return _mm_cvtsi128_si32(_mm_castps_si128(c)) == -1;
#else
        return w_ == rhs.w_ && x_ == rhs.x_ && y_ == rhs.y_ && z_ == rhs.z_;
#endif
    }

    /// Test for inequality with another quaternion without epsilon.
    bool operator !=(const Quaternion& rhs) const { return !(*this == rhs); }

    /// Multiply with a scalar.
    Quaternion operator *(float rhs) const
    {
#ifdef USE_SSE
        return Quaternion(_mm_mul_ps(_mm_loadu_ps(&w_), _mm_set1_ps(rhs)));
#else
        return Quaternion(w_ * rhs, x_ * rhs, y_ * rhs, z_ * rhs);
#endif
    }

    /// Return negation.
    Quaternion operator -() const
    {
#ifdef USE_SSE
        return Quaternion(_mm_xor_ps(_mm_loadu_ps(&w_), _mm_castsi128_ps(_mm_set1_epi32((int)0x80000000UL))));
#else
        return Quaternion(-w_, -x_, -y_, -z_);
#endif
    }

    /// Add a quaternion.
    Quaternion operator +(const Quaternion& rhs) const
    {
#ifdef USE_SSE
        return Quaternion(_mm_add_ps(_mm_loadu_ps(&w_), _mm_loadu_ps(&rhs.w_)));
#else
        return Quaternion(w_ + rhs.w_, x_ + rhs.x_, y_ + rhs.y_, z_ + rhs.z_);
#endif
    }

    /// Subtract a quaternion.
    Quaternion operator -(const Quaternion& rhs) const
    {
#ifdef USE_SSE
        return Quaternion(_mm_sub_ps(_mm_loadu_ps(&w_), _mm_loadu_ps(&rhs.w_)));
#else
        return Quaternion(w_ - rhs.w_, x_ - rhs.x_, y_ - rhs.y_, z_ - rhs.z_);
#endif
    }

    /// Multiply a quaternion.
    Quaternion operator *(const Quaternion& rhs) const
    {
#ifdef USE_SSE
        __m128 q1 = _mm_loadu_ps(&w_);
        __m128 q2 = _mm_loadu_ps(&rhs.w_);
        q2 = _mm_shuffle_ps(q2, q2, _MM_SHUFFLE(0, 3, 2, 1));
        const __m128 signy = _mm_castsi128_ps(_mm_set_epi32((int)0x80000000UL, (int)0x80000000UL, 0, 0));
        const __m128 signx = _mm_shuffle_ps(signy, signy, _MM_SHUFFLE(2, 0, 2, 0));
        const __m128 signz = _mm_shuffle_ps(signy, signy, _MM_SHUFFLE(3, 0, 0, 3));
        __m128 out = _mm_mul_ps(_mm_shuffle_ps(q1, q1, _MM_SHUFFLE(1, 1, 1, 1)), _mm_shuffle_ps(q2, q2, _MM_SHUFFLE(0, 1, 2, 3)));
        out = _mm_add_ps(_mm_mul_ps(_mm_xor_ps(signy, _mm_shuffle_ps(q1, q1, _MM_SHUFFLE(2, 2, 2, 2))), _mm_shuffle_ps(q2, q2, _MM_SHUFFLE(1, 0, 3, 2))), _mm_xor_ps(signx, out));
        out = _mm_add_ps(_mm_mul_ps(_mm_xor_ps(signz, _mm_shuffle_ps(q1, q1, _MM_SHUFFLE(3, 3, 3, 3))), _mm_shuffle_ps(q2, q2, _MM_SHUFFLE(2, 3, 0, 1))), out);
        out = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(q1, q1, _MM_SHUFFLE(0, 0, 0, 0)), q2), out);
        return Quaternion(_mm_shuffle_ps(out, out, _MM_SHUFFLE(2, 1, 0, 3)));
#else
        return Quaternion(
            w_ * rhs.w_ - x_ * rhs.x_ - y_ * rhs.y_ - z_ * rhs.z_,
            w_ * rhs.x_ + x_ * rhs.w_ + y_ * rhs.z_ - z_ * rhs.y_,
            w_ * rhs.y_ + y_ * rhs.w_ + z_ * rhs.x_ - x_ * rhs.z_,
            w_ * rhs.z_ + z_ * rhs.w_ + x_ * rhs.y_ - y_ * rhs.x_
        );
#endif
    }

    /// Multiply a Vector3.
    Vector3 operator *(const Vector3& rhs) const
    {
#ifdef USE_SSE
        __m128 q = _mm_loadu_ps(&w_);
        q = _mm_shuffle_ps(q, q, _MM_SHUFFLE(0, 3, 2, 1));
        __m128 v = _mm_set_ps(0.f, rhs.z_, rhs.y_, rhs.x_);
        const __m128 W = _mm_shuffle_ps(q, q, _MM_SHUFFLE(3, 3, 3, 3));
        const __m128 a_yzx = _mm_shuffle_ps(q, q, _MM_SHUFFLE(3, 0, 2, 1));
        __m128 x = _mm_mul_ps(q, _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 0, 2, 1)));
        __m128 qxv = _mm_sub_ps(x, _mm_mul_ps(a_yzx, v));
        __m128 Wv = _mm_mul_ps(W, v);
        __m128 s = _mm_add_ps(qxv, _mm_shuffle_ps(Wv, Wv, _MM_SHUFFLE(3, 1, 0, 2)));
        __m128 qs = _mm_mul_ps(q, s);
        __m128 y = _mm_shuffle_ps(qs, qs, _MM_SHUFFLE(3, 1, 0, 2));
        s = _mm_sub_ps(_mm_mul_ps(a_yzx, s), y);
        s = _mm_add_ps(s, s);
        s = _mm_add_ps(s, v);

        return Vector3(
            _mm_cvtss_f32(s),
            _mm_cvtss_f32(_mm_shuffle_ps(s, s, _MM_SHUFFLE(1, 1, 1, 1))),
            _mm_cvtss_f32(_mm_movehl_ps(s, s)));
#else
        Vector3 qVec(x_, y_, z_);
        Vector3 cross1(qVec.CrossProduct(rhs));
        Vector3 cross2(qVec.CrossProduct(cross1));

        return rhs + 2.0f * (cross1 * w_ + cross2);
#endif
    }

    /// Define from an angle (in degrees) and axis.
    void FromAngleAxis(float angle, const Vector3& axis);
    /// Define from Euler angles (in degrees.) Equivalent to Y*X*Z.
    void FromEulerAngles(float x, float y, float z);
    /// Define from the rotation difference between two direction vectors.
    void FromRotationTo(const Vector3& start, const Vector3& end);
    /// Define from orthonormal axes.
    void FromAxes(const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis);
    /// Define from a rotation matrix.
    void FromRotationMatrix(const Matrix3& matrix);
    /// Define from a direction to look in and an up direction. Return true if successful, or false if would result in a NaN, in which case the current value remains.
    bool FromLookRotation(const Vector3& direction, const Vector3& up = Vector3::UP);

    /// Normalize to unit length.
    void Normalize()
    {
#ifdef USE_SSE
        __m128 q = _mm_loadu_ps(&w_);
        __m128 n = _mm_mul_ps(q, q);
        n = _mm_add_ps(n, _mm_shuffle_ps(n, n, _MM_SHUFFLE(2, 3, 0, 1)));
        n = _mm_add_ps(n, _mm_shuffle_ps(n, n, _MM_SHUFFLE(0, 1, 2, 3)));
        __m128 e = _mm_rsqrt_ps(n);
        __m128 e3 = _mm_mul_ps(_mm_mul_ps(e, e), e);
        __m128 half = _mm_set1_ps(0.5f);
        n = _mm_add_ps(e, _mm_mul_ps(half, _mm_sub_ps(e, _mm_mul_ps(n, e3))));
        _mm_storeu_ps(&w_, _mm_mul_ps(q, n));
#else
        float lenSquared = LengthSquared();
        if (!Se::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
        {
            float invLen = 1.0f / sqrtf(lenSquared);
            w_ *= invLen;
            x_ *= invLen;
            y_ *= invLen;
            z_ *= invLen;
        }
#endif
    }

    /// Return normalized to unit length.
    Quaternion Normalized() const
    {
#ifdef USE_SSE
        __m128 q = _mm_loadu_ps(&w_);
        __m128 n = _mm_mul_ps(q, q);
        n = _mm_add_ps(n, _mm_shuffle_ps(n, n, _MM_SHUFFLE(2, 3, 0, 1)));
        n = _mm_add_ps(n, _mm_shuffle_ps(n, n, _MM_SHUFFLE(0, 1, 2, 3)));
        __m128 e = _mm_rsqrt_ps(n);
        __m128 e3 = _mm_mul_ps(_mm_mul_ps(e, e), e);
        __m128 half = _mm_set1_ps(0.5f);
        n = _mm_add_ps(e, _mm_mul_ps(half, _mm_sub_ps(e, _mm_mul_ps(n, e3))));
        return Quaternion(_mm_mul_ps(q, n));
#else
        float lenSquared = LengthSquared();
        if (!Se::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
        {
            float invLen = 1.0f / sqrtf(lenSquared);
            return *this * invLen;
        }
        else
            return *this;
#endif
    }

    /// Return inverse.
    Quaternion Inverse() const
    {
#ifdef USE_SSE
        __m128 q = _mm_loadu_ps(&w_);
        __m128 n = _mm_mul_ps(q, q);
        n = _mm_add_ps(n, _mm_shuffle_ps(n, n, _MM_SHUFFLE(2, 3, 0, 1)));
        n = _mm_add_ps(n, _mm_shuffle_ps(n, n, _MM_SHUFFLE(0, 1, 2, 3)));
        return Quaternion(_mm_div_ps(_mm_xor_ps(q, _mm_castsi128_ps(_mm_set_epi32((int)0x80000000UL, (int)0x80000000UL, (int)0x80000000UL, 0))), n));
#else
        float lenSquared = LengthSquared();
        if (lenSquared == 1.0f)
            return Conjugate();
        else if (lenSquared >= M_EPSILON)
            return Conjugate() * (1.0f / lenSquared);
        else
            return IDENTITY;
#endif
    }

    /// Return squared length.
    float LengthSquared() const
    {
#ifdef USE_SSE
        __m128 q = _mm_loadu_ps(&w_);
        __m128 n = _mm_mul_ps(q, q);
        n = _mm_add_ps(n, _mm_shuffle_ps(n, n, _MM_SHUFFLE(2, 3, 0, 1)));
        n = _mm_add_ps(n, _mm_shuffle_ps(n, n, _MM_SHUFFLE(0, 1, 2, 3)));
        return _mm_cvtss_f32(n);
#else
        return w_ * w_ + x_ * x_ + y_ * y_ + z_ * z_;
#endif
    }

    /// Calculate dot product.
    float DotProduct(const Quaternion& rhs) const
    {
#ifdef USE_SSE
        __m128 q1 = _mm_loadu_ps(&w_);
        __m128 q2 = _mm_loadu_ps(&rhs.w_);
        __m128 n = _mm_mul_ps(q1, q2);
        n = _mm_add_ps(n, _mm_shuffle_ps(n, n, _MM_SHUFFLE(2, 3, 0, 1)));
        n = _mm_add_ps(n, _mm_shuffle_ps(n, n, _MM_SHUFFLE(0, 1, 2, 3)));
        return _mm_cvtss_f32(n);
#else
        return w_ * rhs.w_ + x_ * rhs.x_ + y_ * rhs.y_ + z_ * rhs.z_;
#endif
    }

    /// Test for equality with another quaternion with epsilon.
    bool Equals(const Quaternion& rhs, float eps = M_EPSILON) const
    {
        return Se::Equals(w_, rhs.w_, eps) && Se::Equals(x_, rhs.x_, eps) &&
        Se::Equals(y_, rhs.y_, eps) && Se::Equals(z_, rhs.z_, eps);
    }

    /// Test for equivalence with another quaternion with epsilon.
    bool Equivalent(const Quaternion& rhs, float eps = M_EPSILON) const
    {
        return Se::Equals(Abs(DotProduct(rhs)), 1.0f, eps);
    }


    /// Return whether is NaN.
    bool IsNaN() const { return Se::IsNaN(w_) || Se::IsNaN(x_) || Se::IsNaN(y_) || Se::IsNaN(z_); }

    /// Return whether any element is Inf.
    bool IsInf() const { return Se::IsInf(w_) || Se::IsInf(x_) || Se::IsInf(y_) || Se::IsInf(z_); }


    /// Return conjugate.
    Quaternion Conjugate() const
    {
#ifdef USE_SSE
        __m128 q = _mm_loadu_ps(&w_);
        return Quaternion(_mm_xor_ps(q, _mm_castsi128_ps(_mm_set_epi32((int)0x80000000UL, (int)0x80000000UL, (int)0x80000000UL, 0))));
#else
        return Quaternion(w_, -x_, -y_, -z_);
#endif
    }

    Matrix3 GetMat3() const;

    /// Return Euler angles in degrees.
    Vector3 EulerAngles() const;
    /// Return yaw angle in degrees.
    float YawAngle() const {
        return EulerAngles().y_; }
    /// Return pitch angle in degrees.
    float PitchAngle() const {
        return EulerAngles().x_; }
    /// Return roll angle in degrees.
    float RollAngle() const {
        return EulerAngles().z_; }
    /// Return rotation axis.
    Vector3 Axis() const {
        return Vector3(x_, y_, z_) / sqrt(1. - w_ * w_); }
    /// Return rotation angle.
    float Angle() const {
        return 2 * Acos(w_); }
    /// Return angular velocity assuming unit time.
    /// Note: Absolute value of angular velocity is measured in radians.
    Vector3 AngularVelocity() const;
    /// Return the rotation matrix that corresponds to this quaternion.
    Matrix3 RotationMatrix() const;

    /// Spherical interpolation with another quaternion.
    Quaternion Slerp(const Quaternion& rhs, float t) const;
    /// Normalized linear interpolation with another quaternion.
    Quaternion Nlerp(const Quaternion& rhs, float t, bool shortestPath = false) const;
    /// Decompose quaternion to swing and twist components. swing * twist is the original quaternion.
    std::pair<Quaternion, Quaternion> ToSwingTwist(const Vector3& twistAxis) const;

    /// Return float data.
    const float* Data() const { return &w_; }

    /// Return as string.
    String ToString() const {
        return cformat("%g %g %g %g", w_, x_, y_, z_);
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

    static void RotationMatrix(const Matrix4& matrix, Quaternion& result);
    union {
        struct {
            /// W coordinate.
            float w_;
            /// X coordinate.
            float x_;
            /// Y coordinate.
            float y_;
            /// Z coordinate.
            float z_;
        };
        float data[4];
    };

    /// Identity quaternion.
    static const Quaternion IDENTITY;
    /// Zero quaternion.
    static const Quaternion ZERO;
};

inline const Quaternion Quaternion::ZERO{0.0f, 0.0f, 0.0f, 0.0f};
inline const Quaternion Quaternion::IDENTITY;

inline void Quaternion::FromAngleAxis(float angle, const Vector3& axis)
{
    Vector3 normAxis = axis.Normalized();
    angle *= M_DEGTORAD_2;
    float sinAngle = sinf(angle);
    float cosAngle = cosf(angle);

    w_ = cosAngle;
    x_ = normAxis.x_ * sinAngle;
    y_ = normAxis.y_ * sinAngle;
    z_ = normAxis.z_ * sinAngle;
}

inline void Quaternion::FromEulerAngles(float x, float y, float z)
{
    // Order of rotations: Z first, then X, then Y (mimics typical FPS camera with gimbal lock at top/bottom)
    x *= M_DEGTORAD_2;
    y *= M_DEGTORAD_2;
    z *= M_DEGTORAD_2;
    float sinX = sinf(x);
    float cosX = cosf(x);
    float sinY = sinf(y);
    float cosY = cosf(y);
    float sinZ = sinf(z);
    float cosZ = cosf(z);

    w_ = cosY * cosX * cosZ + sinY * sinX * sinZ;
    x_ = cosY * sinX * cosZ + sinY * cosX * sinZ;
    y_ = sinY * cosX * cosZ - cosY * sinX * sinZ;
    z_ = cosY * cosX * sinZ - sinY * sinX * cosZ;
}

inline void Quaternion::FromRotationTo(const Vector3& start, const Vector3& end)
{
    Vector3 normStart = start.Normalized();
    Vector3 normEnd = end.Normalized();
    float d = normStart.DotProduct(normEnd);

    if (d > -1.0f + M_EPSILON)
    {
        Vector3 c = normStart.CrossProduct(normEnd);
        float s = sqrtf((1.0f + d) * 2.0f);
        float invS = 1.0f / s;

        x_ = c.x_ * invS;
        y_ = c.y_ * invS;
        z_ = c.z_ * invS;
        w_ = 0.5f * s;
    }
    else
    {
        Vector3 axis = Vector3::RIGHT.CrossProduct(normStart);
        if (axis.Length() < M_EPSILON)
            axis = Vector3::UP.CrossProduct(normStart);

        FromAngleAxis(180.f, axis);
    }
}

inline void Quaternion::FromAxes(const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis)
{
    Matrix3 matrix(
        xAxis.x_, yAxis.x_, zAxis.x_,
        xAxis.y_, yAxis.y_, zAxis.y_,
        xAxis.z_, yAxis.z_, zAxis.z_
    );

    FromRotationMatrix(matrix);
}

inline void Quaternion::FromRotationMatrix(const Matrix3& matrix)
{
    float t = matrix.m00_ + matrix.m11_ + matrix.m22_;

    if (t > 0.0f)
    {
        float invS = 0.5f / sqrtf(1.0f + t);

        x_ = (matrix.m21_ - matrix.m12_) * invS;
        y_ = (matrix.m02_ - matrix.m20_) * invS;
        z_ = (matrix.m10_ - matrix.m01_) * invS;
        w_ = 0.25f / invS;
    }
    else
    {
        if (matrix.m00_ > matrix.m11_ && matrix.m00_ > matrix.m22_)
        {
            float invS = 0.5f / sqrtf(1.0f + matrix.m00_ - matrix.m11_ - matrix.m22_);

            x_ = 0.25f / invS;
            y_ = (matrix.m01_ + matrix.m10_) * invS;
            z_ = (matrix.m20_ + matrix.m02_) * invS;
            w_ = (matrix.m21_ - matrix.m12_) * invS;
        }
        else if (matrix.m11_ > matrix.m22_)
        {
            float invS = 0.5f / sqrtf(1.0f + matrix.m11_ - matrix.m00_ - matrix.m22_);

            x_ = (matrix.m01_ + matrix.m10_) * invS;
            y_ = 0.25f / invS;
            z_ = (matrix.m12_ + matrix.m21_) * invS;
            w_ = (matrix.m02_ - matrix.m20_) * invS;
        }
        else
        {
            float invS = 0.5f / sqrtf(1.0f + matrix.m22_ - matrix.m00_ - matrix.m11_);

            x_ = (matrix.m02_ + matrix.m20_) * invS;
            y_ = (matrix.m12_ + matrix.m21_) * invS;
            z_ = 0.25f / invS;
            w_ = (matrix.m10_ - matrix.m01_) * invS;
        }
    }
}

inline bool Quaternion::FromLookRotation(const Vector3& direction, const Vector3& up)
{
    Quaternion ret;
    Vector3 forward = direction.Normalized();

    Vector3 v = forward.CrossProduct(up);
    // If direction & up are parallel and crossproduct becomes zero, use FromRotationTo() fallback
    if (v.LengthSquared() >= M_EPSILON)
    {
        v.Normalize();
        Vector3 up = v.CrossProduct(forward);
        Vector3 right = up.CrossProduct(forward);
        ret.FromAxes(right, up, forward);
    }
    else
        ret.FromRotationTo(Vector3::FORWARD, forward);

    if (!ret.IsNaN())
    {
        (*this) = ret;
        return true;
    }
    else
        return false;
}

inline Quaternion Quaternion::FromAngularVelocity(const Vector3& angularVelocity)
{
    const float len2 = angularVelocity.LengthSquared();
    if (len2 < M_EPSILON * M_EPSILON)
        return Quaternion::IDENTITY;

    const float len = sqrtf(len2);
    const Vector3 direction = angularVelocity / len;
    const float angle = len * M_RADTODEG;
    return Quaternion{angle, direction};
}

inline Matrix3 Quaternion::GetMat3() const {
    Matrix3 ret;
    float x2 = x_ + x_;
    float y2 = y_ + y_;
    float z2 = z_ + z_;
    float xx2 = x_ * x2;
    float yy2 = y_ * y2;
    float zz2 = z_ * z2;
    float zx2 = z_ * x2;
    float xy2 = x_ * y2;
    float yz2 = y_ * z2;
    float wx2 = w_ * x2;
    float wy2 = w_ * y2;
    float wz2 = w_ * z2;
    ret.m00_ = 1.0f - yy2 - zz2; ret.m01_ = xy2 - wz2;        ret.m02_ = zx2 + wy2;
    ret.m10_ = xy2 + wz2;        ret.m11_ = 1.0f - xx2 - zz2; ret.m12_ = yz2 - wx2;
    ret.m20_ = zx2 - wy2;        ret.m21_ = yz2 + wx2;        ret.m22_ = 1.0f - xx2 - yy2;
    return ret;
}

inline Vector3 Quaternion::EulerAngles() const
{
    // Derivation from http://www.geometrictools.com/Documentation/EulerAngles.pdf
    // Order of rotations: Z first, then X, then Y
    float check = 2.0f * (-y_ * z_ + w_ * x_);

    if (check < -0.995f)
    {
        return Vector3(
            -90.0f,
            0.0f,
            -atan2f(2.0f * (x_ * z_ - w_ * y_), 1.0f - 2.0f * (y_ * y_ + z_ * z_)) * M_RADTODEG
        );
    }
    else if (check > 0.995f)
    {
        return Vector3(
            90.0f,
            0.0f,
            atan2f(2.0f * (x_ * z_ - w_ * y_), 1.0f - 2.0f * (y_ * y_ + z_ * z_)) * M_RADTODEG
        );
    }
    else
    {
        return Vector3(
            asinf(check) * M_RADTODEG,
            atan2f(2.0f * (x_ * z_ + w_ * y_), 1.0f - 2.0f * (x_ * x_ + y_ * y_)) * M_RADTODEG,
            atan2f(2.0f * (x_ * y_ + w_ * z_), 1.0f - 2.0f * (x_ * x_ + z_ * z_)) * M_RADTODEG
        );
    }
}

inline bool Compare(const Quaternion &v0,const Quaternion &v1) {
    return (v0 == v1);
}

inline bool Compare(const Quaternion &v0,const Quaternion &v1, float epsilon) {
    return (v0 == v1);
}

Quaternion &Slerp(Quaternion &ret,const Quaternion &q0,const Quaternion &q1, float k);

inline Quaternion Slerp(const Quaternion &q0, const Quaternion &q1, float k) {
    Quaternion ret;
    return Slerp(ret,q0,q1,k);
}

inline Vector3 Quaternion::AngularVelocity() const
{
    const float axisScaleInv = sqrt(Max(0.0f, 1.0f - w_ * w_));
    if (axisScaleInv < M_EPSILON)
        return Vector3::ZERO;

    const Vector3 axis = Vector3(x_, y_, z_) / axisScaleInv;
    const float angleRad = 2 * acos(w_);
    return axis * angleRad;
}

inline Matrix3 Quaternion::RotationMatrix() const
{
    return Matrix3(
        1.0f - 2.0f * y_ * y_ - 2.0f * z_ * z_,
        2.0f * x_ * y_ - 2.0f * w_ * z_,
        2.0f * x_ * z_ + 2.0f * w_ * y_,
        2.0f * x_ * y_ + 2.0f * w_ * z_,
        1.0f - 2.0f * x_ * x_ - 2.0f * z_ * z_,
        2.0f * y_ * z_ - 2.0f * w_ * x_,
        2.0f * x_ * z_ - 2.0f * w_ * y_,
        2.0f * y_ * z_ + 2.0f * w_ * x_,
        1.0f - 2.0f * x_ * x_ - 2.0f * y_ * y_
    );
}

inline Quaternion Quaternion::Slerp(const Quaternion& rhs, float t) const
{
    // Use fast approximation for Emscripten builds
#ifdef __EMSCRIPTEN__
    float angle = DotProduct(rhs);
    float sign = 1.f; // Multiply by a sign of +/-1 to guarantee we rotate the shorter arc.
    if (angle < 0.f)
    {
        angle = -angle;
        sign = -1.f;
    }

    float a;
    float b;
    if (angle < 0.999f) // perform spherical linear interpolation.
    {
        // angle = acos(angle); // After this, angle is in the range pi/2 -> 0 as the original angle variable ranged from 0 -> 1.
        angle = (-0.69813170079773212f * angle * angle - 0.87266462599716477f) * angle + 1.5707963267948966f;
        float ta = t*angle;
        // Manually compute the two sines by using a very rough approximation.
        float ta2 = ta*ta;
        b = ((5.64311797634681035370e-03f * ta2 - 1.55271410633428644799e-01f) * ta2 + 9.87862135574673806965e-01f) * ta;
        a = angle - ta;
        float a2 = a*a;
        a = ((5.64311797634681035370e-03f * a2 - 1.55271410633428644799e-01f) * a2 + 9.87862135574673806965e-01f) * a;
    }
    else // If angle is close to taking the denominator to zero, resort to linear interpolation (and normalization).
    {
        a = 1.f - t;
        b = t;
    }
    // Lerp and renormalize.
    return (*this * (a * sign) + rhs * b).Normalized();
#else
    // Favor accuracy for native code builds
    float cosAngle = DotProduct(rhs);
    float sign = 1.0f;
    // Enable shortest path rotation
    if (cosAngle < 0.0f)
    {
        cosAngle = -cosAngle;
        sign = -1.0f;
    }

    float angle = acosf(cosAngle);
    float sinAngle = sinf(angle);
    float t1, t2;

    if (sinAngle > 0.001f)
    {
        float invSinAngle = 1.0f / sinAngle;
        t1 = sinf((1.0f - t) * angle) * invSinAngle;
        t2 = sinf(t * angle) * invSinAngle;
    }
    else
    {
        t1 = 1.0f - t;
        t2 = t;
    }

    return *this * t1 + (rhs * sign) * t2;
#endif
}

inline Quaternion Quaternion::Nlerp(const Quaternion& rhs, float t, bool shortestPath) const
{
    Quaternion result;
    float fCos = DotProduct(rhs);
    if (fCos < 0.0f && shortestPath)
        result = (*this) + (((-rhs) - (*this)) * t);
    else
        result = (*this) + ((rhs - (*this)) * t);
    result.Normalize();
    return result;
}

inline std::pair<Quaternion, Quaternion> Quaternion::ToSwingTwist(const Vector3& twistAxis) const
{
    // https://stackoverflow.com/questions/3684269/component-of-a-quaternion-rotation-around-an-axis
    const Vector3 ra{x_, y_, z_};
    const Vector3 p = twistAxis * ra.ProjectOntoAxis(twistAxis);

    Quaternion twist{w_, p.x_, p.y_, p.z_};
    twist.Normalize();

    const Quaternion swing = *this * twist.Conjugate();
    return {swing, twist};
}


inline Quaternion &Slerp(Quaternion &ret, const Quaternion &q0,const Quaternion &q1, float k) {
    if(k <= 0.0f) {
        ret = q0;
    } else if(k >= 1.0f) {
        ret = q1;
    } else {
        float k0,k1;
        float c = q0.DotProduct(q1);
        float ac = Abs(c);
        if(ac < 1.0f - M_EPSILON) {
            float angle = Acos(ac);
            float is = 1.0f/(Sin(angle));
            k0 = Sin(angle * (1.0f - k)) * is;
            k1 = Sin(angle * k) * is;
        } else {
            k0 = 1.0f - k;
            k1 = k;
        }
        if(c < 0.0f) k1 = -k1;
        ret.x_ = q0.x_ * k0 + q1.x_ * k1;
        ret.y_ = q0.y_ * k0 + q1.y_ * k1;
        ret.z_ = q0.z_ * k0 + q1.z_ * k1;
        ret.w_ = q0.w_ * k0 + q1.w_ * k1;
    }
    return ret;
}

}
