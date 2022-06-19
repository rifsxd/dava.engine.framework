#pragma once

#include "REPlatform/Scene/Private/Systems/CollisionSystem/CollisionBaseObject.h"

#include <bullet/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>

#include <Base/Vector.h>
#include <Math/Plane.h>

namespace DAVA
{
class Landscape;
class Entity;
class CollisionLandscape : public CollisionBaseObject
{
public:
    CollisionLandscape(Entity* entity, btCollisionWorld* word, Landscape* landscape);
    virtual ~CollisionLandscape();

    CollisionBaseObject::ClassifyPlaneResult ClassifyToPlane(const Plane& plane) override;
    CollisionBaseObject::ClassifyPlanesResult ClassifyToPlanes(const Vector<Plane>& planes) override;

protected:
    btHeightfieldTerrainShape* btTerrain;
    Vector<float32> btHMap;
};
} // namespace DAVA
