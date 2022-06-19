#include "Physics/WASDPhysicsControllerComponent.h"
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(WASDPhysicsControllerComponent)
{
    ReflectionRegistrator<WASDPhysicsControllerComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* WASDPhysicsControllerComponent::Clone(Entity* toEntity)
{
    WASDPhysicsControllerComponent* component = new WASDPhysicsControllerComponent();
    component->SetEntity(toEntity);

    return component;
}
};
