#pragma once

#include "Physics/PhysicsConfigs.h"

#include <ModuleManager/IModule.h>
#include <ModuleManager/ModuleManager.h>

#include <Math/Vector.h>
#include <Base/BaseTypes.h>
#include <Base/Type.h>

#include <physx/PxFiltering.h>

namespace physx
{
class PxFoundation;
class PxPhysics;
class PxCooking;
class PxScene;
class PxActor;
class PxShape;
class PxMaterial;
class PxSimulationEventCallback;
class PxDefaultCpuDispatcher;
class PxAllocatorCallback;
}

namespace DAVA
{
class PolygonGroup;
class Landscape;
class PhysicsGeometryCache;
class PhysicsVehiclesSubsystem;
struct Matrix4;

class PhysicsModule : public IModule
{
public:
    PhysicsModule(Engine* engine);

    void Init() override;
    void Shutdown() override;

    bool IsInitialized() const;
    void* Allocate(size_t size, const char* typeName, const char* filename, int line);
    void Deallocate(void* ptr);

    physx::PxScene* CreateScene(const PhysicsSceneConfig& config, physx::PxSimulationFilterShader filterShader, physx::PxSimulationEventCallback* callback) const;

    physx::PxActor* CreateStaticActor() const;
    physx::PxActor* CreateDynamicActor() const;

    physx::PxShape* CreateBoxShape(const Vector3& halfSize, const FastName& materialName) const;
    physx::PxShape* CreateCapsuleShape(float32 radius, float32 halfHeight, const FastName& materialName) const;
    physx::PxShape* CreateSphereShape(float32 radius, const FastName& materialName) const;
    physx::PxShape* CreatePlaneShape(const FastName& materialName) const;
    physx::PxShape* CreateMeshShape(Vector<PolygonGroup*>&& polygons, const Vector3& scale, const FastName& materialName, PhysicsGeometryCache* cache) const;
    physx::PxShape* CreateConvexHullShape(Vector<PolygonGroup*>&& polygons, const Vector3& scale, const FastName& materialName, PhysicsGeometryCache* cache) const;
    physx::PxShape* CreateHeightField(Landscape* landscape, const FastName& materialName, Matrix4& localPose) const;

    physx::PxMaterial* GetMaterial(const FastName& materialName) const;
    Vector<FastName> GetMaterialNames() const;
    void ReleaseMaterials();

    const Vector<const Type*>& GetBodyComponentTypes() const;
    const Vector<const Type*>& GetShapeComponentTypes() const;
    const Vector<const Type*>& GetVehicleComponentTypes() const;
    const Vector<const Type*>& GetCharacterControllerComponentTypes() const;

    physx::PxAllocatorCallback* GetAllocator() const;

private:
    void LazyLoadMaterials() const;
    void LoadMaterials();

private:
    physx::PxFoundation* foundation = nullptr;
    physx::PxPhysics* physics = nullptr;
    physx::PxCooking* cooking = nullptr;

    mutable physx::PxDefaultCpuDispatcher* cpuDispatcher = nullptr;
    physx::PxMaterial* defaultMaterial = nullptr;
    UnorderedMap<FastName, physx::PxMaterial*> materials;

    class PhysicsAllocator;
    PhysicsAllocator* allocator = nullptr;

    class PhysicsErrotCallback;
    PhysicsErrotCallback* errorCallback = nullptr;

    Vector<const Type*> bodyComponents;
    Vector<const Type*> shapeComponents;
    Vector<const Type*> vehicleComponents;
    Vector<const Type*> characterControllerComponents;

    DAVA_VIRTUAL_REFLECTION(PhysicsModule, IModule);
};
}
