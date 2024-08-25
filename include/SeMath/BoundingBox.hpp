#pragma once

#include <SeMath/Rect.hpp>
#include <SeMath/Matrix3x4.hpp>
#include <SeMath/Polyhedron.hpp>
#include <SeMath/BoundingSphere.hpp>

#ifdef SE_SSE
#include <xmmintrin.h>
#endif

namespace Se
{

/// Three-dimensional axis-aligned bounding box.
class BoundingBox
{
public:
    /// Construct with zero size.
    BoundingBox() noexcept :
        min_(M_INFINITY, M_INFINITY, M_INFINITY),
        max_(-M_INFINITY, -M_INFINITY, -M_INFINITY)
    {
    }

    /// Copy-construct from another bounding box.
    BoundingBox(const BoundingBox& box) noexcept :
        min_(box.min_),
        max_(box.max_)
    {
    }

    /// Construct from a rect, with the Z dimension left zero.
    explicit BoundingBox(const Rect& rect) noexcept :
        min_(Vector3(rect.min_, 0.0f)),
        max_(Vector3(rect.max_, 0.0f))
    {
    }

    /// Construct from minimum and maximum vectors.
    BoundingBox(const Vector3& min, const Vector3& max) noexcept :
        min_(min),
        max_(max)
    {
    }

    /// Construct from minimum and maximum floats (all dimensions same.)
    BoundingBox(float min, float max) noexcept :
        min_(Vector3(min, min, min)),
        max_(Vector3(max, max, max))
    {
    }

#ifdef SE_SSE
    BoundingBox(__m128 min, __m128 max) noexcept
    {
        _mm_storeu_ps(&min_.x_, min);
        _mm_storeu_ps(&max_.x_, max);
    }
#endif

    /// Construct from an array of vertices.
    BoundingBox(const Vector3* vertices, unsigned count) :
        min_(M_INFINITY, M_INFINITY, M_INFINITY),
        max_(-M_INFINITY, -M_INFINITY, -M_INFINITY)
    {
        Define(vertices, count);
    }

    /// Construct from a frustum.
    // explicit BoundingBox(const Frustum& frustum) :
    //     min_(M_INFINITY, M_INFINITY, M_INFINITY),
    //     max_(-M_INFINITY, -M_INFINITY, -M_INFINITY)
    // {
    //     Define(frustum);
    // }

    // /// Construct from a polyhedron.
    // explicit BoundingBox(const Polyhedron& poly) :
    //     min_(M_INFINITY, M_INFINITY, M_INFINITY),
    //     max_(-M_INFINITY, -M_INFINITY, -M_INFINITY)
    // {
    //     Define(poly);
    // }

    // /// Construct from a sphere.
    // explicit BoundingBox(const Sphere& sphere) :
    //     min_(M_INFINITY, M_INFINITY, M_INFINITY),
    //     max_(-M_INFINITY, -M_INFINITY, -M_INFINITY)
    // {
    //     Define(sphere);
    // }

    /// Assign from another bounding box.
    BoundingBox& operator =(const BoundingBox& rhs) noexcept
    {
        min_ = rhs.min_;
        max_ = rhs.max_;
        return *this;
    }

    /// Assign from a Rect, with the Z dimension left zero.
    BoundingBox& operator =(const Rect& rhs) noexcept
    {
        min_ = Vector3(rhs.min_, 0.0f);
        max_ = Vector3(rhs.max_, 0.0f);
        return *this;
    }

    /// Test for equality with another bounding box.
    bool operator ==(const BoundingBox& rhs) const { return (min_ == rhs.min_ && max_ == rhs.max_); }

    /// Test for inequality with another bounding box.
    bool operator !=(const BoundingBox& rhs) const { return (min_ != rhs.min_ || max_ != rhs.max_); }

    /// Define from another bounding box.
    void Define(const BoundingBox& box)
    {
        Define(box.min_, box.max_);
    }

    /// Define from a Rect.
    void Define(const Rect& rect)
    {
        Define(Vector3(rect.min_, 0.0f), Vector3(rect.max_, 0.0f));
    }

    /// Define from minimum and maximum vectors.
    void Define(const Vector3& min, const Vector3& max)
    {
        min_ = min;
        max_ = max;
    }

    /// Define from minimum and maximum floats (all dimensions same.)
    void Define(float min, float max)
    {
        min_ = Vector3(min, min, min);
        max_ = Vector3(max, max, max);
    }

    /// Define from a point.
    void Define(const Vector3& point)
    {
        min_ = max_ = point;
    }

    /// Merge a point.
    void Merge(const Vector3& point)
    {
#ifdef SE_SSE
        __m128 vec = _mm_set_ps(1.f, point.z_, point.y_, point.x_);
        _mm_storeu_ps(&min_.x_, _mm_min_ps(_mm_loadu_ps(&min_.x_), vec));
        _mm_storeu_ps(&max_.x_, _mm_max_ps(_mm_loadu_ps(&max_.x_), vec));
#else
        if (point.x_ < min_.x_)
            min_.x_ = point.x_;
        if (point.y_ < min_.y_)
            min_.y_ = point.y_;
        if (point.z_ < min_.z_)
            min_.z_ = point.z_;
        if (point.x_ > max_.x_)
            max_.x_ = point.x_;
        if (point.y_ > max_.y_)
            max_.y_ = point.y_;
        if (point.z_ > max_.z_)
            max_.z_ = point.z_;
#endif
    }

    /// Merge another bounding box.
    void Merge(const BoundingBox& box)
    {
#ifdef SE_SSE
        _mm_storeu_ps(&min_.x_, _mm_min_ps(_mm_loadu_ps(&min_.x_), _mm_loadu_ps(&box.min_.x_)));
        _mm_storeu_ps(&max_.x_, _mm_max_ps(_mm_loadu_ps(&max_.x_), _mm_loadu_ps(&box.max_.x_)));
#else
        if (box.min_.x_ < min_.x_)
            min_.x_ = box.min_.x_;
        if (box.min_.y_ < min_.y_)
            min_.y_ = box.min_.y_;
        if (box.min_.z_ < min_.z_)
            min_.z_ = box.min_.z_;
        if (box.max_.x_ > max_.x_)
            max_.x_ = box.max_.x_;
        if (box.max_.y_ > max_.y_)
            max_.y_ = box.max_.y_;
        if (box.max_.z_ > max_.z_)
            max_.z_ = box.max_.z_;
#endif
    }


    template<typename T> void Define(const T& obj) {
        SE_LOG_ERROR("Define not implemented to this object"); }
    template<typename T> void Merge(const T& obj) {
        SE_LOG_ERROR("Merge not implemented to this object"); }
    // template<typename T> void Define(const T& obj) {
    //     SE_LOG_ERROR("Define not implemented to this object"); }


    template<typename T> Intersection IsInside(const T& box) const {
        SE_LOG_ERROR("IsInside not implemented to this object"); }
    template<typename T> Intersection IsInsideFast(const T& box) const {
        SE_LOG_ERROR("IsInsideFast not implemented to this object"); }

    /// Define from an array of vertices.
    void Define(const Vector3* vertices, unsigned count) {
        Clear();

        if (!count)
            return;

        Merge(vertices, count);
    }
    // /// Define from a frustum.
    // void Define(const Frustum& frustum);
    // /// Define from a polyhedron.
    // void Define(const Polyhedron& poly);
    // /// Define from a sphere.
    // void Define(const Sphere& sphere);
    /// Merge an array of vertices.
    void Merge(const Vector3* vertices, unsigned count)
    {
        while (count--)
            Merge(*vertices++);
    }
    // /// Merge a frustum.
    // void Merge(const Frustum& frustum);
    // /// Merge a polyhedron.
    // void Merge(const Polyhedron& poly);
    // /// Merge a sphere.
    // void Merge(const Sphere& sphere);
    /// Clip with another bounding box. The box can become degenerate (undefined) as a result.
    void Clip(const BoundingBox& box)
    {
        if (box.min_.x_ > min_.x_)
            min_.x_ = box.min_.x_;
        if (box.max_.x_ < max_.x_)
            max_.x_ = box.max_.x_;
        if (box.min_.y_ > min_.y_)
            min_.y_ = box.min_.y_;
        if (box.max_.y_ < max_.y_)
            max_.y_ = box.max_.y_;
        if (box.min_.z_ > min_.z_)
            min_.z_ = box.min_.z_;
        if (box.max_.z_ < max_.z_)
            max_.z_ = box.max_.z_;

        if (min_.x_ > max_.x_ || min_.y_ > max_.y_ || min_.z_ > max_.z_)
        {
            min_ = Vector3(M_INFINITY, M_INFINITY, M_INFINITY);
            max_ = Vector3(-M_INFINITY, -M_INFINITY, -M_INFINITY);
        }
    }
    /// Transform with a 3x3 matrix.
    void Transform(const Matrix3& transform) {
        *this = Transformed(Matrix3x4(transform)); }
    /// Transform with a 3x4 matrix.
    void Transform(const Matrix3x4& transform) {
        *this = Transformed(transform); }

    /// Clear to undefined state.
    void Clear()
    {
#ifdef SE_SSE
        _mm_storeu_ps(&min_.x_, _mm_set1_ps(M_INFINITY));
        _mm_storeu_ps(&max_.x_, _mm_set1_ps(-M_INFINITY));
#else
        min_ = Vector3(M_INFINITY, M_INFINITY, M_INFINITY);
        max_ = Vector3(-M_INFINITY, -M_INFINITY, -M_INFINITY);
#endif
    }

    /// Return this bounding box merged with another shape.
    template <class T>
    BoundingBox Merged(const T& other) const
    {
        BoundingBox copy = *this;
        copy.Merge(other);
        return copy;
    }

    /// Return this bounding box clipped by another box.
    BoundingBox Clipped(const BoundingBox& box) const
    {
        BoundingBox copy = *this;
        copy.Clip(box);
        return copy;
    }

    /// Return this bounding box expanded by given value.
    BoundingBox Padded(const Vector3& minPadding, const Vector3& maxPadding) const
    {
        if (!Defined())
            return BoundingBox{};

        BoundingBox copy = *this;
        copy.min_ -= minPadding;
        copy.max_ += maxPadding;

        if (copy.min_.x_ > copy.max_.x_ || copy.min_.y_ > copy.max_.y_ || copy.min_.z_ > copy.max_.z_)
            return BoundingBox{};

        return copy;
    }

    /// Return this bounding box expanded by given value, same for min and max.
    BoundingBox Padded(const Vector3& padding) const { return Padded(padding, padding); }

    /// Return true if this bounding box is defined via a previous call to Define() or Merge().
    bool Defined() const
    {
        return min_.x_ != M_INFINITY;
    }

    /// Return center.
    Vector3 Center() const { return (max_ + min_) * 0.5f; }

    /// Return size.
    Vector3 Size() const { return max_ - min_; }

    /// Return half-size.
    Vector3 HalfSize() const { return (max_ - min_) * 0.5f; }

    /// Return volume.
    float Volume() const
    {
        const Vector3 size = Size();
        return size.x_ * size.y_ * size.z_;
    }

    /// Return transformed by a 3x3 matrix.
    BoundingBox Transformed(const Matrix3& transform) const {
        return Transformed(Matrix3x4(transform)); }
    /// Return transformed by a 3x4 matrix.
    BoundingBox Transformed(const Matrix3x4& transform) const;
    /// Return projected by a 4x4 projection matrix.
    Rect Projected(const Matrix4& projection) const;
    /// Return distance to point.
    float DistanceToPoint(const Vector3& point) const;
    /// Return signed distance to point. Negative if point is inside bounding box.
    float SignedDistanceToPoint(const Vector3& point) const;
        /// Return distance to another bounding box.
    float DistanceToBoundingBox(const BoundingBox& box) const;
    /// Return signed distance to another bounding box. Negative if inside.
    float SignedDistanceToBoundingBox(const BoundingBox& box) const;

    /// Test if a point is inside.
    Intersection IsInside(const Vector3& point) const
    {
        if (point.x_ < min_.x_ || point.x_ > max_.x_ || point.y_ < min_.y_ || point.y_ > max_.y_ ||
            point.z_ < min_.z_ || point.z_ > max_.z_)
            return OUTSIDE;
        else
            return INSIDE;
    }

    /// Test if another bounding box is inside, outside or intersects.
    Intersection IsInside(const BoundingBox& box) const
    {
        if (box.max_.x_ < min_.x_ || box.min_.x_ > max_.x_ || box.max_.y_ < min_.y_ || box.min_.y_ > max_.y_ ||
            box.max_.z_ < min_.z_ || box.min_.z_ > max_.z_)
            return OUTSIDE;
        else if (box.min_.x_ < min_.x_ || box.max_.x_ > max_.x_ || box.min_.y_ < min_.y_ || box.max_.y_ > max_.y_ ||
                 box.min_.z_ < min_.z_ || box.max_.z_ > max_.z_)
            return INTERSECTS;
        else
            return INSIDE;
    }

    /// Test if another bounding box is (partially) inside or outside.
    Intersection IsInsideFast(const BoundingBox& box) const
    {
        if (box.max_.x_ < min_.x_ || box.min_.x_ > max_.x_ || box.max_.y_ < min_.y_ || box.min_.y_ > max_.y_ ||
            box.max_.z_ < min_.z_ || box.min_.z_ > max_.z_)
            return OUTSIDE;
        else
            return INSIDE;
    }


    /// Return as string.
    String ToString() const {
        return min_.ToString() + " - " + max_.ToString();
    }

    /// Minimum vector.
    Vector3 min_;
    float dummyMin_{}; // This is never used, but exists to pad the min_ value to four floats.
    /// Maximum vector.
    Vector3 max_;
    float dummyMax_{}; // This is never used, but exists to pad the max_ value to four floats.
};

inline BoundingBox BoundingBox::Transformed(const Matrix3x4& transform) const
{
#ifdef SE_SSE
    const __m128 one = _mm_set_ss(1.f);
    __m128 minPt = _mm_movelh_ps(_mm_loadl_pi(_mm_setzero_ps(), (const __m64*)&min_.x_), _mm_unpacklo_ps(_mm_set_ss(min_.z_), one));
    __m128 maxPt = _mm_movelh_ps(_mm_loadl_pi(_mm_setzero_ps(), (const __m64*)&max_.x_), _mm_unpacklo_ps(_mm_set_ss(max_.z_), one));
    __m128 centerPoint = _mm_mul_ps(_mm_add_ps(minPt, maxPt), _mm_set1_ps(0.5f));
    __m128 halfSize = _mm_sub_ps(centerPoint, minPt);
    __m128 m0 = _mm_loadu_ps(&transform.m00_);
    __m128 m1 = _mm_loadu_ps(&transform.m10_);
    __m128 m2 = _mm_loadu_ps(&transform.m20_);
    __m128 r0 = _mm_mul_ps(m0, centerPoint);
    __m128 r1 = _mm_mul_ps(m1, centerPoint);
    __m128 t0 = _mm_add_ps(_mm_unpacklo_ps(r0, r1), _mm_unpackhi_ps(r0, r1));
    __m128 r2 = _mm_mul_ps(m2, centerPoint);
    const __m128 zero = _mm_setzero_ps();
    __m128 t2 = _mm_add_ps(_mm_unpacklo_ps(r2, zero), _mm_unpackhi_ps(r2, zero));
    __m128 newCenter = _mm_add_ps(_mm_movelh_ps(t0, t2), _mm_movehl_ps(t2, t0));
    const __m128 absMask = _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF));
    __m128 x = _mm_and_ps(absMask, _mm_mul_ps(m0, halfSize));
    __m128 y = _mm_and_ps(absMask, _mm_mul_ps(m1, halfSize));
    __m128 z = _mm_and_ps(absMask, _mm_mul_ps(m2, halfSize));
    t0 = _mm_add_ps(_mm_unpacklo_ps(x, y), _mm_unpackhi_ps(x, y));
    t2 = _mm_add_ps(_mm_unpacklo_ps(z, zero), _mm_unpackhi_ps(z, zero));
    __m128 newDir = _mm_add_ps(_mm_movelh_ps(t0, t2), _mm_movehl_ps(t2, t0));
    return BoundingBox(_mm_sub_ps(newCenter, newDir), _mm_add_ps(newCenter, newDir));
#else
    Vector3 newCenter = transform * Center();
    Vector3 oldEdge = Size() * 0.5f;
    Vector3 newEdge = Vector3(
        Abs(transform.m00_) * oldEdge.x_ + Abs(transform.m01_) * oldEdge.y_ + Abs(transform.m02_) * oldEdge.z_,
        Abs(transform.m10_) * oldEdge.x_ + Abs(transform.m11_) * oldEdge.y_ + Abs(transform.m12_) * oldEdge.z_,
        Abs(transform.m20_) * oldEdge.x_ + Abs(transform.m21_) * oldEdge.y_ + Abs(transform.m22_) * oldEdge.z_
    );

    return BoundingBox(newCenter - newEdge, newCenter + newEdge);
#endif
}

inline Rect BoundingBox::Projected(const Matrix4& projection) const
{
    Vector3 projMin = min_;
    Vector3 projMax = max_;
    if (projMin.z_ < M_MIN_NEARCLIP)
        projMin.z_ = M_MIN_NEARCLIP;
    if (projMax.z_ < M_MIN_NEARCLIP)
        projMax.z_ = M_MIN_NEARCLIP;

    Vector3 vertices[8];
    vertices[0] = projMin;
    vertices[1] = Vector3(projMax.x_, projMin.y_, projMin.z_);
    vertices[2] = Vector3(projMin.x_, projMax.y_, projMin.z_);
    vertices[3] = Vector3(projMax.x_, projMax.y_, projMin.z_);
    vertices[4] = Vector3(projMin.x_, projMin.y_, projMax.z_);
    vertices[5] = Vector3(projMax.x_, projMin.y_, projMax.z_);
    vertices[6] = Vector3(projMin.x_, projMax.y_, projMax.z_);
    vertices[7] = projMax;

    Rect rect;
    for (const auto& vertice : vertices)
    {
        Vector3 projected = projection * vertice;
        rect.Merge(Vector2(projected.x_, projected.y_));
    }

    return rect;
}

inline float BoundingBox::DistanceToPoint(const Vector3& point) const
{
    const Vector3 offset = Center() - point;
    const Vector3 absOffset(Abs(offset.x_), Abs(offset.y_), Abs(offset.z_));
    return VectorMax(Vector3::ZERO, absOffset - HalfSize()).Length();
}

inline float BoundingBox::SignedDistanceToPoint(const Vector3& point) const
{
    const Vector3 absOffset = VectorAbs(Center() - point);
    const Vector3 delta = absOffset - HalfSize();
    const float outerDistance = VectorMax(Vector3::ZERO, delta).Length();
    const float innerDistance = -Min(-delta.x_, Min(-delta.y_, -delta.z_));
    return innerDistance < 0 ? innerDistance : outerDistance;
}

inline float BoundingBox::DistanceToBoundingBox(const BoundingBox& box) const
{
    const Vector3 absOffset = VectorAbs(Center() - box.Center());
    const Vector3 maxHalfSize = VectorMax(HalfSize(), box.HalfSize());
    const Vector3 minHalfSize = VectorMin(HalfSize(), box.HalfSize());

    const Vector3 delta = absOffset - maxHalfSize - minHalfSize;
    return VectorMax(Vector3::ZERO, delta).Length();
}

inline float BoundingBox::SignedDistanceToBoundingBox(const BoundingBox& box) const
{
    const Vector3 absOffset = VectorAbs(Center() - box.Center());
    const Vector3 maxHalfSize = VectorMax(HalfSize(), box.HalfSize());
    const Vector3 minHalfSize = VectorMin(HalfSize(), box.HalfSize());

    const Vector3 outerDelta = absOffset - maxHalfSize - minHalfSize;
    const float outerDistance = VectorMax(Vector3::ZERO, outerDelta).Length();
    const Vector3 innerDelta = maxHalfSize - absOffset - minHalfSize;
    const float innerDistance = -Min(innerDelta.x_, Min(innerDelta.y_, innerDelta.z_));
    return innerDistance < 0 ? innerDistance : outerDistance;
}

#pragma region BoundingSphere

/// Test if a sphere is inside, outside or intersects.
template<>
inline Intersection BoundingBox::IsInside(const BoundingSphere& sphere) const {
    float distSquared = 0;
    float temp;
    const Vector3& center = sphere.GetCenter();

    if (center.x_ < min_.x_)
    {
        temp = center.x_ - min_.x_;
        distSquared += temp * temp;
    }
    else if (center.x_ > max_.x_)
    {
        temp = center.x_ - max_.x_;
        distSquared += temp * temp;
    }
    if (center.y_ < min_.y_)
    {
        temp = center.y_ - min_.y_;
        distSquared += temp * temp;
    }
    else if (center.y_ > max_.y_)
    {
        temp = center.y_ - max_.y_;
        distSquared += temp * temp;
    }
    if (center.z_ < min_.z_)
    {
        temp = center.z_ - min_.z_;
        distSquared += temp * temp;
    }
    else if (center.z_ > max_.z_)
    {
        temp = center.z_ - max_.z_;
        distSquared += temp * temp;
    }

    float radius = sphere.GetRadius();
    if(distSquared > radius * radius)
        return OUTSIDE;
    else if (center.x_ - radius < min_.x_ || center.x_ + radius > max_.x_
             || center.y_ - radius < min_.y_ || center.y_ + radius > max_.y_
             || center.z_ - radius < min_.z_ || center.z_ + radius > max_.z_)
        return INTERSECTS;
    else
        return INSIDE;
}

template<>
bool BoundingSphere::IsInside(const BoundingBox& box) const
{
    return box.IsInside(*this);
}

#pragma endregion BoundingSphere

#pragma region Sphere

/// Test if a sphere is inside, outside or intersects.
template<>
inline Intersection BoundingBox::IsInside(const Sphere& sphere) const {
        float distSquared = 0;
    float temp;
    const Vector3& center = sphere.center_;

    if (center.x_ < min_.x_)
    {
        temp = center.x_ - min_.x_;
        distSquared += temp * temp;
    }
    else if (center.x_ > max_.x_)
    {
        temp = center.x_ - max_.x_;
        distSquared += temp * temp;
    }
    if (center.y_ < min_.y_)
    {
        temp = center.y_ - min_.y_;
        distSquared += temp * temp;
    }
    else if (center.y_ > max_.y_)
    {
        temp = center.y_ - max_.y_;
        distSquared += temp * temp;
    }
    if (center.z_ < min_.z_)
    {
        temp = center.z_ - min_.z_;
        distSquared += temp * temp;
    }
    else if (center.z_ > max_.z_)
    {
        temp = center.z_ - max_.z_;
        distSquared += temp * temp;
    }

    float radius = sphere.radius_;
    if (distSquared >= radius * radius)
        return OUTSIDE;
    else if (center.x_ - radius < min_.x_ || center.x_ + radius > max_.x_ || center.y_ - radius < min_.y_ ||
             center.y_ + radius > max_.y_ || center.z_ - radius < min_.z_ || center.z_ + radius > max_.z_)
        return INTERSECTS;
    else
        return INSIDE;
}
/// Test if a sphere is (partially) inside or outside.
template<> Intersection BoundingBox::IsInsideFast(const Sphere& sphere) const {
    float distSquared = 0;
    float temp;
    const Vector3& center = sphere.center_;

    if (center.x_ < min_.x_)
    {
        temp = center.x_ - min_.x_;
        distSquared += temp * temp;
    }
    else if (center.x_ > max_.x_)
    {
        temp = center.x_ - max_.x_;
        distSquared += temp * temp;
    }
    if (center.y_ < min_.y_)
    {
        temp = center.y_ - min_.y_;
        distSquared += temp * temp;
    }
    else if (center.y_ > max_.y_)
    {
        temp = center.y_ - max_.y_;
        distSquared += temp * temp;
    }
    if (center.z_ < min_.z_)
    {
        temp = center.z_ - min_.z_;
        distSquared += temp * temp;
    }
    else if (center.z_ > max_.z_)
    {
        temp = center.z_ - max_.z_;
        distSquared += temp * temp;
    }

    float radius = sphere.radius_;
    if (distSquared >= radius * radius)
        return OUTSIDE;
    else
        return INSIDE;
}
#pragma region Sphere

#pragma region Polyhedron

template<>
inline void BoundingBox::Merge(const Polyhedron& poly)
{
    for (unsigned i = 0; i < poly.faces_.size(); ++i)
    {
        const std::vector<Vector3>& face = poly.faces_[i];
        if (!face.empty())
            Merge(&face[0], face.size());
    }
}

template<>
inline void BoundingBox::Define(const Polyhedron& poly)
{
    Clear();
    Merge(poly);
}

template<>
inline void Polyhedron::Define(const BoundingBox& box)
{
    Vector3 vertices[8];
    vertices[0] = box.min_;
    vertices[1] = Vector3(box.max_.x_, box.min_.y_, box.min_.z_);
    vertices[2] = Vector3(box.min_.x_, box.max_.y_, box.min_.z_);
    vertices[3] = Vector3(box.max_.x_, box.max_.y_, box.min_.z_);
    vertices[4] = Vector3(box.min_.x_, box.min_.y_, box.max_.z_);
    vertices[5] = Vector3(box.max_.x_, box.min_.y_, box.max_.z_);
    vertices[6] = Vector3(box.min_.x_, box.max_.y_, box.max_.z_);
    vertices[7] = box.max_;

    faces_.resize(6);
    SetFace(0, vertices[3], vertices[7], vertices[5], vertices[1]);
    SetFace(1, vertices[6], vertices[2], vertices[0], vertices[4]);
    SetFace(2, vertices[6], vertices[7], vertices[3], vertices[2]);
    SetFace(3, vertices[1], vertices[5], vertices[4], vertices[0]);
    SetFace(4, vertices[7], vertices[6], vertices[4], vertices[5]);
    SetFace(5, vertices[2], vertices[3], vertices[1], vertices[0]);
}

template<>
inline void Polyhedron::Clip(const BoundingBox& box)
{
    Vector3 vertices[8];
    vertices[0] = box.min_;
    vertices[1] = Vector3(box.max_.x_, box.min_.y_, box.min_.z_);
    vertices[2] = Vector3(box.min_.x_, box.max_.y_, box.min_.z_);
    vertices[3] = Vector3(box.max_.x_, box.max_.y_, box.min_.z_);
    vertices[4] = Vector3(box.min_.x_, box.min_.y_, box.max_.z_);
    vertices[5] = Vector3(box.max_.x_, box.min_.y_, box.max_.z_);
    vertices[6] = Vector3(box.min_.x_, box.max_.y_, box.max_.z_);
    vertices[7] = box.max_;

    Clip(Plane(vertices[5], vertices[7], vertices[3]));
    Clip(Plane(vertices[0], vertices[2], vertices[6]));
    Clip(Plane(vertices[3], vertices[7], vertices[6]));
    Clip(Plane(vertices[4], vertices[5], vertices[1]));
    Clip(Plane(vertices[4], vertices[6], vertices[7]));
    Clip(Plane(vertices[1], vertices[3], vertices[2]));
}

#pragma endregion Polyhedron

#pragma region Sphere

template<>
void BoundingBox::Define(const Sphere& sphere)
{
    const Vector3& center = sphere.center_;
    float radius = sphere.radius_;

    min_ = center + Vector3(-radius, -radius, -radius);
    max_ = center + Vector3(radius, radius, radius);
}

template<>
void BoundingBox::Merge(const Sphere& sphere)
{
    const Vector3& center = sphere.center_;
    float radius = sphere.radius_;

    Merge(center + Vector3(radius, radius, radius));
    Merge(center + Vector3(-radius, -radius, -radius));
}


template<>
void Sphere::Define(const BoundingBox& box)
{
    const Vector3& min = box.min_;
    const Vector3& max = box.max_;

    Clear();
    Merge(min);
    Merge(Vector3(max.x_, min.y_, min.z_));
    Merge(Vector3(min.x_, max.y_, min.z_));
    Merge(Vector3(max.x_, max.y_, min.z_));
    Merge(Vector3(min.x_, min.y_, max.z_));
    Merge(Vector3(max.x_, min.y_, max.z_));
    Merge(Vector3(min.x_, max.y_, max.z_));
    Merge(max);
}

template<>
void Sphere::Merge(const BoundingBox& box)
{
    const Vector3& min = box.min_;
    const Vector3& max = box.max_;

    Merge(min);
    Merge(Vector3(max.x_, min.y_, min.z_));
    Merge(Vector3(min.x_, max.y_, min.z_));
    Merge(Vector3(max.x_, max.y_, min.z_));
    Merge(Vector3(min.x_, min.y_, max.z_));
    Merge(Vector3(max.x_, min.y_, max.z_));
    Merge(Vector3(min.x_, max.y_, max.z_));
    Merge(max);
}

template<>
inline Intersection Sphere::IsInside(const BoundingBox& box) const
{
    float radiusSquared = radius_ * radius_;
    float distSquared = 0;
    float temp;
    Vector3 min = box.min_;
    Vector3 max = box.max_;

    if (center_.x_ < min.x_)
    {
        temp = center_.x_ - min.x_;
        distSquared += temp * temp;
    }
    else if (center_.x_ > max.x_)
    {
        temp = center_.x_ - max.x_;
        distSquared += temp * temp;
    }
    if (center_.y_ < min.y_)
    {
        temp = center_.y_ - min.y_;
        distSquared += temp * temp;
    }
    else if (center_.y_ > max.y_)
    {
        temp = center_.y_ - max.y_;
        distSquared += temp * temp;
    }
    if (center_.z_ < min.z_)
    {
        temp = center_.z_ - min.z_;
        distSquared += temp * temp;
    }
    else if (center_.z_ > max.z_)
    {
        temp = center_.z_ - max.z_;
        distSquared += temp * temp;
    }

    if (distSquared >= radiusSquared)
        return OUTSIDE;

    min -= center_;
    max -= center_;

    Vector3 tempVec = min; // - - -
    if (tempVec.LengthSquared() >= radiusSquared)
        return INTERSECTS;
    tempVec.x_ = max.x_; // + - -
    if (tempVec.LengthSquared() >= radiusSquared)
        return INTERSECTS;
    tempVec.y_ = max.y_; // + + -
    if (tempVec.LengthSquared() >= radiusSquared)
        return INTERSECTS;
    tempVec.x_ = min.x_; // - + -
    if (tempVec.LengthSquared() >= radiusSquared)
        return INTERSECTS;
    tempVec.z_ = max.z_; // - + +
    if (tempVec.LengthSquared() >= radiusSquared)
        return INTERSECTS;
    tempVec.y_ = min.y_; // - - +
    if (tempVec.LengthSquared() >= radiusSquared)
        return INTERSECTS;
    tempVec.x_ = max.x_; // + - +
    if (tempVec.LengthSquared() >= radiusSquared)
        return INTERSECTS;
    tempVec.y_ = max.y_; // + + +
    if (tempVec.LengthSquared() >= radiusSquared)
        return INTERSECTS;

    return INSIDE;
}

template<>
inline Intersection Sphere::IsInsideFast(const BoundingBox& box) const
{
    float radiusSquared = radius_ * radius_;
    float distSquared = 0;
    float temp;
    Vector3 min = box.min_;
    Vector3 max = box.max_;

    if (center_.x_ < min.x_)
    {
        temp = center_.x_ - min.x_;
        distSquared += temp * temp;
    }
    else if (center_.x_ > max.x_)
    {
        temp = center_.x_ - max.x_;
        distSquared += temp * temp;
    }
    if (center_.y_ < min.y_)
    {
        temp = center_.y_ - min.y_;
        distSquared += temp * temp;
    }
    else if (center_.y_ > max.y_)
    {
        temp = center_.y_ - max.y_;
        distSquared += temp * temp;
    }
    if (center_.z_ < min.z_)
    {
        temp = center_.z_ - min.z_;
        distSquared += temp * temp;
    }
    else if (center_.z_ > max.z_)
    {
        temp = center_.z_ - max.z_;
        distSquared += temp * temp;
    }

    if (distSquared >= radiusSquared)
        return OUTSIDE;
    else
        return INSIDE;
}

#pragma endregion Sphere

}
