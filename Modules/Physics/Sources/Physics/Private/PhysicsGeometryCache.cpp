#include "Physics/PhysicsGeometryCache.h"

#include <Debug/DVAssert.h>
#include <Base/BaseTypes.h>

#include <physx/geometry/PxConvexMesh.h>
#include <physx/geometry/PxTriangleMesh.h>

namespace PhysicsGeometryCache
{
using namespace DAVA;
#if defined(__DAVAENGINE_DEBUG__)
bool ValidateKey(const Vector<PolygonGroup*>& key)
{
    Vector<PolygonGroup*> copy = key;
    std::sort(copy.begin(), copy.end());
    copy.erase(std::unique(copy.begin(), copy.end()), copy.end());

    return copy == key;
}
#endif

void ReleaseCache(Map<Vector<PolygonGroup*>, physx::PxBase*>& cache)
{
    for (const auto& node : cache)
    {
        node.second->release();
    }

    cache.clear();
}

physx::PxBase* GetEntry(const Map<Vector<PolygonGroup*>, physx::PxBase*>& cache,
                        const Vector<PolygonGroup*>& key)
{
    auto iter = cache.find(key);
    if (iter != cache.end())
    {
        return iter->second;
    }

    return nullptr;
}
}

namespace DAVA
{
PhysicsGeometryCache::~PhysicsGeometryCache()
{
    using namespace PhysicsGeometryCache;
    ReleaseCache(convexHullCache);
    ReleaseCache(triangleMeshCache);
}

physx::PxBase* PhysicsGeometryCache::GetConvexHullEntry(const Vector<PolygonGroup*>& key) const
{
    using namespace PhysicsGeometryCache;
#if defined(__DAVAENGINE_DEBUG__)
    DVASSERT(ValidateKey(key) == true);
#endif

    return GetEntry(convexHullCache, key);
}

physx::PxBase* PhysicsGeometryCache::GetTriangleMeshEntry(const Vector<PolygonGroup*>& key) const
{
    using namespace PhysicsGeometryCache;
#if defined(__DAVAENGINE_DEBUG__)
    DVASSERT(ValidateKey(key) == true);
#endif

    return GetEntry(triangleMeshCache, key);
}

void PhysicsGeometryCache::AddEntry(const Vector<PolygonGroup*>& key, physx::PxBase* value)
{
    using namespace PhysicsGeometryCache;
#if defined(__DAVAENGINE_DEBUG__)
    DVASSERT(ValidateKey(key) == true);
#endif

    if (value->is<physx::PxConvexMesh>() != nullptr)
    {
        bool inserted = convexHullCache.emplace(key, value).second;
        DVASSERT(inserted == true);
    }
    else if (value->is<physx::PxTriangleMesh>() != nullptr)
    {
        bool inserted = triangleMeshCache.emplace(key, value).second;
        DVASSERT(inserted == true);
    }
    else
    {
        DVASSERT(false);
    }
}

} // namespace DAVA
