#pragma once

#include <SeMath/Matrix3x4.hpp>
#include <SeMath/Vector3.hpp>
#include <SeMath/Quaternion.hpp>

namespace Se
{

/// Position, rotation and scale in 3D.
/// TODO: Expand it into something more user-friendly.
struct Transform
{
    Vector3 position_;
    Quaternion rotation_;
    Vector3 scale_{Vector3::ONE};


    static const Transform Identity;

    /// Construct Transform from Matrix3x4. It is not precise for non-uniform scale.
    static Transform FromMatrix3x4(const Matrix3x4& matrix)
    {
        Transform result;
        matrix.Decompose(result.position_, result.rotation_, result.scale_);
        return result;
    }

    /// Construct Matrix3x4 from Transform.
    Matrix3x4 ToMatrix3x4() const { return Matrix3x4(position_, rotation_, scale_); };

    /// Interpolate between two transforms.
    Transform Lerp(const Transform& rhs, float t) const
    {
        return Transform{
                position_.Lerp(rhs.position_, t), rotation_.Slerp(rhs.rotation_, t), scale_.Lerp(rhs.scale_, t)};
    }

    /// Return inverse transform. It is not precise for non-uniform scale.
    Transform Inverse() const
    {
        Transform ret;
        ret.rotation_ = rotation_.Inverse();
        ret.scale_ = Vector3::ONE / scale_;
        ret.position_ = -(ret.rotation_ * position_) * ret.scale_;
        return ret;
    }

    /// Return transform multiplied by another transform.
    Transform operator *(const Transform& rhs) const
    {
        Transform ret;
        ret.rotation_ = rotation_ * rhs.rotation_;
        ret.scale_ = scale_ * rhs.scale_;
        ret.position_ = position_ + rotation_ * (rhs.position_ * scale_);
        return ret;
    }

    /// Return position multiplied by the transform.
    Vector3 operator *(const Vector3& rhs) const
    {
        return position_ + rotation_ * (rhs * scale_);
    }

    /// Return rotation multiplied by the transform.
    Quaternion operator *(const Quaternion& rhs) const
    {
        return rotation_ * rhs;
    }
};

inline const Transform Transform::Identity{Vector3{}, Quaternion{}, Vector3{1.0f, 1.0f, 1.0f}};

}