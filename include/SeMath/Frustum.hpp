#pragma once

#include <SeMath/BoundingBox.hpp>

#include <SeMath/Matrix3x4.hpp>
#include <SeMath/Plane.hpp>
#include <SeMath/Rect.hpp>

#include <functional>

#define NUM_FRUSTUM_PLANES 6
#define NUM_FRUSTUM_VERTICES 8

#ifdef near
#undef near
#endif

#ifdef far
#undef far
#endif

namespace Se
{

/// Frustum planes.
enum FrustumPlane
{
    PLANE_NEAR = 0,
    PLANE_LEFT,
    PLANE_RIGHT,
    PLANE_UP,
    PLANE_DOWN,
    PLANE_FAR,
};

/// Convex constructed of 6 planes.
class Frustum
{
public:
    /// Construct a degenerate frustum with all points at origin.
    Frustum() noexcept = default;

    /// Copy-construct from another frustum.
    Frustum(const Frustum& frustum) noexcept {
        *this = frustum; }

    /// Assign from another frustum.
    Frustum& operator =(const Frustum& rhs) noexcept
    {
        for (unsigned i = 0; i < NUM_FRUSTUM_PLANES; ++i)
            planes_[i] = rhs.planes_[i];
        for (unsigned i = 0; i < NUM_FRUSTUM_VERTICES; ++i)
            vertices_[i] = rhs.vertices_[i];

        return *this;
    }

    /// Define with projection parameters and a transform matrix.
    void
        Define(float fov, float aspectRatio, float zoom, float nearZ, float farZ, const Matrix3x4& transform = Matrix3x4::IDENTITY);
    /// Define with near and far dimension vectors and a transform matrix.
    void Define(const Vector3& near, const Vector3& far, const Matrix3x4& transform = Matrix3x4::IDENTITY);
    /// Define with a bounding box and a transform matrix.
    void Define(const BoundingBox& box, const Matrix3x4& transform = Matrix3x4::IDENTITY);
    /// Define from a projection or view-projection matrix.
    void Define(const Matrix4& projection);
    /// Define with orthographic projection parameters and a transform matrix.
    void DefineOrtho
        (float orthoSize, float aspectRatio, float zoom, float nearZ, float farZ, const Matrix3x4& transform = Matrix3x4::IDENTITY);
    /// Define a split (limited) frustum from a projection matrix, with near & far distances specified.
    void DefineSplit(const Matrix4& projection, float near, float far);
    /// Transform by a 3x3 matrix.
    void Transform(const Matrix3& transform)
    {
        for (auto& vertice : vertices_)
            vertice = transform * vertice;

        UpdatePlanes();
    }
    /// Transform by a 3x4 matrix.
    void Transform(const Matrix3x4& transform)
    {
        for (auto& vertice : vertices_)
            vertice = transform * vertice;

        UpdatePlanes();
    }

    /// Test if a point is inside or outside.
    Intersection IsInside(const Vector3& point) const
    {
        for (const auto& plane : planes_)
        {
            if (plane.Distance(point) < 0.0f)
                return OUTSIDE;
        }

        return INSIDE;
    }

    /// Test if a sphere is inside, outside or intersects.
    Intersection IsInside(const Sphere& sphere) const
    {
        bool allInside = true;
        for (const auto& plane : planes_)
        {
            float dist = plane.Distance(sphere.center_);
            if (dist < -sphere.radius_)
                return OUTSIDE;
            else if (dist < sphere.radius_)
                allInside = false;
        }

        return allInside ? INSIDE : INTERSECTS;
    }

    /// Test if a sphere if (partially) inside or outside.
    Intersection IsInsideFast(const Sphere& sphere) const
    {
        for (const auto& plane : planes_)
        {
            if (plane.Distance(sphere.center_) < -sphere.radius_)
                return OUTSIDE;
        }

        return INSIDE;
    }

    /// Test if a bounding box is inside, outside or intersects.
    Intersection IsInside(const BoundingBox& box) const
    {
        Vector3 center = box.Center();
        Vector3 edge = center - box.min_;
        bool allInside = true;

        for (const auto& plane : planes_)
        {
            float dist = plane.normal_.DotProduct(center) + plane.d_;
            float absDist = plane.absNormal_.DotProduct(edge);

            if (dist < -absDist)
                return OUTSIDE;
            else if (dist < absDist)
                allInside = false;
        }

        return allInside ? INSIDE : INTERSECTS;
    }

    bool IsInside(const BoundingSphere& sphere) const
    {
        for(int i = 0; i < 6; i++)
        {
            if(planes_[i].Distance(sphere.GetCenter()) < -sphere.GetRadius())
            {
                return false;
            }
        }

        return true;
    }


    /// Test if a bounding box is (partially) inside or outside.
    Intersection IsInsideFast(const BoundingBox& box) const
    {
        Vector3 center = box.Center();
        Vector3 edge = center - box.min_;

        for (const auto& plane : planes_)
        {
            float dist = plane.normal_.DotProduct(center) + plane.d_;
            float absDist = plane.absNormal_.DotProduct(edge);

            if (dist < -absDist)
                return OUTSIDE;
        }

        return INSIDE;
    }

    /// Return distance of a point to the frustum, or 0 if inside.
    float Distance(const Vector3& point) const
    {
        float distance = 0.0f;
        for (const auto& plane : planes_)
            distance = Max(-plane.Distance(point), distance);

        return distance;
    }

    /// Return transformed by a 3x3 matrix.
    Frustum Transformed(const Matrix3& transform) const
    {
        Frustum transformed;
        for (unsigned i = 0; i < NUM_FRUSTUM_VERTICES; ++i)
            transformed.vertices_[i] = transform * vertices_[i];

        transformed.UpdatePlanes();
        return transformed;
    }

    /// Return transformed by a 3x4 matrix.
    Frustum Transformed(const Matrix3x4& transform) const
    {
        Frustum transformed;
        for (unsigned i = 0; i < NUM_FRUSTUM_VERTICES; ++i)
            transformed.vertices_[i] = transform * vertices_[i];

        transformed.UpdatePlanes();
        return transformed;
    }
    /// Return projected by a 4x4 projection matrix.
    Rect Projected(const Matrix4& projection) const;

    /// Update the planes. Called internally.
    void UpdatePlanes()
    {
        planes_[PLANE_NEAR].Define(vertices_[2], vertices_[1], vertices_[0]);
        planes_[PLANE_LEFT].Define(vertices_[3], vertices_[7], vertices_[6]);
        planes_[PLANE_RIGHT].Define(vertices_[1], vertices_[5], vertices_[4]);
        planes_[PLANE_UP].Define(vertices_[0], vertices_[4], vertices_[7]);
        planes_[PLANE_DOWN].Define(vertices_[6], vertices_[5], vertices_[1]);
        planes_[PLANE_FAR].Define(vertices_[5], vertices_[6], vertices_[7]);

        // Check if we ended up with inverted planes (reflected transform) and flip in that case
        if (planes_[PLANE_NEAR].Distance(vertices_[5]) < 0.0f)
        {
            for (auto& plane : planes_)
            {
                plane.normal_ = -plane.normal_;
                plane.d_ = -plane.d_;
            }
        }
    }



    /// Frustum planes.
    Plane planes_[NUM_FRUSTUM_PLANES];
    /// Frustum vertices.
    Vector3 vertices_[NUM_FRUSTUM_VERTICES];
};



inline void Frustum::Define(float fov, float aspectRatio, float zoom, float nearZ, float farZ, const Matrix3x4& transform)
{
    nearZ = Max(nearZ, 0.0f);
    farZ = Max(farZ, nearZ);
    float halfViewSize = tanf(fov * M_DEGTORAD_2) / zoom;
    Vector3 near, far;

    near.z_ = nearZ;
    near.y_ = near.z_ * halfViewSize;
    near.x_ = near.y_ * aspectRatio;
    far.z_ = farZ;
    far.y_ = far.z_ * halfViewSize;
    far.x_ = far.y_ * aspectRatio;

    Define(near, far, transform);
}

inline void Frustum::Define(const Vector3& near, const Vector3& far, const Matrix3x4& transform)
{
    vertices_[0] = transform * near;
    vertices_[1] = transform * Vector3(near.x_, -near.y_, near.z_);
    vertices_[2] = transform * Vector3(-near.x_, -near.y_, near.z_);
    vertices_[3] = transform * Vector3(-near.x_, near.y_, near.z_);
    vertices_[4] = transform * far;
    vertices_[5] = transform * Vector3(far.x_, -far.y_, far.z_);
    vertices_[6] = transform * Vector3(-far.x_, -far.y_, far.z_);
    vertices_[7] = transform * Vector3(-far.x_, far.y_, far.z_);

    UpdatePlanes();
}

inline void Frustum::Define(const BoundingBox& box, const Matrix3x4& transform)
{
    vertices_[0] = transform * Vector3(box.max_.x_, box.max_.y_, box.min_.z_);
    vertices_[1] = transform * Vector3(box.max_.x_, box.min_.y_, box.min_.z_);
    vertices_[2] = transform * Vector3(box.min_.x_, box.min_.y_, box.min_.z_);
    vertices_[3] = transform * Vector3(box.min_.x_, box.max_.y_, box.min_.z_);
    vertices_[4] = transform * Vector3(box.max_.x_, box.max_.y_, box.max_.z_);
    vertices_[5] = transform * Vector3(box.max_.x_, box.min_.y_, box.max_.z_);
    vertices_[6] = transform * Vector3(box.min_.x_, box.min_.y_, box.max_.z_);
    vertices_[7] = transform * Vector3(box.min_.x_, box.max_.y_, box.max_.z_);

    UpdatePlanes();
}

inline void Frustum::Define(const Matrix4& projection)
{
    Matrix4 projInverse = projection.Inverse();

    vertices_[0] = projInverse * Vector3(1.0f, 1.0f, 0.0f);
    vertices_[1] = projInverse * Vector3(1.0f, -1.0f, 0.0f);
    vertices_[2] = projInverse * Vector3(-1.0f, -1.0f, 0.0f);
    vertices_[3] = projInverse * Vector3(-1.0f, 1.0f, 0.0f);
    vertices_[4] = projInverse * Vector3(1.0f, 1.0f, 1.0f);
    vertices_[5] = projInverse * Vector3(1.0f, -1.0f, 1.0f);
    vertices_[6] = projInverse * Vector3(-1.0f, -1.0f, 1.0f);
    vertices_[7] = projInverse * Vector3(-1.0f, 1.0f, 1.0f);

    UpdatePlanes();
}

inline void Frustum::DefineOrtho(float orthoSize, float aspectRatio, float zoom, float nearZ, float farZ, const Matrix3x4& transform)
{
    nearZ = Max(nearZ, 0.0f);
    farZ = Max(farZ, nearZ);
    float halfViewSize = orthoSize * 0.5f / zoom;
    Vector3 near, far;

    near.z_ = nearZ;
    far.z_ = farZ;
    far.y_ = near.y_ = halfViewSize;
    far.x_ = near.x_ = near.y_ * aspectRatio;

    Define(near, far, transform);
}

inline void Frustum::DefineSplit(const Matrix4& projection, float near, float far)
{
    Matrix4 projInverse = projection.Inverse();

    // Figure out depth values for near & far
    Vector4 nearTemp = projection * Vector4(0.0f, 0.0f, near, 1.0f);
    Vector4 farTemp = projection * Vector4(0.0f, 0.0f, far, 1.0f);
    float nearZ = nearTemp.z_ / nearTemp.w_;
    float farZ = farTemp.z_ / farTemp.w_;

    vertices_[0] = projInverse * Vector3(1.0f, 1.0f, nearZ);
    vertices_[1] = projInverse * Vector3(1.0f, -1.0f, nearZ);
    vertices_[2] = projInverse * Vector3(-1.0f, -1.0f, nearZ);
    vertices_[3] = projInverse * Vector3(-1.0f, 1.0f, nearZ);
    vertices_[4] = projInverse * Vector3(1.0f, 1.0f, farZ);
    vertices_[5] = projInverse * Vector3(1.0f, -1.0f, farZ);
    vertices_[6] = projInverse * Vector3(-1.0f, -1.0f, farZ);
    vertices_[7] = projInverse * Vector3(-1.0f, 1.0f, farZ);

    UpdatePlanes();
}

inline Rect Frustum::Projected(const Matrix4& projection) const
{
    auto ClipEdgeZ = [](const Vector3& v0, const Vector3& v1, float clipZ) -> Vector3
    {
        return Vector3(
            v1.x_ + (v0.x_ - v1.x_) * ((clipZ - v1.z_) / (v0.z_ - v1.z_)),
            v1.y_ + (v0.y_ - v1.y_) * ((clipZ - v1.z_) / (v0.z_ - v1.z_)),
            clipZ
        );
    };

    static auto ProjectAndMergeEdge = [ClipEdgeZ](Vector3 v0, Vector3 v1, Rect& rect, const Matrix4& projection)
    {
        // Check if both vertices behind near plane
        if (v0.z_ < M_MIN_NEARCLIP && v1.z_ < M_MIN_NEARCLIP)
            return;

        // Check if need to clip one of the vertices
        if (v1.z_ < M_MIN_NEARCLIP)
            v1 = ClipEdgeZ(v1, v0, M_MIN_NEARCLIP);
        else if (v0.z_ < M_MIN_NEARCLIP)
            v0 = ClipEdgeZ(v0, v1, M_MIN_NEARCLIP);

        // Project, perspective divide and merge
        Vector3 tV0(projection * v0);
        Vector3 tV1(projection * v1);
        rect.Merge(Vector2(tV0.x_, tV0.y_));
        rect.Merge(Vector2(tV1.x_, tV1.y_));
    };


    Rect rect;

    ProjectAndMergeEdge(vertices_[0], vertices_[4], rect, projection);
    ProjectAndMergeEdge(vertices_[1], vertices_[5], rect, projection);
    ProjectAndMergeEdge(vertices_[2], vertices_[6], rect, projection);
    ProjectAndMergeEdge(vertices_[3], vertices_[7], rect, projection);
    ProjectAndMergeEdge(vertices_[4], vertices_[5], rect, projection);
    ProjectAndMergeEdge(vertices_[5], vertices_[6], rect, projection);
    ProjectAndMergeEdge(vertices_[6], vertices_[7], rect, projection);
    ProjectAndMergeEdge(vertices_[7], vertices_[4], rect, projection);

    return rect;
}

#pragma region BoundingSphere
template<>
bool BoundingSphere::IsInside(const Frustum& frustum) const
{
    return frustum.IsInside(*this);
}
#pragma endregion BoundingSphere

#pragma region Polyhedron

template<>
inline void Polyhedron::Define(const Frustum& frustum)
{
    const Vector3* vertices = frustum.vertices_;

    faces_.resize(6);
    SetFace(0, vertices[0], vertices[4], vertices[5], vertices[1]);
    SetFace(1, vertices[7], vertices[3], vertices[2], vertices[6]);
    SetFace(2, vertices[7], vertices[4], vertices[0], vertices[3]);
    SetFace(3, vertices[1], vertices[5], vertices[6], vertices[2]);
    SetFace(4, vertices[4], vertices[7], vertices[6], vertices[5]);
    SetFace(5, vertices[3], vertices[0], vertices[1], vertices[2]);
}

template<>
inline void Polyhedron::Clip(const Frustum& frustum)
{
    for (const auto& plane : frustum.planes_)
        Clip(plane);
}


#pragma endregion Polyhedron

#pragma region BoundingBox


template<>
void BoundingBox::Define(const Frustum& frustum)
{
    Clear();
    Define(frustum.vertices_, NUM_FRUSTUM_VERTICES);
}

template<>
void BoundingBox::Merge(const Frustum& frustum)
{
    Merge(frustum.vertices_, NUM_FRUSTUM_VERTICES);
}

#pragma endregion BoundingBox


#pragma region Sphere
template<>
inline void Sphere::Define(const Frustum& frustum)
{
    Define(frustum.vertices_, NUM_FRUSTUM_VERTICES);
}

template<>
inline void Sphere::Merge(const Frustum& frustum)
{
    const Vector3* vertices = frustum.vertices_;
    Merge(vertices, NUM_FRUSTUM_VERTICES);
}

#pragma endregion Sphere


}
