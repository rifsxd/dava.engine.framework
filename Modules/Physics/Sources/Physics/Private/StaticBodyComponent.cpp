#include "Physics/StaticBodyComponent.h"
#include "Physics/PhysicsModule.h"

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>

#include <ModuleManager/ModuleManager.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Debug/DVAssert.h>

#include <physx/PxRigidStatic.h>

namespace DAVA
{
Component* StaticBodyComponent::Clone(Entity* toEntity)
{
    StaticBodyComponent* result = new StaticBodyComponent();
    result->SetEntity(toEntity);

    CopyFieldsIntoClone(result);
    return result;
}

void StaticBodyComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    PhysicsComponent::Serialize(archive, serializationContext);
}

void StaticBodyComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    PhysicsComponent::Deserialize(archive, serializationContext);
}

#if defined(__DAVAENGINE_DEBUG__)
void StaticBodyComponent::ValidateActorType() const
{
    DVASSERT(GetPxActor()->is<physx::PxRigidStatic>());
}
#endif

DAVA_VIRTUAL_REFLECTION_IMPL(StaticBodyComponent)
{
    ReflectionRegistrator<StaticBodyComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

} // namespace DAVA
