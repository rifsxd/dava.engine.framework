#include "REPlatform/Scene/Components/CollisionTypeComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedTypeDB.h>
#include <Scene3D/Entity.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(CollisionTypeComponent)
{
    ReflectionRegistrator<CollisionTypeComponent>::Begin()
    [M::NonExportableComponent(), M::NonSerializableComponent()]
    .ConstructorByPointer()
    .Field(CollisionTypeComponent::CollisionTypeFieldName.c_str(),
           &CollisionTypeComponent::GetCollisionType,
           &CollisionTypeComponent::SetCollisionType)
    [M::DisplayName("Collision Type")]
    .Field(CollisionTypeComponent::CollisionTypeCrashedFieldName.c_str(),
           &CollisionTypeComponent::GetCollisionTypeCrashed,
           &CollisionTypeComponent::SetCollisionTypeCrashed)
    [M::DisplayName("Collision Type Crashed")]
    .End();
}

int32 CollisionTypeComponent::GetCollisionType() const
{
    return collisionType;
}

void CollisionTypeComponent::SetCollisionType(int32 newType)
{
    collisionType = newType;
}

int32 CollisionTypeComponent::GetCollisionTypeCrashed() const
{
    return collisionTypeCrashed;
}

void CollisionTypeComponent::SetCollisionTypeCrashed(int32 newType)
{
    collisionTypeCrashed = newType;
}

Component* CollisionTypeComponent::Clone(Entity* toEntity)
{
    CollisionTypeComponent* newComponent = new CollisionTypeComponent();
    newComponent->SetCollisionType(collisionType);
    newComponent->SetCollisionTypeCrashed(collisionTypeCrashed);
    newComponent->SetEntity(toEntity);
    return newComponent;
}

CollisionTypeComponent* GetCollisionTypeComponent(const Entity* const fromEntity)
{
    if (fromEntity != nullptr)
    {
        return fromEntity->GetComponent<CollisionTypeComponent>();
    }
    return nullptr;
}

const String CollisionTypeComponent::CollisionTypeFieldName = "collisionType";
const String CollisionTypeComponent::CollisionTypeCrashedFieldName = "collisionTypeCrashed";
} //namespace DAVA
