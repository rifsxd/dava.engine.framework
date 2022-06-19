#pragma once

#include <Base/Vector.h>
#include <Base/Map.h>

namespace physx
{
class PxBase;
}

namespace DAVA
{
class PolygonGroup;
class PhysicsGeometryCache final
{
public:
    ~PhysicsGeometryCache();

    physx::PxBase* GetConvexHullEntry(const Vector<PolygonGroup*>& key) const;
    physx::PxBase* GetTriangleMeshEntry(const Vector<PolygonGroup*>& key) const;
    void AddEntry(const Vector<PolygonGroup*>& key, physx::PxBase* value);

private:
    Map<Vector<PolygonGroup*>, physx::PxBase*> convexHullCache;
    Map<Vector<PolygonGroup*>, physx::PxBase*> triangleMeshCache;
};
} // namespace DAVA