#pragma once

#include <SeMath/Matrix4.hpp>

namespace Se
{
class BoundingBox;
class Frustum;

class BoundingSphere
{
    friend class Frustum;
    friend class BoundingBox;

public:
    BoundingSphere() {
        m_Center = Vector3(0.0f);
        m_Radius = 0.0f;
    }
    BoundingSphere(const Vector3& center, float radius) {
        m_Center = center;
        m_Radius = radius;
    }
    BoundingSphere(const Vector3* points, unsigned int count, const Vector3& center = 0.0f, float radius = 0.0f)
    {
        if (count == 0)
        {
            m_Center = Vector3(0.0f);
            m_Radius = 0.0f;
            return;
        }

        m_Center = center;
        m_Radius = radius;

        for (unsigned int i = 0; i < count; i++)
        {
            m_Center += points[i];
        }

        m_Center /= (float)count;

        float maxDistSq = 0.0f;
        for (unsigned int i = 0; i < count; i++)
        {
            float dist = points[i].Distance(m_Center);
            if (dist > maxDistSq)
            {
                maxDistSq = dist;
            }
        }

        m_Radius = maxDistSq;
    }

    BoundingSphere(const BoundingSphere& other) {
        m_Center = other.m_Center;
        m_Radius = other.m_Radius;
    }
    //BoundingSphere(BoundingSphere&& other);
    BoundingSphere& operator=(const BoundingSphere& rhs)
    {
        if (this == &rhs)
            return *this;

        m_Center = rhs.m_Center;
        m_Radius = rhs.m_Radius;

        return *this;
    }
    //BoundingSphere& operator=(BoundingSphere&& other);
    ~BoundingSphere() = default;

    const Vector3& GetCenter() const { return m_Center; }
    float GetRadius() const { return m_Radius; }

    void SetCenter(const Vector3& center) { m_Center = center; }
    void SetRadius(float radius) { m_Radius = radius; }

    template<typename T> bool IsInside(const T& box) const {
        assert(0 && "IsInside not implemented to this object");
        return false;
    }

    // bool IsInside(const BoundingBox& box) const;
    // bool IsInside(const Frustum& frustum) const;

    bool Contains(const Vector3& point) const {
        return (point - m_Center).LengthSquared() <= m_Radius * m_Radius; }
    bool Contains(const BoundingSphere& other) const {
        return other.m_Center.DistanceSquared(m_Center) <= (m_Radius + other.m_Radius) * (m_Radius + other.m_Radius); }
    bool Intersects(const BoundingSphere& other) const {
         return other.m_Center.DistanceSquared(m_Center) <= (m_Radius + other.m_Radius) * (m_Radius + other.m_Radius); }
    bool Intersects(const Vector3& point) const {
        return point.DistanceSquared(m_Center) <= m_Radius * m_Radius; }
    bool Intersects(const Vector3& point, float radius) const {
        return point.DistanceSquared(m_Center) <= (m_Radius + radius) * (m_Radius + radius); }

    void Merge(const BoundingSphere& other)
    {
        float distance = other.m_Center.Distance(m_Center);

        if (distance > m_Radius + other.m_Radius)
            return;

        if (distance <= m_Radius - other.m_Radius)
        {
            m_Center = other.m_Center;
            m_Radius = other.m_Radius;
            return;
        }

        if (distance <= other.m_Radius - m_Radius)
            return;

        float half  = (distance + m_Radius + other.m_Radius) * 0.5f;
        float scale = half / distance;
        m_Center    = (m_Center + other.m_Center) * scale;
        m_Radius    = half;
    }

    void Merge(const Vector3& point)
    {
        float distance = point.Distance(m_Center);

        if (distance > m_Radius)
            return;

        if (distance <= 0.0f)
        {
            m_Center = point;
            m_Radius = 0.0f;
            return;
        }

        float half  = (distance + m_Radius) * 0.5f;
        float scale = half / distance;
        m_Center    = (m_Center + point) * scale;
        m_Radius    = half;
    }

    void Merge(const Vector3* points, unsigned int count)
    {
        if (count == 0)
            return;

        float radius   = 0.0f;
        Vector3 center = points[0];

        for (unsigned int i = 1; i < count; i++)
        {
            float distance = points[i].Distance(center);

            if (distance > radius)
                radius = distance;

            center += points[i];
        }

        center /= (float)count;

        float distance = center.Distance(m_Center);

        if (distance > m_Radius)
            return;

        if (distance <= 0.0f)
        {
            m_Center = center;
            m_Radius = 0.0f;
            return;
        }

        float half  = (distance + m_Radius + radius) * 0.5f;
        float scale = half / distance;
        m_Center    = (m_Center + center) * scale;
        m_Radius    = half;
    }

    void Transform(const Matrix4& transform)
    {
        Vector3 center = (transform * Vector4(m_Center, 1.0f)).GetVector3();

        m_Center = center;
    }

private:
    Vector3 m_Center;
    float m_Radius;
};

template<>
inline bool BoundingSphere::IsInside(const Vector3& point) const
{
    return (point - m_Center).LengthSquared() <= m_Radius * m_Radius;
}

template<>
inline bool BoundingSphere::IsInside(const BoundingSphere& sphere) const
{
    return (sphere.m_Center - m_Center).LengthSquared() <= (m_Radius + sphere.m_Radius) * (m_Radius + sphere.m_Radius);
}


// bool BoundingSphere::IsInside(const Frustum& frustum) const
// {
//     return frustum.IsInside(*this);
// }
    
}
