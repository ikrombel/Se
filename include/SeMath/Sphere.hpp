#pragma once

#include <SeMath/MathDefs.hpp>
#include <SeMath/Vector3.hpp>

namespace Se
{

// class BoundingBox;
// class Polyhedron;
// class Frustum;

/// Circle in three-dimensional space.
class Circle
{
public:
    /// Return whether the circle is valid.
    bool IsValid() const { return radius_ >= 0.0f; }
    /// Return point on the sphere closest to given direction.
    Vector3 GetPoint(const Vector3& directionHint) const
    {
        if (!IsValid())
            return center_;

        const Vector3 direction = directionHint.Orthogonalize(normal_);
        return center_ + direction * radius_;
    }

    /// Center of the circle.
    Vector3 center_{};
    /// Normal of the plane containing the circle.
    Vector3 normal_{};
    /// Radius of the circle. Negative is invalid.
    float radius_{ -M_INFINITY };
};

/// %Sphere in three-dimensional space.
class Sphere
{
public:
    /// Construct undefined.
    Sphere() noexcept :
        center_(Vector3::ZERO),
        radius_(-M_INFINITY)
    {
    }

    /// Copy-construct from another sphere.
    Sphere(const Sphere& sphere) noexcept = default;

    /// Construct from center and radius.
    Sphere(const Vector3& center, float radius) noexcept :
        center_(center),
        radius_(radius)
    {
    }

    /// Construct from an array of vertices.
    Sphere(const Vector3* vertices, unsigned count) noexcept
    {
        Define(vertices, count);
    }

    // /// Construct from a bounding box.
    // explicit Sphere(const BoundingBox& box) noexcept
    // {
    //     Define(box);
    // }

    // /// Construct from a frustum.
    // explicit Sphere(const Frustum& frustum) noexcept
    // {
    //     Define(frustum);
    // }

    // /// Construct from a polyhedron.
    // explicit Sphere(const Polyhedron& poly) noexcept
    // {
    //     Define(poly);
    // }

    /// Assign from another sphere.
    Sphere& operator =(const Sphere& rhs) noexcept = default;

    /// Test for equality with another sphere.
    bool operator ==(const Sphere& rhs) const { return center_ == rhs.center_ && radius_ == rhs.radius_; }

    /// Test for inequality with another sphere.
    bool operator !=(const Sphere& rhs) const { return center_ != rhs.center_ || radius_ != rhs.radius_; }

    /// Define from another sphere.
    void Define(const Sphere& sphere)
    {
        Define(sphere.center_, sphere.radius_);
    }

    /// Define from center and radius.
    void Define(const Vector3& center, float radius)
    {
        center_ = center;
        radius_ = radius;
    }

    template<typename T> void Define(const T& obj) {
        SE_LOG_ERROR("Define not implemented to this object"); }
    template<typename T> void Merge(const T& obj) {
        SE_LOG_ERROR("Merge not implemented to this object"); }

    template<typename T> Intersection IsInside(const T& box) const {
        SE_LOG_ERROR("IsInside not implemented to this object");
        return Intersection::OUTSIDE; }
    template<typename T> Intersection IsInsideFast(const T& box) const {
        SE_LOG_ERROR("IsInsideFast not implemented to this object");
        return Intersection::OUTSIDE; }

    /// Define from an array of vertices.
    void Define(const Vector3* vertices, unsigned count) {
        if (!count)
            return;

        Clear();
        Merge(vertices, count);
    }


    

    /// Merge an array of vertices.
    void Merge(const Vector3* vertices, unsigned count);

    // /// Merge a bounding box.
    // void Merge(const BoundingBox& box);
    // /// Merge a frustum.
    // void Merge(const Frustum& frustum);
    // /// Merge a polyhedron.
    // void Merge(const Polyhedron& poly);

    /// Clear to undefined state.
    void Clear()
    {
        center_ = Vector3::ZERO;
        radius_ = -M_INFINITY;
    }

    /// Return true if this sphere is defined via a previous call to Define() or Merge().
    bool Defined() const
    {
        return radius_ >= 0.0f;
    }

    

    // /// Test if a bounding box is inside, outside or intersects.
    // Intersection IsInside(const BoundingBox& box) const;
    // /// Test if a bounding box is (partially) inside or outside.
    // Intersection IsInsideFast(const BoundingBox& box) const;

    /// Intersect with another sphere.
    /// Return optional distance from the center of this sphere to intersection circle.
    /// If there's no intersection, return closest point to another sphere.
    Circle Intersect(const Sphere& sphere, float* distanceFromCenter = nullptr) const;

    /// Return distance of a point to the surface, or 0 if inside.
    float Distance(const Vector3& point) const { return Max((point - center_).Length() - radius_, 0.0f); }
    /// Return point on the sphere relative to sphere position.
    Vector3 GetLocalPoint(float theta, float phi) const
    {
        return Vector3(
            radius_ * Sin(theta) * Sin(phi),
            radius_ * Cos(phi),
            radius_ * Cos(theta) * Sin(phi)
        );
    }
    /// Return point on the sphere.
    Vector3 GetPoint(float theta, float phi) const { return center_ + GetLocalPoint(theta, phi); }

    /// Sphere center.
    Vector3 center_;
    /// Sphere radius.
    float radius_{};
};

inline Circle Sphere::Intersect(const Sphere& sphere, float* distanceFromCenter) const
{
    const Vector3 offset = sphere.center_ - center_;
    const float distance = offset.Length();

    // http://mathworld.wolfram.com/Sphere-SphereIntersection.html
    const float R = radius_;
    const float r = sphere.radius_;
    const float d = Min(R + r, distance);
    const float a2 = (-d + r - R) * (-d - r + R) * (-d + r + R) * (d + r + R);
    const float a = Sqrt(Max(0.0f, a2)) / (2 * d);

    const bool isOutside = distance > R + r;
    const bool isInside = a2 < 0.0f;

    const float distanceToCircle = Sqrt(R * R - a * a);
    if (distanceFromCenter)
        *distanceFromCenter = distanceToCircle;

    const Vector3 normal = offset / distance;
    const Vector3 center = center_ + distanceToCircle * normal;
    const float effectiveRadius = isInside || isOutside ? -M_INFINITY : a;
    return Circle{ center, normal, effectiveRadius };
}


/// Test if a point is inside.
template<>
Intersection Sphere::IsInside(const Vector3& point) const
{
    float distSquared = Vector3(point - center_).LengthSquared();
    if (distSquared < radius_ * radius_)
        return INSIDE;
    else
        return OUTSIDE;
}

/// Test if another sphere is inside, outside or intersects.
template<>
Intersection Sphere::IsInside(const Sphere& sphere) const
{
    float dist = (sphere.center_ - center_).Length();
    if (dist >= sphere.radius_ + radius_)
        return OUTSIDE;
    else if (dist + sphere.radius_ < radius_)
        return INSIDE;
    else
        return INTERSECTS;
}

/// Test if another sphere is (partially) inside or outside.
template<>
Intersection Sphere::IsInsideFast(const Sphere& sphere) const
{
    float distSquared = Vector3(sphere.center_ - center_).LengthSquared();
    float combined = sphere.radius_ + radius_;

    if (distSquared >= combined * combined)
        return OUTSIDE;
    else
        return INSIDE;
}

template<>
void Sphere::Merge(const Sphere& sphere)
{
    if (radius_ < 0.0f)
    {
        center_ = sphere.center_;
        radius_ = sphere.radius_;
        return;
    }

    Vector3 offset = sphere.center_ - center_;
    float dist = offset.Length();

    // If sphere fits inside, do nothing
    if (dist + sphere.radius_ < radius_)
        return;

    // If we fit inside the other sphere, become it
    if (dist + radius_ < sphere.radius_)
    {
        center_ = sphere.center_;
        radius_ = sphere.radius_;
    }
    else
    {
        Vector3 NormalizedOffset = offset / dist;

        Vector3 min = center_ - radius_ * NormalizedOffset;
        Vector3 max = sphere.center_ + sphere.radius_ * NormalizedOffset;
        center_ = (min + max) * 0.5f;
        radius_ = (max - center_).Length();
    }
}

/// Merge a point.
template<>
inline void Sphere::Merge(const Vector3& point)
{
    if (radius_ < 0.0f)
    {
        center_ = point;
        radius_ = 0.0f;
        return;
    }

    Vector3 offset = point - center_;
    float dist = offset.Length();

    if (dist > radius_)
    {
        float half = (dist - radius_) * 0.5f;
        radius_ += half;
        center_ += (half / dist) * offset;
    }
}

inline void Sphere::Merge(const Vector3* vertices, unsigned count) {
    while (count--)
        Merge(*vertices++);
}

}
