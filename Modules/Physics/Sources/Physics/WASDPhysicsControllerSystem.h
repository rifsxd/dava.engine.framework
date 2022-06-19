#pragma once

#include <Entity/SceneSystem.h>

namespace DAVA
{
class WASDPhysicsControllerComponent;

class WASDPhysicsControllerSystem : public SceneSystem
{
public:
    WASDPhysicsControllerSystem(Scene* scene);

    void RegisterEntity(Entity* e) override;
    void UnregisterEntity(Entity* e) override;
    void RegisterComponent(Entity* e, Component* c) override;
    void UnregisterComponent(Entity* e, Component* c) override;
    void PrepareForRemove() override;

    void Process(float32 timeElapsed) override;

private:
    Set<WASDPhysicsControllerComponent*> wasdComponents;
    float32 moveSpeedCoeff = 0.2f;
};
}