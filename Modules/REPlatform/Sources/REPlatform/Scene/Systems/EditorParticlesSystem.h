#pragma once

#include "REPlatform/Scene/Systems/EditorSceneSystem.h"

#include <Base/Vector.h>
#include <Entity/SceneSystem.h>
#include <UI/UIEvent.h>

namespace DAVA
{
class RECommandNotificationObject;

struct ParticleLayer;
class ParticleEmitterInstance;
class ParticleEffectComponent;
class ParticleForce;
class Entity;
class RECommand;
class SceneEditor2;

class EditorParticlesSystem : public SceneSystem, public EditorSceneSystem
{
    friend class SceneEditor2;

public:
    EditorParticlesSystem(Scene* scene);

    void RestartParticleEffects();

    ParticleEffectComponent* GetEmitterOwner(ParticleEmitterInstance* emitter) const;
    template <typename T>
    ParticleLayer* GetForceOwner(T* force) const;
    ParticleEmitterInstance* GetRootEmitterLayerOwner(ParticleLayer* layer) const;
    ParticleEmitterInstance* GetDirectEmitterLayerOwner(ParticleLayer* layer) const;

protected:
    void Draw() override;
    void ProcessCommand(const RECommandNotificationObject& commandNotification) override;

private:
    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;

    void DrawDebugInfoForEffect(Entity* effectEntity);
    void DrawEmitter(ParticleEmitterInstance* emitter, Entity* owner, bool selected);

    void DrawSizeCircle(Entity* effectEntity, ParticleEmitterInstance* emitter);
    void DrawSizeSphere(Entity* effectEntity, ParticleEmitterInstance* emitter);
    void DrawSizeBox(Entity* effectEntity, ParticleEmitterInstance* emitter);
    void DrawVectorArrow(ParticleEmitterInstance* emitter, Vector3 center);

    void DrawParticleForces(ParticleForce* force);
    void FillEmitterRadii(Entity* effectEntity, ParticleEmitterInstance* emitter, float32& radius, float32& innerRadius);

private:
    Vector<Entity*> entities;
};
} // namespace DAVA
