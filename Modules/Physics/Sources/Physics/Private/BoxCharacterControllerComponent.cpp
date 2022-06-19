#include "Physics/BoxCharacterControllerComponent.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA::Component* BoxCharacterControllerComponent::Clone(Entity* toEntity)
{
    BoxCharacterControllerComponent* result = new BoxCharacterControllerComponent();
    result->SetEntity(toEntity);

    CopyFieldsToComponent(result);

    return result;
}

void BoxCharacterControllerComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    CharacterControllerComponent::Serialize(archive, serializationContext);
    archive->SetFloat("boxCharacterController.halfHeight", halfHeight);
    archive->SetFloat("boxCharacterController.halfForwardExtent", halfForwardExtent);
    archive->SetFloat("boxCharacterController.halfSideExtent", halfSideExtent);
}

void BoxCharacterControllerComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    CharacterControllerComponent::Deserialize(archive, serializationContext);
    halfHeight = archive->GetFloat("boxCharacterController.halfHeight", 1.0f);
    halfForwardExtent = archive->GetFloat("boxCharacterController.halfForwardExtent", 0.5f);
    halfSideExtent = archive->GetFloat("boxCharacterController.halfSideExtent", 0.5f);
}

float32 BoxCharacterControllerComponent::GetHalfHeight() const
{
    return halfHeight;
}

void BoxCharacterControllerComponent::SetHalfHeight(float32 newHalfHeight)
{
    DVASSERT(newHalfHeight > 0.0f);
    halfHeight = newHalfHeight;

    geometryChanged = true;
    ScheduleUpdate();
}

float32 BoxCharacterControllerComponent::GetHalfForwardExtent() const
{
    return halfForwardExtent;
}

void BoxCharacterControllerComponent::SetHalfForwardExtent(float32 newHalfForwardExtent)
{
    DVASSERT(newHalfForwardExtent > 0.0f);
    halfForwardExtent = newHalfForwardExtent;

    geometryChanged = true;
    ScheduleUpdate();
}

float32 BoxCharacterControllerComponent::GetHalfSideExtent() const
{
    return halfSideExtent;
}

void BoxCharacterControllerComponent::SetHalfSideExtent(float32 newHalfSideExtent)
{
    DVASSERT(newHalfSideExtent > 0.0f);
    halfSideExtent = newHalfSideExtent;

    geometryChanged = true;
    ScheduleUpdate();
}

void BoxCharacterControllerComponent::CopyFieldsToComponent(CharacterControllerComponent* dest)
{
    CharacterControllerComponent::CopyFieldsToComponent(dest);

    BoxCharacterControllerComponent* boxComponent = static_cast<BoxCharacterControllerComponent*>(dest);
    boxComponent->halfHeight = halfHeight;
    boxComponent->halfForwardExtent = halfForwardExtent;
    boxComponent->halfSideExtent = halfSideExtent;
}

DAVA_VIRTUAL_REFLECTION_IMPL(BoxCharacterControllerComponent)
{
    ReflectionRegistrator<BoxCharacterControllerComponent>::Begin()
    .ConstructorByPointer()
    .Field("Half height", &BoxCharacterControllerComponent::GetHalfHeight, &BoxCharacterControllerComponent::SetHalfHeight)
    .Field("Half forward extent", &BoxCharacterControllerComponent::GetHalfForwardExtent, &BoxCharacterControllerComponent::SetHalfForwardExtent)
    .Field("Half side extent", &BoxCharacterControllerComponent::GetHalfSideExtent, &BoxCharacterControllerComponent::SetHalfSideExtent)
    .End();
}
} // namespace DAVA