#pragma once

#include <SeMath/Plane.hpp>
#include <SeMath/Sphere.hpp>

//#include <SeMath/BoundingBox.h>

#include <vector>
#include <algorithm>

namespace Se
{

//class BoundingBox;
//class Frustum;
//class Matrix3;
//class Matrix3x4;
//class Plane;

/// A convex volume built from polygon faces.
class Polyhedron
{
public:
    /// Construct empty.
    Polyhedron() noexcept = default;
    /// Destruct.
    ~Polyhedron() noexcept = default;

    /// Copy-construct from another polyhedron.
    Polyhedron(const Polyhedron& polyhedron) :
        faces_(polyhedron.faces_)
    {
    }

    /// Construct from a list of faces.
    explicit Polyhedron(const std::vector<std::vector<Vector3> >& faces) :
        faces_(faces)
    {
    }

    // /// Construct from a bounding box.
    // explicit Polyhedron(const BoundingBox& box)
    // {
    //     Define(box);
    // }

    /// Construct from a frustum.
    // explicit Polyhedron(const Frustum& frustum)
    // {
    //     Define(frustum);
    // }

    /// Assign from another polyhedron.
    Polyhedron& operator =(const Polyhedron& rhs)
    {
        faces_ = rhs.faces_;
        return *this;
    }

    template<typename T> void Define(const T& obj) {
        SE_LOG_ERROR("Define not implemented to this object"); }
    template<typename T> void Clip(const T& obj) {
        SE_LOG_ERROR("Clip not implemented to this object"); }



    // /// Define from a bounding box.
    // void Define(const BoundingBox& box);
    // /// Define from a frustum.
    // void Define(const Frustum& frustum);
    /// Add a triangle face.
    void AddFace(const Vector3& v0, const Vector3& v1, const Vector3& v2);
    /// Add a quadrilateral face.
    void AddFace(const Vector3& v0, const Vector3& v1, const Vector3& v2, const Vector3& v3);
    /// Add an arbitrary face.
    void AddFace(const std::vector<Vector3>& face);
    // /// Clip with a plane.
    // void Clip(const Plane& plane);
    // /// Clip with a bounding box.
    // void Clip(const BoundingBox& box);
    // /// Clip with a frustum.
    // void Clip(const Frustum& frustum);
    /// clear all faces.
    void clear();
    /// Transform with a 3x3 matrix.
    void Transform(const Matrix3& transform);
    /// Transform with a 3x4 matrix.
    void Transform(const Matrix3x4& transform);

    /// Return transformed with a 3x3 matrix.
    Polyhedron Transformed(const Matrix3& transform) const;
    /// Return transformed with a 3x4 matrix.
    Polyhedron Transformed(const Matrix3x4& transform) const;

    /// Return whether is empty.
    bool Empty() const { return faces_.empty(); }

    /// Polygon faces.
    std::vector<std::vector<Vector3>> faces_;

private:
    /// Set a triangle face by index.
    void SetFace(unsigned index, const Vector3& v0, const Vector3& v1, const Vector3& v2);
    /// Set a quadrilateral face by index.
    void SetFace(unsigned index, const Vector3& v0, const Vector3& v1, const Vector3& v2, const Vector3& v3);
    /// Internal vector for clipped vertices.
    std::vector<Vector3> clippedVertices_;
    /// Internal vector for the new face being constructed.
    std::vector<Vector3> outFace_;
};

#if 1

// inline void Polyhedron::Define(const BoundingBox& box)
// {
//     Vector3 vertices[8];
//     vertices[0] = box.min_;
//     vertices[1] = Vector3(box.max_.x_, box.min_.y_, box.min_.z_);
//     vertices[2] = Vector3(box.min_.x_, box.max_.y_, box.min_.z_);
//     vertices[3] = Vector3(box.max_.x_, box.max_.y_, box.min_.z_);
//     vertices[4] = Vector3(box.min_.x_, box.min_.y_, box.max_.z_);
//     vertices[5] = Vector3(box.max_.x_, box.min_.y_, box.max_.z_);
//     vertices[6] = Vector3(box.min_.x_, box.max_.y_, box.max_.z_);
//     vertices[7] = box.max_;

//     faces_.resize(6);
//     SetFace(0, vertices[3], vertices[7], vertices[5], vertices[1]);
//     SetFace(1, vertices[6], vertices[2], vertices[0], vertices[4]);
//     SetFace(2, vertices[6], vertices[7], vertices[3], vertices[2]);
//     SetFace(3, vertices[1], vertices[5], vertices[4], vertices[0]);
//     SetFace(4, vertices[7], vertices[6], vertices[4], vertices[5]);
//     SetFace(5, vertices[2], vertices[3], vertices[1], vertices[0]);
// }


inline void Polyhedron::AddFace(const Vector3& v0, const Vector3& v1, const Vector3& v2)
{
    faces_.resize(faces_.size() + 1);
    std::vector<Vector3>& face = faces_[faces_.size() - 1];
    face.resize(3);
    face[0] = v0;
    face[1] = v1;
    face[2] = v2;
}

inline void Polyhedron::AddFace(const Vector3& v0, const Vector3& v1, const Vector3& v2, const Vector3& v3)
{
    faces_.resize(faces_.size() + 1);
    std::vector<Vector3>& face = faces_[faces_.size() - 1];
    face.resize(4);
    face[0] = v0;
    face[1] = v1;
    face[2] = v2;
    face[3] = v3;
}

inline void Polyhedron::AddFace(const std::vector<Vector3>& face)
{
    faces_.push_back(face);
}

template<>
inline void Polyhedron::Clip(const Plane& plane)
{
    clippedVertices_.clear();

    for (unsigned i = 0; i < faces_.size(); ++i)
    {
        std::vector<Vector3>& face = faces_[i];
        Vector3 lastVertex;
        float lastDistance = 0.0f;

        outFace_.clear();

        for (unsigned j = 0; j < face.size(); ++j)
        {
            float distance = plane.Distance(face[j]);
            if (distance >= 0.0f)
            {
                if (lastDistance < 0.0f)
                {
                    float t = lastDistance / (lastDistance - distance);
                    Vector3 clippedVertex = lastVertex + t * (face[j] - lastVertex);
                    outFace_.push_back(clippedVertex);
                    clippedVertices_.push_back(clippedVertex);
                }

                outFace_.push_back(face[j]);
            }
            else
            {
                if (lastDistance >= 0.0f && j != 0)
                {
                    float t = lastDistance / (lastDistance - distance);
                    Vector3 clippedVertex = lastVertex + t * (face[j] - lastVertex);
                    outFace_.push_back(clippedVertex);
                    clippedVertices_.push_back(clippedVertex);
                }
            }

            lastVertex = face[j];
            lastDistance = distance;
        }

        // Recheck the distances of the last and first vertices and add the final clipped vertex if applicable
        float distance = plane.Distance(face[0]);
        if ((lastDistance < 0.0f && distance >= 0.0f) || (lastDistance >= 0.0f && distance < 0.0f))
        {
            float t = lastDistance / (lastDistance - distance);
            Vector3 clippedVertex = lastVertex + t * (face[0] - lastVertex);
            outFace_.push_back(clippedVertex);
            clippedVertices_.push_back(clippedVertex);
        }

        // Do not keep faces which are less than triangles
        if (outFace_.size() < 3)
            outFace_.clear();

        face = outFace_;
    }

    // Remove empty faces
    for (unsigned i = faces_.size() - 1; i < faces_.size(); --i)
    {
        if (faces_[i].empty())
            faces_.erase(faces_.begin() + i); // faces_.erase(i);
    }

    // std::erase_if(faces_, [](std::vector<Vector3> fase) { 
    //         return fase.empty(); });

    

    // Create a new face from the clipped vertices. First remove duplicates
    for (unsigned i = 0; i < clippedVertices_.size(); ++i)
    {
        for (unsigned j = clippedVertices_.size() - 1; j > i; --j)
        {
            if (clippedVertices_[j].Equals(clippedVertices_[i]))
                clippedVertices_.erase(clippedVertices_.begin() + j);
                //clippedVertices_.erase(j); 
        }
    }

    if (clippedVertices_.size() > 3)
    {
        outFace_.clear();

        // Start with the first vertex
        outFace_.push_back(clippedVertices_.front());
        clippedVertices_.erase(clippedVertices_.begin()); //erase(0)

        while (!clippedVertices_.empty())
        {
            // Then add the vertex which is closest to the last added
            const Vector3& lastAdded = outFace_.back();
            float bestDistance = M_INFINITY;
            unsigned bestIndex = 0;

            for (unsigned i = 0; i < clippedVertices_.size(); ++i)
            {
                float distance = Vector3(clippedVertices_[i] - lastAdded).LengthSquared();
                if (distance < bestDistance)
                {
                    bestDistance = distance;
                    bestIndex = i;
                }
            }

            outFace_.push_back(clippedVertices_[bestIndex]);
            clippedVertices_.erase(clippedVertices_.begin() + bestIndex);
        }

        faces_.push_back(outFace_);
    }
}





inline void Polyhedron::clear()
{
    faces_.clear();
}

inline void Polyhedron::Transform(const Matrix3& transform)
{
    for (unsigned i = 0; i < faces_.size(); ++i)
    {
        std::vector<Vector3>& face = faces_[i];
        for (unsigned j = 0; j < face.size(); ++j)
            face[j] = transform * face[j];
    }
}

inline void Polyhedron::Transform(const Matrix3x4& transform)
{
    for (unsigned i = 0; i < faces_.size(); ++i)
    {
        std::vector<Vector3>& face = faces_[i];
        for (unsigned j = 0; j < face.size(); ++j)
            face[j] = transform * face[j];
    }
}

inline Polyhedron Polyhedron::Transformed(const Matrix3& transform) const
{
    Polyhedron ret;
    ret.faces_.resize(faces_.size());

    for (unsigned i = 0; i < faces_.size(); ++i)
    {
        const std::vector<Vector3>& face = faces_[i];
        std::vector<Vector3>& newFace = ret.faces_[i];
        newFace.resize(face.size());

        for (unsigned j = 0; j < face.size(); ++j)
            newFace[j] = transform * face[j];
    }

    return ret;
}

inline Polyhedron Polyhedron::Transformed(const Matrix3x4& transform) const
{
    Polyhedron ret;
    ret.faces_.resize(faces_.size());

    for (unsigned i = 0; i < faces_.size(); ++i)
    {
        const std::vector<Vector3>& face = faces_[i];
        std::vector<Vector3>& newFace = ret.faces_[i];
        newFace.resize(face.size());

        for (unsigned j = 0; j < face.size(); ++j)
            newFace[j] = transform * face[j];
    }

    return ret;
}

inline void Polyhedron::SetFace(unsigned index, const Vector3& v0, const Vector3& v1, const Vector3& v2)
{
    std::vector<Vector3>& face = faces_[index];
    face.resize(3);
    face[0] = v0;
    face[1] = v1;
    face[2] = v2;
}

inline void Polyhedron::SetFace(unsigned index, const Vector3& v0, const Vector3& v1, const Vector3& v2, const Vector3& v3)
{
    std::vector<Vector3>& face = faces_[index];
    face.resize(4);
    face[0] = v0;
    face[1] = v1;
    face[2] = v2;
    face[3] = v3;
}


#endif


#pragma region Sphere

template<>
inline void Sphere::Merge(const Polyhedron& poly)
{
    for (unsigned i = 0; i < poly.faces_.size(); ++i)
    {
        const std::vector<Vector3>& face = poly.faces_[i];
        if (!face.empty())
            Merge(&face[0], face.size());
    }
}

template<>
inline void Sphere::Define(const Polyhedron& poly)
{
    Clear();
    Merge(poly);
}


#pragma endregion Sphere



}
