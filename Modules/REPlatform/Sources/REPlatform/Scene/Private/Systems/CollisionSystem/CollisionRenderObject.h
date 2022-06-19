#pragma once

#include "REPlatform/Scene/Private/Systems/CollisionSystem/CollisionBaseObject.h"

#include <Base/Vector.h>
#include <Math/Plane.h>

namespace DAVA
{
class Entity;
class RenderObject;
class CollisionRenderObject : public CollisionBaseObject
{
public:
    CollisionRenderObject(Entity* entity, btCollisionWorld* word, RenderObject* renderObject);
    ~CollisionRenderObject() override;

    CollisionBaseObject::ClassifyPlaneResult ClassifyToPlane(const Plane& plane) override;
    ClassifyPlanesResult ClassifyToPlanes(const Vector<Plane>& planes) override;

protected:
    btTriangleMesh* btTriangles = nullptr;
    btCollisionShape* btShape = nullptr;
};
} // namespace DAVA
