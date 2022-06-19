#pragma once

#include "REPlatform/Scene/Private/Systems/CollisionSystem/CollisionBaseObject.h"

#include <Base/Any.h>
#include <Base/BaseTypes.h>
#include <Math/Plane.h>

namespace DAVA
{
class CollisionBox : public CollisionBaseObject
{
public:
    CollisionBox(const Any& object, btCollisionWorld* word, const Vector3& position, float32 boxSize);
    CollisionBox(const Any& object, btCollisionWorld* word, const Vector3& position, const Vector3& boxSize);
    ~CollisionBox();

    CollisionBaseObject::ClassifyPlaneResult ClassifyToPlane(const Plane& plane) override;
    CollisionBaseObject::ClassifyPlanesResult ClassifyToPlanes(const Vector<Plane>& planes) override;

private:
    void Initialize(const Any& object, btCollisionWorld* word, const Vector3& position, const Vector3& boxSize);

private:
    btCollisionShape* btShape = nullptr;
};
} // namespace DAVA
