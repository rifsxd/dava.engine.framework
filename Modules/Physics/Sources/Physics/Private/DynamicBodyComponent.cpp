#include "Physics/DynamicBodyComponent.h"
#include "Physics/PhysicsModule.h"

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>

#include <ModuleManager/ModuleManager.h>
#include <Reflection/ReflectionRegistrator.h>

#include <Base/GlobalEnum.h>

#include <physx/PxRigidDynamic.h>
#include <physx/extensions/PxRigidBodyExt.h>

ENUM_DECLARE(DAVA::DynamicBodyComponent::eLockFlags)
{
    ENUM_ADD_DESCR(DAVA::DynamicBodyComponent::LinearX, "Linear X");
    ENUM_ADD_DESCR(DAVA::DynamicBodyComponent::LinearY, "Linear Y");
    ENUM_ADD_DESCR(DAVA::DynamicBodyComponent::LinearZ, "Linear Z");
    ENUM_ADD_DESCR(DAVA::DynamicBodyComponent::AngularX, "Angular X");
    ENUM_ADD_DESCR(DAVA::DynamicBodyComponent::AngularY, "Angular Y");
    ENUM_ADD_DESCR(DAVA::DynamicBodyComponent::AngularZ, "Angular Z");
}

namespace DAVA
{
Component* DynamicBodyComponent::Clone(Entity* toEntity)
{
    DynamicBodyComponent* result = new DynamicBodyComponent();
    result->SetEntity(toEntity);
    CopyFieldsIntoClone(result);
    result->linearDamping = linearDamping;
    result->angularDamping = angularDamping;
    result->maxAngularVelocity = maxAngularVelocity;
    result->lockFlags = lockFlags;
    result->minPositionIters = minPositionIters;
    result->minVelocityIters = minVelocityIters;
    result->enableCCD = enableCCD;

    return result;
}

void DynamicBodyComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    PhysicsComponent::Serialize(archive, serializationContext);
    archive->SetFloat("dynamicBody.linearDamping", linearDamping);
    archive->SetFloat("dynamicBody.angularDamping", angularDamping);
    archive->SetFloat("dynamicBody.maxAngularVelocity", maxAngularVelocity);
    archive->SetUInt32("dynamicBody.lockFlags", static_cast<uint32>(lockFlags));
    archive->SetUInt32("dynamicBody.minPositionIters", minPositionIters);
    archive->SetUInt32("dynamicBody.minVelocityIters", minVelocityIters);
    archive->SetBool("dynamicBody.enabledCCD", enableCCD);
}

void DynamicBodyComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    PhysicsComponent::Deserialize(archive, serializationContext);
    linearDamping = archive->GetFloat("dynamicBody.linearDamping", linearDamping);
    angularDamping = archive->GetFloat("dynamicBody.angularDamping", angularDamping);
    maxAngularVelocity = archive->GetFloat("dynamicBody.maxAngularVelocity", maxAngularVelocity);
    lockFlags = static_cast<eLockFlags>(archive->GetUInt32("dynamicBody.lockFlags", static_cast<uint32>(lockFlags)));
    minPositionIters = archive->GetUInt32("dynamicBody.minPositionIters", minPositionIters);
    minVelocityIters = archive->GetUInt32("dynamicBody.minVelocityIters", minVelocityIters);
    enableCCD = archive->GetBool("dynamicBody.enabledCCD", enableCCD);
}

float32 DynamicBodyComponent::GetLinearDamping() const
{
    return linearDamping;
}

void DynamicBodyComponent::SetLinearDamping(float32 damping)
{
    linearDamping = damping;
    ScheduleUpdate();
}

float32 DynamicBodyComponent::GetAngularDamping() const
{
    return angularDamping;
}

void DynamicBodyComponent::SetAngularDamping(float32 damping)
{
    angularDamping = damping;
    ScheduleUpdate();
}

float32 DynamicBodyComponent::GetMaxAngularVelocity() const
{
    return maxAngularVelocity;
}

void DynamicBodyComponent::SetMaxAngularVelocity(float32 velocity)
{
    maxAngularVelocity = velocity;
    ScheduleUpdate();
}

uint32 DynamicBodyComponent::GetMinPositionIters() const
{
    return minPositionIters;
}

void DynamicBodyComponent::SetMinPositionIters(uint32 minPositionIters_)
{
    minPositionIters = minPositionIters_;
    ScheduleUpdate();
}

uint32 DynamicBodyComponent::GetMinVelocityIters() const
{
    return minVelocityIters;
}

void DynamicBodyComponent::SetMinVelocityIters(uint32 minVelocityIters_)
{
    minVelocityIters = minVelocityIters_;
    ScheduleUpdate();
}

DynamicBodyComponent::eLockFlags DynamicBodyComponent::GetLockFlags() const
{
    return lockFlags;
}

void DynamicBodyComponent::SetLockFlags(eLockFlags lockFlags_)
{
    lockFlags = lockFlags_;
    ScheduleUpdate();
}

bool DynamicBodyComponent::IsCCDEnabled() const
{
    return enableCCD;
}

void DynamicBodyComponent::SetCCDEnabled(bool isCCDEnabled)
{
    if (isCCDEnabled != enableCCD)
    {
        enableCCD = isCCDEnabled;
        ScheduleUpdate();
    }
}

#if defined(__DAVAENGINE_DEBUG__)
void DynamicBodyComponent::ValidateActorType() const
{
    DVASSERT(GetPxActor()->is<physx::PxRigidDynamic>());
}
#endif

void DynamicBodyComponent::UpdateLocalProperties()
{
    physx::PxRigidDynamic* actor = GetPxActor()->is<physx::PxRigidDynamic>();
    DVASSERT(actor);

    actor->setLinearDamping(linearDamping);
    actor->setAngularDamping(angularDamping);
    actor->setMaxAngularVelocity(maxAngularVelocity);
    actor->setRigidDynamicLockFlags(physx::PxRigidDynamicLockFlags(static_cast<physx::PxRigidDynamicLockFlag::Enum>(lockFlags)));
    actor->setSolverIterationCounts(minPositionIters, minVelocityIters);
    actor->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, enableCCD);
    PhysicsComponent::UpdateLocalProperties();
}

DAVA_VIRTUAL_REFLECTION_IMPL(DynamicBodyComponent)
{
    ReflectionRegistrator<DynamicBodyComponent>::Begin()
    .ConstructorByPointer()
    .Field("Linear damping", &DynamicBodyComponent::GetLinearDamping, &DynamicBodyComponent::SetLinearDamping)[M::Range(0, Any(), 1.0f), M::Group("Damping")]
    .Field("Angular damping", &DynamicBodyComponent::GetAngularDamping, &DynamicBodyComponent::SetAngularDamping)[M::Range(0, Any(), 1.0f), M::Group("Damping")]
    .Field("Max angular velocity", &DynamicBodyComponent::GetMaxAngularVelocity, &DynamicBodyComponent::SetMaxAngularVelocity)[M::Range(0, Any(), 1.0f)]
    .Field("Lock flags", &DynamicBodyComponent::GetLockFlags, &DynamicBodyComponent::SetLockFlags)[M::FlagsT<DynamicBodyComponent::eLockFlags>()]
    .Field("Position iterations count", &DynamicBodyComponent::GetMinPositionIters, &DynamicBodyComponent::SetMinPositionIters)[M::Range(1, 255, 1)]
    .Field("Velocity iterations count", &DynamicBodyComponent::GetMinVelocityIters, &DynamicBodyComponent::SetMinVelocityIters)[M::Range(1, 255, 1)]
    .Field("CCD", &DynamicBodyComponent::IsCCDEnabled, &DynamicBodyComponent::SetCCDEnabled)[M::DisplayName("CCD enabled")]
    .End();
}
} // namespace DAVA