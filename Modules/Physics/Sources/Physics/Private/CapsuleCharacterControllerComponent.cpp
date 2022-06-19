#include "Physics/CapsuleCharacterControllerComponent.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA::Component* CapsuleCharacterControllerComponent::Clone(Entity* toEntity)
{
    CapsuleCharacterControllerComponent* result = new CapsuleCharacterControllerComponent();
    result->SetEntity(toEntity);

    CopyFieldsToComponent(result);

    return result;
}

void CapsuleCharacterControllerComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    CharacterControllerComponent::Serialize(archive, serializationContext);
    archive->SetFloat("capsuleCharacterController.radius", radius);
    archive->SetFloat("capsuleCharacterController.height", height);
}

void CapsuleCharacterControllerComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    CharacterControllerComponent::Deserialize(archive, serializationContext);
    radius = archive->GetFloat("capsuleCharacterController.radius", radius);
    height = archive->GetFloat("capsuleCharacterController.height", height);
}

float32 CapsuleCharacterControllerComponent::GetRadius() const
{
    return radius;
}

void CapsuleCharacterControllerComponent::SetRadius(float32 newRadius)
{
    DVASSERT(newRadius > 0.0f);
    radius = newRadius;

    geometryChanged = true;
    ScheduleUpdate();
}

float32 CapsuleCharacterControllerComponent::GetHeight() const
{
    return height;
}

void CapsuleCharacterControllerComponent::SetHeight(float32 newHeight)
{
    DVASSERT(newHeight > 0.0f);
    height = newHeight;

    geometryChanged = true;
    ScheduleUpdate();
}

void CapsuleCharacterControllerComponent::CopyFieldsToComponent(CharacterControllerComponent* dest)
{
    CharacterControllerComponent::CopyFieldsToComponent(dest);

    CapsuleCharacterControllerComponent* capsuleComponent = static_cast<CapsuleCharacterControllerComponent*>(dest);
    capsuleComponent->radius = radius;
    capsuleComponent->height = height;
}

DAVA_VIRTUAL_REFLECTION_IMPL(CapsuleCharacterControllerComponent)
{
    ReflectionRegistrator<CapsuleCharacterControllerComponent>::Begin()
    .ConstructorByPointer()
    .Field("Radius", &CapsuleCharacterControllerComponent::GetRadius, &CapsuleCharacterControllerComponent::SetRadius)
    .Field("Height", &CapsuleCharacterControllerComponent::GetHeight, &CapsuleCharacterControllerComponent::SetHeight)
    .End();
}
} // namespace DAVA