#include "Physics/PhysicsComponent.h"
#include "Physics/PhysicsModule.h"
#include "Physics/PhysicsSystem.h"

#include <Scene3D/Scene.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Base/GlobalEnum.h>

#include <physx/PxActor.h>

ENUM_DECLARE(DAVA::PhysicsComponent::eBodyFlags)
{
    ENUM_ADD_DESCR(DAVA::PhysicsComponent::VISUALIZE, "Visualize shape");
    ENUM_ADD_DESCR(DAVA::PhysicsComponent::DISABLE_GRAVITY, "Disable gravity for body");
    ENUM_ADD_DESCR(DAVA::PhysicsComponent::WAKEUP_SPEEL_NOTIFY, "Generate wakeup/sleep events");
    ENUM_ADD_DESCR(DAVA::PhysicsComponent::EXCLUDE_FROM_SIMULATION, "Disable simulation for body");
}

namespace DAVA
{
void PhysicsComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
    archive->SetUInt32("body.flags", static_cast<uint32>(flags));
}

void PhysicsComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);
    flags = static_cast<eBodyFlags>(archive->GetUInt32("body.flags", static_cast<uint32>(flags)));
}

PhysicsComponent::eBodyFlags PhysicsComponent::GetBodyFlags() const
{
    return flags;
}

void PhysicsComponent::SetBodyFlags(eBodyFlags flags_)
{
    flags = flags_;
    ScheduleUpdate();
}

physx::PxActor* PhysicsComponent::GetPxActor() const
{
    return actor;
}

PhysicsComponent* PhysicsComponent::GetComponent(physx::PxActor* actor)
{
    DVASSERT(actor != nullptr);
    return reinterpret_cast<PhysicsComponent*>(actor->userData);
}

void PhysicsComponent::SetPxActor(physx::PxActor* actor_)
{
    DVASSERT(actor_ != nullptr);
    DVASSERT(actor == nullptr);
    actor = actor_;
    actor->userData = this;
#if defined(__DAVAENGINE_DEBUG__)
    ValidateActorType();
#endif

    ScheduleUpdate();
}

void PhysicsComponent::CopyFieldsIntoClone(PhysicsComponent* component) const
{
    component->flags = flags;
}

void PhysicsComponent::ScheduleUpdate()
{
    if (actor != nullptr)
    {
        Entity* entity = GetEntity();
        DVASSERT(entity != nullptr);
        Scene* scene = entity->GetScene();
        DVASSERT(scene != nullptr);
        scene->physicsSystem->ScheduleUpdate(this);
    }
}

void PhysicsComponent::UpdateLocalProperties()
{
    DVASSERT(actor != nullptr);
    auto flagConvert = [](eBodyFlags bodyFlags, eBodyFlags flag, physx::PxActorFlag::Enum pxFlag, physx::PxActorFlags& pxFlags)
    {
        if ((bodyFlags & flag) != 0)
        {
            pxFlags |= pxFlag;
        }
    };

    physx::PxActorFlags pxFlags;
    flagConvert(flags, VISUALIZE, physx::PxActorFlag::eVISUALIZATION, pxFlags);
    flagConvert(flags, DISABLE_GRAVITY, physx::PxActorFlag::eDISABLE_GRAVITY, pxFlags);
    flagConvert(flags, WAKEUP_SPEEL_NOTIFY, physx::PxActorFlag::eSEND_SLEEP_NOTIFIES, pxFlags);
    flagConvert(flags, EXCLUDE_FROM_SIMULATION, physx::PxActorFlag::eDISABLE_SIMULATION, pxFlags);
    actor->setActorFlags(pxFlags);
}

void PhysicsComponent::ReleasePxActor()
{
    DVASSERT(actor != nullptr);
    actor->release();
    actor = nullptr;
}

DAVA_VIRTUAL_REFLECTION_IMPL(PhysicsComponent)
{
    ReflectionRegistrator<PhysicsComponent>::Begin()
    .Field("Body flags", &PhysicsComponent::GetBodyFlags, &PhysicsComponent::SetBodyFlags)[M::FlagsT<PhysicsComponent::eBodyFlags>()]
    .End();
}

} // namespace DAVA
