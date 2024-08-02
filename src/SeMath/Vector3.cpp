#include "Vector3.hpp"

#include <cstdio>

namespace Se
{


#if 0

int rayBoundBoxIntersection(const Vector3 &point,const Vector3 &dir,const Vector3 &min,const Vector3 &max) {
    float tmin,tmax;
    float idirx = 1.0f/dir.x_;
    float idiry = 1.0f/dir.y_;
    if(dir.x_ >= 0.0f) {
        tmin = (min.x_ - point.x_) * idirx;
        tmax = (max.x_ - point.x_) * idirx;
    } else {
        tmin = (max.x_ - point.x_) * idirx;
        tmax = (min.x_ - point.x_) * idirx;
    }
    float tymin,tymax;
    if(dir.y_ >= 0.0f) {
        tymin = (min.y_ - point.y_) * idiry;
        tymax = (max.y_ - point.y_) * idiry;
    } else {
        tymin = (max.y_ - point.y_) * idiry;
        tymax = (min.y_ - point.y_) * idiry;
    }
    if((tmin > tymax) || (tmax < tymin))
        return 0;
    if(tmin < tymin) tmin = tymin;
    if(tmax > tymax) tmax = tymax;
    float tzmin,tzmax;
    float idirz = 1.0f/dir.z_;
    if(dir.z_ >= 0.0f) {
        tzmin = (min.z_ - point.z_) * idirz;
        tzmax = (max.z_ - point.z_) * idirz;
    } else {
        tzmin = (max.z_ - point.z_) * idirz;
        tzmax = (min.z_ - point.z_) * idirz;
    }
    if((tmin > tzmax) || (tmax < tzmin))
        return 0;
    if(tmin < tzmin) tmin = tzmin;
    if(tmax > tzmax) tmax = tzmax;
    return (tmax > 0.0f && tmin < 1.0f);
}

int irayBoundBoxIntersection(const Vector3 &point,const Vector3 &idir,const Vector3 &min,const Vector3 &max) {
    float tmin,tmax;
    if(idir.x_ >= 0.0f) {
        tmin = (min.x_ - point.x_) * idir.x_;
        tmax = (max.x_ - point.x_) * idir.x_;
    } else {
        tmin = (max.x_ - point.x_) * idir.x_;
        tmax = (min.x_ - point.x_) * idir.x_;
    }
    float tymin,tymax;
    if(idir.y_ >= 0.0f) {
        tymin = (min.y_ - point.y_) * idir.y_;
        tymax = (max.y_ - point.y_) * idir.y_;
    } else {
        tymin = (max.y_ - point.y_) * idir.y_;
        tymax = (min.y_ - point.y_) * idir.y_;
    }
    if((tmin > tymax) || (tmax < tymin))
        return 0;
    if(tmin < tymin) tmin = tymin;
    if(tmax > tymax) tmax = tymax;
    float tzmin,tzmax;
    if(idir.z_ >= 0.0f) {
        tzmin = (min.z_ - point.z_) * idir.z_;
        tzmax = (max.z_ - point.z_) * idir.z_;
    } else {
        tzmin = (max.z_ - point.z_) * idir.z_;
        tzmax = (min.z_ - point.z_) * idir.z_;
    }
    if((tmin > tzmax) || (tmax < tzmin)) return 0;
    if(tmin < tzmin) tmin = tzmin;
    if(tmax > tzmax) tmax = tzmax;
    return (tmax > 0.0f && tmin < 1.0f);
}

int rayTriangleIntersection(const Vector3 &point,const Vector3 &dir,const Vector3 &v0,const Vector3 &v1,const Vector3 &v2) {
    Vector3 v10,v20;
    Vector3 pv0,axis;
    v10 = v1 - v0;
    v20 = v2 - v0;
    axis = dir.CrossProduct(v20);
//    cross(axis,dir,v20);
    float det = v10.DotProduct(axis);
    if(det > 0.0f) {
        pv0 = point - v0;
        float s = pv0.DotProduct(axis);
        if(s < 0.0f || s > det) return 0;
        axis = pv0.CrossProduct(v10);
        float t = dir.DotProduct(axis);
        if(t < 0.0f || t + s > det) return 0;
        return 1;
    } else if(det < 0.0f) {
        pv0 = point - v0;
        float s = pv0.DotProduct(axis);
        if(s > 0.0f || s < det) return 0;
        axis = pv0.CrossProduct(v10);
        float t = dir.DotProduct(axis);
        if(t > 0.0f || t + s < det) return 0;
        return 1;
    }
    return 0;
}

#endif

}
