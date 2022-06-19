#pragma once

#include "REPlatform/DataNodes/Selectable.h"

#include <bullet/btBulletCollisionCommon.h>
#include <Base/Any.h>
#include <Math/Transform.h>
#include <Math/TransformUtils.h>

namespace DAVA
{
class CollisionBaseObject
{
public:
    enum class ClassifyPointResult
    {
        InFront,
        Behind
    };

    enum class ClassifyPlaneResult
    {
        InFront,
        Intersects,
        Behind
    };

    enum class ClassifyPlanesResult
    {
        ContainsOrIntersects,
        Outside
    };

public:
    CollisionBaseObject(const Any& object_, btCollisionWorld* word)
        : btWord(word)
        , object(object_)
    {
    }

    virtual ~CollisionBaseObject() = default;

    virtual ClassifyPlaneResult ClassifyToPlane(const Plane& plane) = 0;
    virtual ClassifyPlanesResult ClassifyToPlanes(const Vector<Plane>& planes) = 0;

    CollisionBaseObject::ClassifyPlaneResult ClassifyBoundingBoxToPlane(const AABBox3& bbox, const Plane& plane) const;
    Plane TransformPlaneToLocalSpace(const Plane& plane) const;

    btCollisionObject* btObject = nullptr;
    btCollisionWorld* btWord = nullptr;
    Selectable object;
};

inline CollisionBaseObject::ClassifyPlaneResult CollisionBaseObject::ClassifyBoundingBoxToPlane(const AABBox3& bbox, const Plane& plane) const
{
    char cornersData[8 * sizeof(Vector3)];
    Vector3* corners = reinterpret_cast<Vector3*>(cornersData);
    bbox.GetCorners(corners);

    float32 minDistance = std::numeric_limits<float>::max();
    float32 maxDistance = -minDistance;
    for (uint32 i = 0; i < 8; ++i)
    {
        float d = plane.DistanceToPoint(corners[i]);
        minDistance = std::min(minDistance, d);
        maxDistance = std::max(maxDistance, d);
    }

    if ((minDistance > 0.0f) && (maxDistance > 0.0f))
        return ClassifyPlaneResult::InFront;

    if ((minDistance < 0.0f) && (maxDistance < 0.0f))
        return ClassifyPlaneResult::Behind;

    return ClassifyPlaneResult::Intersects;
}

inline Plane CollisionBaseObject::TransformPlaneToLocalSpace(const Plane& plane) const
{
    Matrix4 transform = TransformUtils::ToMatrix(object.GetWorldTransform());
    transform.Transpose();
    return Plane(Vector4(plane.n.x, plane.n.y, plane.n.z, plane.d) * transform);
}
} // namespace DAVA
