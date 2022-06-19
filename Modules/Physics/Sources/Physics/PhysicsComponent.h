#pragma once

#include <Entity/Component.h>
#include <Scene3D/Entity.h>
#include <Reflection/Reflection.h>

#include <Base/BaseTypes.h>

namespace physx
{
class PxActor;
} // namespace physx

namespace DAVA
{
class Entity;
class KeyedArchive;
class SerializationContext;

class PhysicsComponent : public Component
{
public:
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    enum eBodyFlags
    {
        VISUALIZE = 0x1,
        DISABLE_GRAVITY = 0x2,
        WAKEUP_SPEEL_NOTIFY = 0x4,
        EXCLUDE_FROM_SIMULATION = 0x8,
    };

    eBodyFlags GetBodyFlags() const;
    void SetBodyFlags(eBodyFlags flags);

    physx::PxActor* GetPxActor() const;

    Vector3 currentScale;

    static PhysicsComponent* GetComponent(physx::PxActor* actor);

protected:
#if defined(__DAVAENGINE_DEBUG__)
    virtual void ValidateActorType() const = 0;
#endif
    void SetPxActor(physx::PxActor* actor);
    void CopyFieldsIntoClone(PhysicsComponent* component) const;
    void ScheduleUpdate();

    virtual void UpdateLocalProperties();

private:
    friend class PhysicsSystem;
    void ReleasePxActor();
    physx::PxActor* actor = nullptr;

    eBodyFlags flags = VISUALIZE;

    DAVA_VIRTUAL_REFLECTION(PhysicsComponent, Component);
};

} // namespace DAVA
