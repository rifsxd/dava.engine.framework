#pragma once

#include <Entity/SceneSystem.h>
#include <Math/Vector.h>
#include <Base/BaseTypes.h>

#include <physx/PxQueryReport.h>
#include <physx/PxSimulationEventCallback.h>
#include <physx/PxForceMode.h>

namespace physx
{
class PxScene;
class PxRigidActor;
class PxShape;
class PxControllerManager;
}

namespace DAVA
{
class Scene;
class CollisionSingleComponent;
class PhysicsModule;
class PhysicsComponent;
class DynamicBodyComponent;
class CollisionShapeComponent;
class PhysicsGeometryCache;
class PhysicsVehiclesSubsystem;
class CharacterControllerComponent;

class PhysicsSystem final : public SceneSystem
{
public:
    PhysicsSystem(Scene* scene);
    ~PhysicsSystem() override;

    void RegisterEntity(Entity* entity) override;
    void UnregisterEntity(Entity* entity) override;

    void RegisterComponent(Entity* entity, Component* component) override;
    void UnregisterComponent(Entity* entity, Component* component) override;

    void PrepareForRemove() override;

    void Process(float32 timeElapsed) override;

    void SetSimulationEnabled(bool isEnabled);
    bool IsSimulationEnabled() const;

    void SetDebugDrawEnabled(bool drawDebugInfo);
    bool IsDebugDrawEnabled() const;

    void ScheduleUpdate(PhysicsComponent* component);
    void ScheduleUpdate(CollisionShapeComponent* component);
    void ScheduleUpdate(CharacterControllerComponent* component);

    bool Raycast(const Vector3& origin, const Vector3& direction, float32 distance, physx::PxRaycastCallback& callback);
    void AddForce(DynamicBodyComponent* component, const Vector3& force, physx::PxForceMode::Enum mode);

    PhysicsVehiclesSubsystem* GetVehiclesSystem();

private:
    bool FetchResults(bool waitForFetchFinish);

    void DrawDebugInfo();

    void InitNewObjects();
    void AttachShapesRecursively(Entity* entity, PhysicsComponent* bodyComponent, const Vector3& scale);
    void AttachShape(PhysicsComponent* bodyComponent, CollisionShapeComponent* shapeComponent, const Vector3& scale);

    void ReleaseShape(CollisionShapeComponent* component);
    physx::PxShape* CreateShape(CollisionShapeComponent* component, PhysicsModule* physics);

    void SyncTransformToPhysx();
    void SyncEntityTransformToPhysx(Entity* entity);
    void UpdateComponents();
    void ApplyForces();

    void MoveCharacterControllers(float32 timeElapsed);

private:
    class SimulationEventCallback : public physx::PxSimulationEventCallback
    {
    public:
        SimulationEventCallback(CollisionSingleComponent* targetCollisionSingleComponent);
        void onConstraintBreak(physx::PxConstraintInfo*, physx::PxU32) override;
        void onWake(physx::PxActor**, physx::PxU32) override;
        void onSleep(physx::PxActor**, physx::PxU32) override;
        void onTrigger(physx::PxTriggerPair*, physx::PxU32) override;
        void onAdvance(const physx::PxRigidBody* const*, const physx::PxTransform*, const physx::PxU32) override;
        void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;

    private:
        CollisionSingleComponent* targetCollisionSingleComponent;
    };

private:
    friend class PhysicsSystemPrivate; // for tests only

    void* simulationBlock = nullptr;
    uint32 simulationBlockSize = 0;

    bool isSimulationEnabled = true;
    bool isSimulationRunning = false;
    physx::PxScene* physicsScene = nullptr;
    physx::PxControllerManager* controllerManager = nullptr;
    PhysicsGeometryCache* geometryCache = nullptr;

    PhysicsVehiclesSubsystem* vehiclesSubsystem = nullptr;

    Vector<PhysicsComponent*> physicsComponents;
    Vector<PhysicsComponent*> pendingAddPhysicsComponents;

    Vector<CollisionShapeComponent*> collisionComponents;
    Vector<CollisionShapeComponent*> pendingAddCollisionComponents;

    Vector<CharacterControllerComponent*> characterControllerComponents;
    Vector<CharacterControllerComponent*> pendingAddCharacterControllerComponents;

    UnorderedMap<Entity*, Vector<CollisionShapeComponent*>> waitRenderInfoComponents;

    Set<PhysicsComponent*> physicsComponensUpdatePending;
    Set<CollisionShapeComponent*> collisionComponentsUpdatePending;
    Set<CharacterControllerComponent*> characterControllerComponentsUpdatePending;

    struct PendingForce
    {
        DynamicBodyComponent* component = nullptr;
        Vector3 force;
        physx::PxForceMode::Enum mode;
    };

    Vector<PendingForce> forces;
    SimulationEventCallback simulationEventCallback;

    bool drawDebugInfo = false;
};
} // namespace DAVA