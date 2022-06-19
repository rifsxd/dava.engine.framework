#pragma once

#include <Entity/SceneSystem.h>
#include <Base/Set.h>

#include <physx/geometry/PxGeometryHelpers.h>

namespace DAVA
{
class RenderObject;
class CollisionShapeComponent;
class PhysicsDebugDrawSystem final : public SceneSystem
{
public:
    PhysicsDebugDrawSystem(Scene* scene);

    void RegisterEntity(Entity* entity) override;
    void UnregisterEntity(Entity* entity) override;

    void RegisterComponent(Entity* entity, Component* component) override;
    void UnregisterComponent(Entity* entity, Component* component) override;
    void PrepareForRemove() override;
    void Process(float32 timeElapsed) override;

private:
    void Cleanup();

    struct RenderObjectInfo
    {
        RenderObject* ro = nullptr;
        physx::PxGeometryHolder geomHolder;
        physx::PxTransform localPose;
    };
    UnorderedMap<CollisionShapeComponent*, RenderObjectInfo> renderObjects;
    static uint32 vertexLayoutId;

    Set<CollisionShapeComponent*> pendingComponents;
};
} // namespace DAVA