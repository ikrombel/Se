#pragma once

#include <SeMath/Quaternion.hpp>
#include <SeMath/Vector4.hpp>

#ifdef USE_SSE
#include <emmintrin.h>
#endif

namespace Se
{

class Matrix3x4;

/// 4x4 matrix for arbitrary linear transforms including projection.
class Matrix4
{
public:
    /// Construct an identity matrix.
    Matrix4() noexcept
#ifndef USE_SSE
       :m00_(1.0f),
        m01_(0.0f),
        m02_(0.0f),
        m03_(0.0f),
        m10_(0.0f),
        m11_(1.0f),
        m12_(0.0f),
        m13_(0.0f),
        m20_(0.0f),
        m21_(0.0f),
        m22_(1.0f),
        m23_(0.0f),
        m30_(0.0f),
        m31_(0.0f),
        m32_(0.0f),
        m33_(1.0f)
#endif
    {
#ifdef USE_SSE
        _mm_storeu_ps(&m00_, _mm_set_ps(0.f, 0.f, 0.f, 1.f));
        _mm_storeu_ps(&m10_, _mm_set_ps(0.f, 0.f, 1.f, 0.f));
        _mm_storeu_ps(&m20_, _mm_set_ps(0.f, 1.f, 0.f, 0.f));
        _mm_storeu_ps(&m30_, _mm_set_ps(1.f, 0.f, 0.f, 0.f));
#endif
    }

    /// Copy-construct from another matrix.
    Matrix4(const Matrix4& matrix) noexcept
#ifndef USE_SSE
       :m00_(matrix.m00_),
        m01_(matrix.m01_),
        m02_(matrix.m02_),
        m03_(matrix.m03_),
        m10_(matrix.m10_),
        m11_(matrix.m11_),
        m12_(matrix.m12_),
        m13_(matrix.m13_),
        m20_(matrix.m20_),
        m21_(matrix.m21_),
        m22_(matrix.m22_),
        m23_(matrix.m23_),
        m30_(matrix.m30_),
        m31_(matrix.m31_),
        m32_(matrix.m32_),
        m33_(matrix.m33_)
#endif
    {
#ifdef USE_SSE
        _mm_storeu_ps(&m00_, _mm_loadu_ps(&matrix.m00_));
        _mm_storeu_ps(&m10_, _mm_loadu_ps(&matrix.m10_));
        _mm_storeu_ps(&m20_, _mm_loadu_ps(&matrix.m20_));
        _mm_storeu_ps(&m30_, _mm_loadu_ps(&matrix.m30_));
#endif
    }

    /// Copy-construct from a 3x3 matrix and set the extra elements to identity.
    explicit Matrix4(const Matrix3& matrix) noexcept :
        m00_(matrix.m00_),
        m01_(matrix.m01_),
        m02_(matrix.m02_),
        m03_(0.0f),
        m10_(matrix.m10_),
        m11_(matrix.m11_),
        m12_(matrix.m12_),
        m13_(0.0f),
        m20_(matrix.m20_),
        m21_(matrix.m21_),
        m22_(matrix.m22_),
        m23_(0.0f),
        m30_(0.0f),
        m31_(0.0f),
        m32_(0.0f),
        m33_(1.0f)
    {
    }

    explicit Matrix4(float value) noexcept :
            m00_(value),
            m01_(value),
            m02_(value),
            m03_(value),
            m10_(value),
            m11_(value),
            m12_(value),
            m13_(0.0f),
            m20_(value),
            m21_(value),
            m22_(value),
            m23_(0.0f),
            m30_(0.0f),
            m31_(0.0f),
            m32_(0.0f),
            m33_(1.0f)
    {
    }

    /// Construct from values.
    Matrix4(float v00, float v01, float v02, float v03,
            float v10, float v11, float v12, float v13,
            float v20, float v21, float v22, float v23,
            float v30, float v31, float v32, float v33) noexcept :
        m00_(v00),
        m01_(v01),
        m02_(v02),
        m03_(v03),
        m10_(v10),
        m11_(v11),
        m12_(v12),
        m13_(v13),
        m20_(v20),
        m21_(v21),
        m22_(v22),
        m23_(v23),
        m30_(v30),
        m31_(v31),
        m32_(v32),
        m33_(v33)
    {
    }

    Matrix4(const Matrix3 &m,const Vector3 &v) {
        m00_ = m.m00_; m01_ = m.m01_; m02_ = m.m02_; m03_ = v.x_;
        m10_ = m.m10_; m11_ = m.m11_; m12_ = m.m12_; m13_ = v.y_;
        m20_ = m.m20_; m21_ = m.m21_; m22_ = m.m22_; m23_ = v.z_;
        m30_ = 0.0f;  m31_ = 0.0f;  m32_ = 0.0f;  m33_ = 1.0f;
    }

    /// Construct from a float array.
    explicit Matrix4(const float* data) noexcept
#ifndef USE_SSE
       :m00_(data[0]),
        m01_(data[1]),
        m02_(data[2]),
        m03_(data[3]),
        m10_(data[4]),
        m11_(data[5]),
        m12_(data[6]),
        m13_(data[7]),
        m20_(data[8]),
        m21_(data[9]),
        m22_(data[10]),
        m23_(data[11]),
        m30_(data[12]),
        m31_(data[13]),
        m32_(data[14]),
        m33_(data[15])
#endif
    {
#ifdef USE_SSE
        _mm_storeu_ps(&m00_, _mm_loadu_ps(data));
        _mm_storeu_ps(&m10_, _mm_loadu_ps(data + 4));
        _mm_storeu_ps(&m20_, _mm_loadu_ps(data + 8));
        _mm_storeu_ps(&m30_, _mm_loadu_ps(data + 12));
#endif
    }

    /// Return mutable value by index.
    float& operator[](unsigned index) { return (&m00_)[index]; }

    /// Assign from another matrix.
    Matrix4& operator =(const Matrix4& rhs) noexcept
    {
#ifdef USE_SSE
        _mm_storeu_ps(&m00_, _mm_loadu_ps(&rhs.m00_));
        _mm_storeu_ps(&m10_, _mm_loadu_ps(&rhs.m10_));
        _mm_storeu_ps(&m20_, _mm_loadu_ps(&rhs.m20_));
        _mm_storeu_ps(&m30_, _mm_loadu_ps(&rhs.m30_));
#else
        m00_ = rhs.m00_;
        m01_ = rhs.m01_;
        m02_ = rhs.m02_;
        m03_ = rhs.m03_;
        m10_ = rhs.m10_;
        m11_ = rhs.m11_;
        m12_ = rhs.m12_;
        m13_ = rhs.m13_;
        m20_ = rhs.m20_;
        m21_ = rhs.m21_;
        m22_ = rhs.m22_;
        m23_ = rhs.m23_;
        m30_ = rhs.m30_;
        m31_ = rhs.m31_;
        m32_ = rhs.m32_;
        m33_ = rhs.m33_;
#endif
        return *this;
    }

    /// Assign from a 3x3 matrix. Set the extra elements to identity.
    Matrix4& operator =(const Matrix3& rhs) noexcept
    {
        m00_ = rhs.m00_;
        m01_ = rhs.m01_;
        m02_ = rhs.m02_;
        m03_ = 0.0f;
        m10_ = rhs.m10_;
        m11_ = rhs.m11_;
        m12_ = rhs.m12_;
        m13_ = 0.0f;
        m20_ = rhs.m20_;
        m21_ = rhs.m21_;
        m22_ = rhs.m22_;
        m23_ = 0.0f;
        m30_ = 0.0f;
        m31_ = 0.0f;
        m32_ = 0.0f;
        m33_ = 1.0f;
        return *this;
    }

    /// Test for equality with another matrix without epsilon.
    bool operator ==(const Matrix4& rhs) const
    {
#ifdef USE_SSE
        __m128 c0 = _mm_cmpeq_ps(_mm_loadu_ps(&m00_), _mm_loadu_ps(&rhs.m00_));
        __m128 c1 = _mm_cmpeq_ps(_mm_loadu_ps(&m10_), _mm_loadu_ps(&rhs.m10_));
        c0 = _mm_and_ps(c0, c1);
        __m128 c2 = _mm_cmpeq_ps(_mm_loadu_ps(&m20_), _mm_loadu_ps(&rhs.m20_));
        __m128 c3 = _mm_cmpeq_ps(_mm_loadu_ps(&m30_), _mm_loadu_ps(&rhs.m30_));
        c2 = _mm_and_ps(c2, c3);
        c0 = _mm_and_ps(c0, c2);
        __m128 hi = _mm_movehl_ps(c0, c0);
        c0 = _mm_and_ps(c0, hi);
        hi = _mm_shuffle_ps(c0, c0, _MM_SHUFFLE(1, 1, 1, 1));
        c0 = _mm_and_ps(c0, hi);
        return _mm_cvtsi128_si32(_mm_castps_si128(c0)) == -1;
#else
        const float* leftData = Data();
        const float* rightData = rhs.Data();

        for (unsigned i = 0; i < 16; ++i)
        {
            if (leftData[i] != rightData[i])
                return false;
        }

        return true;
#endif
    }

    /// Test for inequality with another matrix without epsilon.
    bool operator !=(const Matrix4& rhs) const { return !(*this == rhs); }

    /// Multiply a Vector3 which is assumed to represent position.
    Vector3 operator *(const Vector3& rhs) const
    {
#ifdef USE_SSE
        __m128 vec = _mm_set_ps(1.f, rhs.z_, rhs.y_, rhs.x_);
        __m128 r0 = _mm_mul_ps(_mm_loadu_ps(&m00_), vec);
        __m128 r1 = _mm_mul_ps(_mm_loadu_ps(&m10_), vec);
        __m128 t0 = _mm_unpacklo_ps(r0, r1);
        __m128 t1 = _mm_unpackhi_ps(r0, r1);
        t0 = _mm_add_ps(t0, t1);
        __m128 r2 = _mm_mul_ps(_mm_loadu_ps(&m20_), vec);
        __m128 r3 = _mm_mul_ps(_mm_loadu_ps(&m30_), vec);
        __m128 t2 = _mm_unpacklo_ps(r2, r3);
        __m128 t3 = _mm_unpackhi_ps(r2, r3);
        t2 = _mm_add_ps(t2, t3);
        vec = _mm_add_ps(_mm_movelh_ps(t0, t2), _mm_movehl_ps(t2, t0));
        vec = _mm_div_ps(vec, _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(3, 3, 3, 3)));
        return Vector3(
            _mm_cvtss_f32(vec),
            _mm_cvtss_f32(_mm_shuffle_ps(vec, vec, _MM_SHUFFLE(1, 1, 1, 1))),
            _mm_cvtss_f32(_mm_movehl_ps(vec, vec)));
#else
        float invW = 1.0f / (m30_ * rhs.x_ + m31_ * rhs.y_ + m32_ * rhs.z_ + m33_);

        return Vector3(
            (m00_ * rhs.x_ + m01_ * rhs.y_ + m02_ * rhs.z_ + m03_) * invW,
            (m10_ * rhs.x_ + m11_ * rhs.y_ + m12_ * rhs.z_ + m13_) * invW,
            (m20_ * rhs.x_ + m21_ * rhs.y_ + m22_ * rhs.z_ + m23_) * invW
        );
#endif
    }

    /// Multiply a Vector4.
    Vector4 operator *(const Vector4& rhs) const
    {
#ifdef USE_SSE
        __m128 vec = _mm_loadu_ps(&rhs.x_);
        __m128 r0 = _mm_mul_ps(_mm_loadu_ps(&m00_), vec);
        __m128 r1 = _mm_mul_ps(_mm_loadu_ps(&m10_), vec);
        __m128 t0 = _mm_unpacklo_ps(r0, r1);
        __m128 t1 = _mm_unpackhi_ps(r0, r1);
        t0 = _mm_add_ps(t0, t1);
        __m128 r2 = _mm_mul_ps(_mm_loadu_ps(&m20_), vec);
        __m128 r3 = _mm_mul_ps(_mm_loadu_ps(&m30_), vec);
        __m128 t2 = _mm_unpacklo_ps(r2, r3);
        __m128 t3 = _mm_unpackhi_ps(r2, r3);
        t2 = _mm_add_ps(t2, t3);
        vec = _mm_add_ps(_mm_movelh_ps(t0, t2), _mm_movehl_ps(t2, t0));

        Vector4 ret;
        _mm_storeu_ps(&ret.x_, vec);
        return ret;
#else
        return Vector4(
            m00_ * rhs.x_ + m01_ * rhs.y_ + m02_ * rhs.z_ + m03_ * rhs.w_,
            m10_ * rhs.x_ + m11_ * rhs.y_ + m12_ * rhs.z_ + m13_ * rhs.w_,
            m20_ * rhs.x_ + m21_ * rhs.y_ + m22_ * rhs.z_ + m23_ * rhs.w_,
            m30_ * rhs.x_ + m31_ * rhs.y_ + m32_ * rhs.z_ + m33_ * rhs.w_
        );
#endif
    }

    /// Add a matrix.
    Matrix4 operator +(const Matrix4& rhs) const
    {
#ifdef USE_SSE
        Matrix4 ret;
        _mm_storeu_ps(&ret.m00_, _mm_add_ps(_mm_loadu_ps(&m00_), _mm_loadu_ps(&rhs.m00_)));
        _mm_storeu_ps(&ret.m10_, _mm_add_ps(_mm_loadu_ps(&m10_), _mm_loadu_ps(&rhs.m10_)));
        _mm_storeu_ps(&ret.m20_, _mm_add_ps(_mm_loadu_ps(&m20_), _mm_loadu_ps(&rhs.m20_)));
        _mm_storeu_ps(&ret.m30_, _mm_add_ps(_mm_loadu_ps(&m30_), _mm_loadu_ps(&rhs.m30_)));
        return ret;
#else
        return Matrix4(
            m00_ + rhs.m00_,
            m01_ + rhs.m01_,
            m02_ + rhs.m02_,
            m03_ + rhs.m03_,
            m10_ + rhs.m10_,
            m11_ + rhs.m11_,
            m12_ + rhs.m12_,
            m13_ + rhs.m13_,
            m20_ + rhs.m20_,
            m21_ + rhs.m21_,
            m22_ + rhs.m22_,
            m23_ + rhs.m23_,
            m30_ + rhs.m30_,
            m31_ + rhs.m31_,
            m32_ + rhs.m32_,
            m33_ + rhs.m33_
        );
#endif
    }

    /// Subtract a matrix.
    Matrix4 operator -(const Matrix4& rhs) const
    {
#ifdef USE_SSE
        Matrix4 ret;
        _mm_storeu_ps(&ret.m00_, _mm_sub_ps(_mm_loadu_ps(&m00_), _mm_loadu_ps(&rhs.m00_)));
        _mm_storeu_ps(&ret.m10_, _mm_sub_ps(_mm_loadu_ps(&m10_), _mm_loadu_ps(&rhs.m10_)));
        _mm_storeu_ps(&ret.m20_, _mm_sub_ps(_mm_loadu_ps(&m20_), _mm_loadu_ps(&rhs.m20_)));
        _mm_storeu_ps(&ret.m30_, _mm_sub_ps(_mm_loadu_ps(&m30_), _mm_loadu_ps(&rhs.m30_)));
        return ret;
#else
        return Matrix4(
            m00_ - rhs.m00_,
            m01_ - rhs.m01_,
            m02_ - rhs.m02_,
            m03_ - rhs.m03_,
            m10_ - rhs.m10_,
            m11_ - rhs.m11_,
            m12_ - rhs.m12_,
            m13_ - rhs.m13_,
            m20_ - rhs.m20_,
            m21_ - rhs.m21_,
            m22_ - rhs.m22_,
            m23_ - rhs.m23_,
            m30_ - rhs.m30_,
            m31_ - rhs.m31_,
            m32_ - rhs.m32_,
            m33_ - rhs.m33_
        );
#endif
    }

    /// Multiply with a scalar.
    Matrix4 operator *(float rhs) const
    {
#ifdef USE_SSE
        Matrix4 ret;
        const __m128 mul = _mm_set1_ps(rhs);
        _mm_storeu_ps(&ret.m00_, _mm_mul_ps(_mm_loadu_ps(&m00_), mul));
        _mm_storeu_ps(&ret.m10_, _mm_mul_ps(_mm_loadu_ps(&m10_), mul));
        _mm_storeu_ps(&ret.m20_, _mm_mul_ps(_mm_loadu_ps(&m20_), mul));
        _mm_storeu_ps(&ret.m30_, _mm_mul_ps(_mm_loadu_ps(&m30_), mul));
        return ret;
#else
        return Matrix4(
            m00_ * rhs,
            m01_ * rhs,
            m02_ * rhs,
            m03_ * rhs,
            m10_ * rhs,
            m11_ * rhs,
            m12_ * rhs,
            m13_ * rhs,
            m20_ * rhs,
            m21_ * rhs,
            m22_ * rhs,
            m23_ * rhs,
            m30_ * rhs,
            m31_ * rhs,
            m32_ * rhs,
            m33_ * rhs
        );
#endif
    }

    /// Multiply with a scalar.
    Matrix4 operator *(int rhs) const
    {
        return Matrix4(
                m00_ * rhs,
                m01_ * rhs,
                m02_ * rhs,
                m03_ * rhs,
                m10_ * rhs,
                m11_ * rhs,
                m12_ * rhs,
                m13_ * rhs,
                m20_ * rhs,
                m21_ * rhs,
                m22_ * rhs,
                m23_ * rhs,
                m30_ * rhs,
                m31_ * rhs,
                m32_ * rhs,
                m33_ * rhs
        );
    }

    Vector3 operator +(const Vector3 &v) const {
        Vector3 ret;
        ret.x_ = m00_ * v.x_ + m01_ * v.y_ + m02_ * v.z_ + m03_;
        ret.y_ = m10_ * v.x_ + m11_ * v.y_ + m12_ * v.z_ + m13_;
        ret.z_ = m20_ * v.x_ + m21_ * v.y_ + m22_ * v.z_ + m23_;
        return ret;
    }

    /// Multiply a matrix.
    Matrix4 operator *(const Matrix4& rhs) const
    {
#ifdef USE_SSE
        Matrix4 out;

        __m128 r0 = _mm_loadu_ps(&rhs.m00_);
        __m128 r1 = _mm_loadu_ps(&rhs.m10_);
        __m128 r2 = _mm_loadu_ps(&rhs.m20_);
        __m128 r3 = _mm_loadu_ps(&rhs.m30_);

        __m128 l = _mm_loadu_ps(&m00_);
        __m128 t0 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(0, 0, 0, 0)), r0);
        __m128 t1 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(1, 1, 1, 1)), r1);
        __m128 t2 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(2, 2, 2, 2)), r2);
        __m128 t3 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(3, 3, 3, 3)), r3);
        _mm_storeu_ps(&out.m00_, _mm_add_ps(_mm_add_ps(t0, t1), _mm_add_ps(t2, t3)));

        l = _mm_loadu_ps(&m10_);
        t0 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(0, 0, 0, 0)), r0);
        t1 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(1, 1, 1, 1)), r1);
        t2 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(2, 2, 2, 2)), r2);
        t3 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(3, 3, 3, 3)), r3);
        _mm_storeu_ps(&out.m10_, _mm_add_ps(_mm_add_ps(t0, t1), _mm_add_ps(t2, t3)));

        l = _mm_loadu_ps(&m20_);
        t0 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(0, 0, 0, 0)), r0);
        t1 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(1, 1, 1, 1)), r1);
        t2 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(2, 2, 2, 2)), r2);
        t3 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(3, 3, 3, 3)), r3);
        _mm_storeu_ps(&out.m20_, _mm_add_ps(_mm_add_ps(t0, t1), _mm_add_ps(t2, t3)));

        l = _mm_loadu_ps(&m30_);
        t0 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(0, 0, 0, 0)), r0);
        t1 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(1, 1, 1, 1)), r1);
        t2 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(2, 2, 2, 2)), r2);
        t3 = _mm_mul_ps(_mm_shuffle_ps(l, l, _MM_SHUFFLE(3, 3, 3, 3)), r3);
        _mm_storeu_ps(&out.m30_, _mm_add_ps(_mm_add_ps(t0, t1), _mm_add_ps(t2, t3)));

        return out;
#else
        return Matrix4(
            m00_ * rhs.m00_ + m01_ * rhs.m10_ + m02_ * rhs.m20_ + m03_ * rhs.m30_,
            m00_ * rhs.m01_ + m01_ * rhs.m11_ + m02_ * rhs.m21_ + m03_ * rhs.m31_,
            m00_ * rhs.m02_ + m01_ * rhs.m12_ + m02_ * rhs.m22_ + m03_ * rhs.m32_,
            m00_ * rhs.m03_ + m01_ * rhs.m13_ + m02_ * rhs.m23_ + m03_ * rhs.m33_,
            m10_ * rhs.m00_ + m11_ * rhs.m10_ + m12_ * rhs.m20_ + m13_ * rhs.m30_,
            m10_ * rhs.m01_ + m11_ * rhs.m11_ + m12_ * rhs.m21_ + m13_ * rhs.m31_,
            m10_ * rhs.m02_ + m11_ * rhs.m12_ + m12_ * rhs.m22_ + m13_ * rhs.m32_,
            m10_ * rhs.m03_ + m11_ * rhs.m13_ + m12_ * rhs.m23_ + m13_ * rhs.m33_,
            m20_ * rhs.m00_ + m21_ * rhs.m10_ + m22_ * rhs.m20_ + m23_ * rhs.m30_,
            m20_ * rhs.m01_ + m21_ * rhs.m11_ + m22_ * rhs.m21_ + m23_ * rhs.m31_,
            m20_ * rhs.m02_ + m21_ * rhs.m12_ + m22_ * rhs.m22_ + m23_ * rhs.m32_,
            m20_ * rhs.m03_ + m21_ * rhs.m13_ + m22_ * rhs.m23_ + m23_ * rhs.m33_,
            m30_ * rhs.m00_ + m31_ * rhs.m10_ + m32_ * rhs.m20_ + m33_ * rhs.m30_,
            m30_ * rhs.m01_ + m31_ * rhs.m11_ + m32_ * rhs.m21_ + m33_ * rhs.m31_,
            m30_ * rhs.m02_ + m31_ * rhs.m12_ + m32_ * rhs.m22_ + m33_ * rhs.m32_,
            m30_ * rhs.m03_ + m31_ * rhs.m13_ + m32_ * rhs.m23_ + m33_ * rhs.m33_
        );
#endif
    }

    /// Multiply with a 3x4 matrix.
    Matrix4 operator *(const Matrix3x4& rhs) const;

    /// Set translation elements.
    void SetTranslation(const Vector3& translation)
    {
        m03_ = translation.x_;
        m13_ = translation.y_;
        m23_ = translation.z_;
    }

    /// Set rotation elements from a 3x3 matrix.
    void SetRotation(const Matrix3& rotation)
    {
        m00_ = rotation.m00_;
        m01_ = rotation.m01_;
        m02_ = rotation.m02_;
        m10_ = rotation.m10_;
        m11_ = rotation.m11_;
        m12_ = rotation.m12_;
        m20_ = rotation.m20_;
        m21_ = rotation.m21_;
        m22_ = rotation.m22_;
    }

    /// Set scaling elements.
    void SetScale(const Vector3& scale)
    {
        m00_ = scale.x_;
        m11_ = scale.y_;
        m22_ = scale.z_;
    }

    /// Set uniform scaling elements.
    void SetScale(float scale)
    {
        m00_ = scale;
        m11_ = scale;
        m22_ = scale;
    }

    /// Return the combined rotation and scaling matrix.
    Matrix3 ToMatrix3() const
    {
        return Matrix3(
            m00_,
            m01_,
            m02_,
            m10_,
            m11_,
            m12_,
            m20_,
            m21_,
            m22_
        );
    }

    /// Return the rotation matrix with scaling removed.
    Matrix3 RotationMatrix() const
    {
        Vector3 invScale(
            1.0f / sqrtf(m00_ * m00_ + m10_ * m10_ + m20_ * m20_),
            1.0f / sqrtf(m01_ * m01_ + m11_ * m11_ + m21_ * m21_),
            1.0f / sqrtf(m02_ * m02_ + m12_ * m12_ + m22_ * m22_)
        );

        return ToMatrix3().Scaled(invScale);
    }

    /// Return the translation part.
    Vector3 Translation() const
    {
        return Vector3(
            m03_,
            m13_,
            m23_
        );
    }

    /// Return the rotation part.
    Quaternion Rotation() const { return Quaternion(RotationMatrix()); }

    /// Return the scaling part.
    Vector3 Scale() const
    {
        return Vector3(
            sqrtf(m00_ * m00_ + m10_ * m10_ + m20_ * m20_),
            sqrtf(m01_ * m01_ + m11_ * m11_ + m21_ * m21_),
            sqrtf(m02_ * m02_ + m12_ * m12_ + m22_ * m22_)
        );
    }

    /// Return the scaling part with the sign. Reference rotation matrix is required to avoid ambiguity.
    Vector3 SignedScale(const Matrix3& rotation) const
    {
        return Vector3(
            rotation.m00_ * m00_ + rotation.m10_ * m10_ + rotation.m20_ * m20_,
            rotation.m01_ * m01_ + rotation.m11_ * m11_ + rotation.m21_ * m21_,
            rotation.m02_ * m02_ + rotation.m12_ * m12_ + rotation.m22_ * m22_
        );
    }

    /// Return transposed.
    Matrix4 Transpose() const
    {
#ifdef USE_SSE
        __m128 m0 = _mm_loadu_ps(&m00_);
        __m128 m1 = _mm_loadu_ps(&m10_);
        __m128 m2 = _mm_loadu_ps(&m20_);
        __m128 m3 = _mm_loadu_ps(&m30_);
        _MM_TRANSPOSE4_PS(m0, m1, m2, m3);      // NOLINT(modernize-use-bool-literals)
        Matrix4 out;
        _mm_storeu_ps(&out.m00_, m0);
        _mm_storeu_ps(&out.m10_, m1);
        _mm_storeu_ps(&out.m20_, m2);
        _mm_storeu_ps(&out.m30_, m3);
        return out;
#else
        return Matrix4(
            m00_,
            m10_,
            m20_,
            m30_,
            m01_,
            m11_,
            m21_,
            m31_,
            m02_,
            m12_,
            m22_,
            m32_,
            m03_,
            m13_,
            m23_,
            m33_
        );
#endif
    }

    /// Test for equality with another matrix with epsilon.
    bool Equals(const Matrix4& rhs) const
    {
        const float* leftData = Data();
        const float* rightData = rhs.Data();

        for (unsigned i = 0; i < 16; ++i)
        {
            if (!Se::Equals(leftData[i], rightData[i]))
                return false;
        }

        return true;
    }

    /// Return decomposition to translation, rotation and scale.
    void Decompose(Vector3& translation, Quaternion& rotation, Vector3& scale) const
    {
        translation.x_ = m03_;
        translation.y_ = m13_;
        translation.z_ = m23_;

        scale.x_ = sqrtf(m00_ * m00_ + m10_ * m10_ + m20_ * m20_);
        scale.y_ = sqrtf(m01_ * m01_ + m11_ * m11_ + m21_ * m21_);
        scale.z_ = sqrtf(m02_ * m02_ + m12_ * m12_ + m22_ * m22_);

        Vector3 invScale(1.0f / scale.x_, 1.0f / scale.y_, 1.0f / scale.z_);
        rotation = Quaternion(ToMatrix3().Scaled(invScale));
    }

    /// Return inverse.
    Matrix4 Inverse() const
    {
        float v0 = m20_ * m31_ - m21_ * m30_;
        float v1 = m20_ * m32_ - m22_ * m30_;
        float v2 = m20_ * m33_ - m23_ * m30_;
        float v3 = m21_ * m32_ - m22_ * m31_;
        float v4 = m21_ * m33_ - m23_ * m31_;
        float v5 = m22_ * m33_ - m23_ * m32_;

        float i00 = (v5 * m11_ - v4 * m12_ + v3 * m13_);
        float i10 = -(v5 * m10_ - v2 * m12_ + v1 * m13_);
        float i20 = (v4 * m10_ - v2 * m11_ + v0 * m13_);
        float i30 = -(v3 * m10_ - v1 * m11_ + v0 * m12_);

        float invDet = 1.0f / (i00 * m00_ + i10 * m01_ + i20 * m02_ + i30 * m03_);

        i00 *= invDet;
        i10 *= invDet;
        i20 *= invDet;
        i30 *= invDet;

        float i01 = -(v5 * m01_ - v4 * m02_ + v3 * m03_) * invDet;
        float i11 = (v5 * m00_ - v2 * m02_ + v1 * m03_) * invDet;
        float i21 = -(v4 * m00_ - v2 * m01_ + v0 * m03_) * invDet;
        float i31 = (v3 * m00_ - v1 * m01_ + v0 * m02_) * invDet;

        v0 = m10_ * m31_ - m11_ * m30_;
        v1 = m10_ * m32_ - m12_ * m30_;
        v2 = m10_ * m33_ - m13_ * m30_;
        v3 = m11_ * m32_ - m12_ * m31_;
        v4 = m11_ * m33_ - m13_ * m31_;
        v5 = m12_ * m33_ - m13_ * m32_;

        float i02 = (v5 * m01_ - v4 * m02_ + v3 * m03_) * invDet;
        float i12 = -(v5 * m00_ - v2 * m02_ + v1 * m03_) * invDet;
        float i22 = (v4 * m00_ - v2 * m01_ + v0 * m03_) * invDet;
        float i32 = -(v3 * m00_ - v1 * m01_ + v0 * m02_) * invDet;

        v0 = m21_ * m10_ - m20_ * m11_;
        v1 = m22_ * m10_ - m20_ * m12_;
        v2 = m23_ * m10_ - m20_ * m13_;
        v3 = m22_ * m11_ - m21_ * m12_;
        v4 = m23_ * m11_ - m21_ * m13_;
        v5 = m23_ * m12_ - m22_ * m13_;

        float i03 = -(v5 * m01_ - v4 * m02_ + v3 * m03_) * invDet;
        float i13 = (v5 * m00_ - v2 * m02_ + v1 * m03_) * invDet;
        float i23 = -(v4 * m00_ - v2 * m01_ + v0 * m03_) * invDet;
        float i33 = (v3 * m00_ - v1 * m01_ + v0 * m02_) * invDet;

        return Matrix4(
            i00, i01, i02, i03,
            i10, i11, i12, i13,
            i20, i21, i22, i23,
            i30, i31, i32, i33);
    }

    /// Return float data.
    const float* Data() const { return &m00_; }

    /// Return matrix element.
    float Element(unsigned i, unsigned j) const { return Data()[i * 4 + j]; }

    /// Return matrix element.
    void SetElement(unsigned i, unsigned j, float value) {
        data[i * 4 + j] = value; }

    /// Return matrix row.
    Vector4 Row(unsigned i) const { return Vector4(Element(i, 0), Element(i, 1), Element(i, 2), Element(i, 3)); }

    void SetRow(unsigned i, const Vector4& row) {
        SetElement(i, 0, row.x_);
        SetElement(i, 1, row.y_);
        SetElement(i, 2, row.z_);
        SetElement(i, 3, row.w_); }

    /// Return matrix column.
    Vector4 Column(unsigned j) const { return Vector4(Element(0, j), Element(1, j), Element(2, j), Element(3, j)); }

    Vector3 Projection(const Vector3 &v) const 
    {
            Vector3 ret;
#ifdef USE_SSE
        __m128 res_0 = _mm_mul_ps(m.col0,_MM_SWIZZLE(v.vec,X,X,X,X));
        __m128 res_1 = _mm_mul_ps(m.col1,_MM_SWIZZLE(v.vec,Y,Y,Y,Y));
        __m128 res_2 = _mm_mul_ps(m.col2,_MM_SWIZZLE(v.vec,Z,Z,Z,Z));
        __m128 res_3 = _mm_add_ps(_mm_add_ps(res_0,res_1),_mm_add_ps(res_2,m.col3));
        ret.vec = _mm_div_ps(res_3,_MM_SWIZZLE(res_3,W,W,W,W));
#elif USE_ALTIVEC
        vec_float4 res_0 = vec_madd(m.col0,VEC_SWIZZLE(v.vec,X,X,X,X),m.col3);
        vec_float4 res_1 = vec_madd(m.col1,VEC_SWIZZLE(v.vec,Y,Y,Y,Y),res_0);
        vec_float4 res_2 = vec_madd(m.col2,VEC_SWIZZLE(v.vec,Z,Z,Z,Z),res_1);
        ret.vec = vec_madd(res_2,vec_rcp_nr(VEC_SWIZZLE(res_2,W,W,W,W)),vec_splats(0.0f));
#else
        float iw = 1.0f/(m30_ * v.x_ + m31_ * v.y_ + m32_ * v.z_ + m33_);
        ret.x_ = (m00_ * v.x_ + m01_ * v.y_ + m02_ * v.z_ + m03_) * iw;
        ret.y_ = (m10_ * v.x_ + m11_ * v.y_ + m12_ * v.z_ + m13_) * iw;
        ret.z_ = (m20_ * v.x_ + m21_ * v.y_ + m22_ * v.z_ + m23_) * iw;
#endif
        return ret;
    }

    Vector4 Projection(const Vector4 &v) const
    {
        Vector4 ret;
#ifdef USE_SSE
        __m128 res_0 = _mm_mul_ps(m.col0,_MM_SWIZZLE(v.vec,X,X,X,X));
        __m128 res_1 = _mm_mul_ps(m.col1,_MM_SWIZZLE(v.vec,Y,Y,Y,Y));
        __m128 res_2 = _mm_mul_ps(m.col2,_MM_SWIZZLE(v.vec,Z,Z,Z,Z));
        __m128 res_3 = _mm_add_ps(_mm_add_ps(res_0,res_1),_mm_add_ps(res_2,m.col3));
        ret.vec = _mm_div_ps(res_3,_MM_SWIZZLE(res_3,W,W,W,W));
#elif USE_ALTIVEC
        vec_float4 res_0 = vec_madd(m.col0,VEC_SWIZZLE(v.vec,X,X,X,X),m.col3);
        vec_float4 res_1 = vec_madd(m.col1,VEC_SWIZZLE(v.vec,Y,Y,Y,Y),res_0);
        vec_float4 res_2 = vec_madd(m.col2,VEC_SWIZZLE(v.vec,Z,Z,Z,Z),res_1);
        ret.vec = vec_madd(res_2,vec_rcp_nr(VEC_SWIZZLE(res_2,W,W,W,W)),vec_splats(0.0f));
#else
        float iw = 1.0f/(m30_ * v.x_ + m31_ * v.y_ + m32_ * v.z_ + m33_ * v.w_);
        ret.x_ = (m00_ * v.x_ + m01_ * v.y_ + m02_ * v.z_ + m03_ * v.w_) * iw;
        ret.y_ = (m10_ * v.x_ + m11_ * v.y_ + m12_ * v.z_ + m13_ * v.w_) * iw;
        ret.z_ = (m20_ * v.x_ + m21_ * v.y_ + m22_ * v.z_ + m23_ * v.w_) * iw;
#endif
        return ret;
    }

    /// Return as string.
    String ToString() const {
    return cformat("%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g", 
            m00_, m01_, m02_, m03_, m10_, m11_, m12_, m13_, m20_,
            m21_, m22_, m23_, m30_, m31_, m32_, m33_);
    }

    unsigned ToHash() const {
        unsigned hash = 37;
        for (int i = 0; i < 16; i++)
            hash = 37 * hash + FloatToRawIntBits(data[i]);
        return hash;
    }

    union {
        struct {
            float m00_;
            float m01_;
            float m02_;
            float m03_;
            float m10_;
            float m11_;
            float m12_;
            float m13_;
            float m20_;
            float m21_;
            float m22_;
            float m23_;
            float m30_;
            float m31_;
            float m32_;
            float m33_;
        };
        float data[16];
    };

    /// Bulk transpose matrices.
    static void BulkTranspose(float* dest, const float* src, unsigned count)
    {
        for (unsigned i = 0; i < count; ++i)
        {
#ifdef USE_SSE
            __m128 m0 = _mm_loadu_ps(src);
            __m128 m1 = _mm_loadu_ps(src + 4);
            __m128 m2 = _mm_loadu_ps(src + 8);
            __m128 m3 = _mm_loadu_ps(src + 12);
            _MM_TRANSPOSE4_PS(m0, m1, m2, m3);      // NOLINT(modernize-use-bool-literals)
            _mm_storeu_ps(dest, m0);
            _mm_storeu_ps(dest + 4, m1);
            _mm_storeu_ps(dest + 8, m2);
            _mm_storeu_ps(dest + 12, m3);
#else
            dest[0] = src[0];
            dest[1] = src[4];
            dest[2] = src[8];
            dest[3] = src[12];
            dest[4] = src[1];
            dest[5] = src[5];
            dest[6] = src[9];
            dest[7] = src[13];
            dest[8] = src[2];
            dest[9] = src[6];
            dest[10] = src[10];
            dest[11] = src[14];
            dest[12] = src[3];
            dest[13] = src[7];
            dest[14] = src[11];
            dest[15] = src[15];
#endif
            dest += 16;
            src += 16;
        }
    }

    /// Zero matrix.
    static const Matrix4 ZERO;
    /// Identity matrix.
    static const Matrix4 IDENTITY;
};

inline const Matrix4 Matrix4::ZERO(
    0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f);

inline const Matrix4 Matrix4::IDENTITY;

/// Multiply a 4x4 matrix with a scalar.
inline Matrix4 operator *(float lhs, const Matrix4& rhs) { return rhs * lhs; }

inline bool Compare(const Matrix4 &v0,const Matrix4 &v1) {
    return (v0 == v1);
}

inline bool Compare(const Matrix4 &v0,const Matrix4 &v1, float epsilon) {
    return (v0 == v1);
}

inline Vector4 operator*(const Vector4 &v,const Matrix4 &m)
{
    Vector4 ret;
    ret.x_ = m.m00_ * v.x_ + m.m10_ * v.y_ + m.m20_ * v.z_ + m.m30_ * v.w_;
    ret.y_ = m.m01_ * v.x_ + m.m11_ * v.y_ + m.m21_ * v.z_ + m.m31_ * v.w_;
    ret.z_ = m.m02_ * v.x_ + m.m12_ * v.y_ + m.m22_ * v.z_ + m.m32_ * v.w_;
    ret.w_ = m.m03_ * v.x_ + m.m13_ * v.y_ + m.m23_ * v.z_ + m.m33_ * v.w_;
    return ret;
}

inline Matrix4 &ComposeTransform(Matrix4 &ret,const Vector4 &xyz,const Quaternion &rot)
{
    float x2 = (rot.x_ + rot.x_) * xyz.w_;
    float y2 = (rot.y_ + rot.y_) * xyz.w_;
    float z2 = (rot.z_ + rot.z_) * xyz.w_;
    float xx2 = rot.x_ * x2;
    float yy2 = rot.y_ * y2;
    float zz2 = rot.z_ * z2;
    float zx2 = rot.z_ * x2;
    float xy2 = rot.x_ * y2;
    float yz2 = rot.y_ * z2;
    float wx2 = rot.w_ * x2;
    float wy2 = rot.w_ * y2;
    float wz2 = rot.w_ * z2;
    ret.m00_ = xyz.w_ - yy2 - zz2;
    ret.m10_ = xy2 + wz2;
    ret.m20_ = zx2 - wy2;
    ret.m30_ = 0.0f;
    ret.m01_ = xy2 - wz2;
    ret.m11_ = xyz.w_ - xx2 - zz2;
    ret.m21_ = yz2 + wx2;
    ret.m31_ = 0.0f;
    ret.m02_ = zx2 + wy2;
    ret.m12_ = yz2 - wx2;
    ret.m22_ = xyz.w_ - xx2 - yy2;
    ret.m32_ = 0.0f;
    ret.m03_ = xyz.x_;
    ret.m13_ = xyz.y_;
    ret.m23_ = xyz.z_;
    ret.m33_ = 1.0f;
    return ret;
}

Matrix4 &ComposeTransform(Matrix4 &ret, const Vector3 &xyz, const Quaternion &rot, const Vector3 &s)
{
    Matrix3 rotation,scale;
    scale.SetDiagonal(s);
    rotation = rot.GetMat3() * scale;
    ret = Matrix4(rotation, xyz);
    return ret;
}


inline void DecomposeTransform(const Matrix4 &m, Vector4 &xyz, Quaternion &rot)
{
    Matrix3 rotate,scale;
    Matrix3 rotation = m.ToMatrix3();

    Matrix3::Orthonormalize(rotate,rotation);
    scale = rotate.Transpose() * rotation;
    xyz.x_ = m.m03_;
    xyz.y_ = m.m13_;
    xyz.z_ = m.m23_;
    xyz.w_ = (scale.m00_ + scale.m11_ + scale.m22_) * (1.0f / 3.0f);
    rot = Quaternion(rotate);
}

inline void DecomposeTransform(const Matrix4 &m,Quaternion &q0,Quaternion &q1)
{
    Matrix3 rotate;
    q0 = Quaternion(Matrix3::Orthonormalize(rotate, m.ToMatrix3()));
    q1.x_ = ( m.m03_ * q0.w_ + m.m13_ * q0.z_ - m.m23_ * q0.y_) * 0.5f;
    q1.y_ = (-m.m03_ * q0.z_ + m.m13_ * q0.w_ + m.m23_ * q0.x_) * 0.5f;
    q1.z_ = ( m.m03_ * q0.y_ - m.m13_ * q0.x_ + m.m23_ * q0.w_) * 0.5f;
    q1.w_ = ( m.m03_ * q0.x_ + m.m13_ * q0.y_ + m.m23_ * q0.z_) * 0.5f;
    q0 = q0 * Sqrt(m.m00_ * m.m00_ + m.m01_ * m.m01_ + m.m02_ * m.m02_);
}

inline void DecomposeTransform(const Matrix4 &m, Vector3 &xyz, Quaternion &rot, Vector3 &s)
{
    Matrix3 rotate,scale;
    Matrix3 rotation = m.ToMatrix3();
    Matrix3::Orthonormalize(rotate, rotation);
    scale = rotate.Transpose()*rotation;
    xyz.x_ = m.m03_;
    xyz.y_ = m.m13_;
    xyz.z_ = m.m23_;
    rot = Quaternion(rotate); //.getQuat();
    s.x_ = scale.m00_;
    s.y_ = scale.m11_;
    s.z_ = scale.m22_;
}

inline Matrix4 Translate(const Vector3 &v) {
    Matrix4 ret;
    ret.SetTranslation(v);
    return ret;
}

inline Matrix4 &Rotation(Matrix4 &ret,const Matrix4 &m) {
    ret.m00_ = m.m00_; ret.m01_ = m.m01_; ret.m02_ = m.m02_; ret.m03_ = 0.0f;
    ret.m10_ = m.m10_; ret.m11_ = m.m11_; ret.m12_ = m.m12_; ret.m13_ = 0.0f;
    ret.m20_ = m.m20_; ret.m21_ = m.m21_; ret.m22_ = m.m22_; ret.m23_ = 0.0f;
    ret.m30_ = 0.0f;   ret.m31_ = 0.0f;   ret.m32_ = 0.0f;   ret.m33_ = 1.0f;
    return ret;
}

inline Matrix4 Rotation(const Matrix4 &m) {
    Matrix4 ret;
    return Rotation(ret,m);
}

inline void Quaternion::RotationMatrix(const Matrix4& matrix, Quaternion& result)
{
    float sqrtV;
    float half;
    const float scale = matrix.m00_ + matrix.m11_ + matrix.m22_;

    if (scale > 0.0f)
    {
        sqrtV = Sqrt(scale + 1.0f);
        result.w_ = sqrtV * 0.5f;
        sqrtV = 0.5f / sqrtV;

        result.x_ = (matrix.m12_ - matrix.m21_) * sqrtV;
        result.y_ = (matrix.m20_ - matrix.m02_) * sqrtV;
        result.z_ = (matrix.m01_ - matrix.m10_) * sqrtV;
    }
    else if (matrix.m00_ >= matrix.m11_ && matrix.m00_ >= matrix.m22_)
    {
        sqrtV = Sqrt(1.0f + matrix.m00_ - matrix.m11_ - matrix.m22_);
        half = 0.5f / sqrtV;

        result = Quaternion(
                0.5f * sqrtV,
                (matrix.m01_ + matrix.m10_) * half,
                (matrix.m02_ + matrix.m20_) * half,
                (matrix.m12_ - matrix.m21_) * half);
    }
    else if (matrix.m11_ > matrix.m22_)
    {
        sqrtV = Sqrt(1.0f + matrix.m11_ - matrix.m00_ - matrix.m22_);
        half = 0.5f / sqrtV;

        result = Quaternion(
                (matrix.m10_ + matrix.m01_) * half,
                0.5f * sqrtV,
                (matrix.m21_ + matrix.m12_) * half,
                (matrix.m20_ - matrix.m02_) * half);
    }
    else
    {
        sqrtV = Sqrt(1.0f + matrix.m22_ - matrix.m00_ - matrix.m11_);
        half = 0.5f / sqrtV;

        result = Quaternion(
                (matrix.m20_ + matrix.m02_) * half,
                (matrix.m21_ + matrix.m12_) * half,
                0.5f * sqrtV,
                (matrix.m01_ - matrix.m10_) * half);
    }

    result.Normalize();
}

}
