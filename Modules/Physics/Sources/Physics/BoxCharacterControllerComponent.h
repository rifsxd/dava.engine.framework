#pragma once

#include <Entity/Component.h>
#include <Scene3D/Entity.h>

#include <Reflection/Reflection.h>

#include <Physics/CharacterControllerComponent.h>

namespace DAVA
{
class BoxCharacterControllerComponent final : public CharacterControllerComponent
{
public:
    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    float32 GetHalfHeight() const;
    void SetHalfHeight(float32 newHalfHeight);

    float32 GetHalfForwardExtent() const;
    void SetHalfForwardExtent(float32 newHalfForwardExtent);

    float32 GetHalfSideExtent() const;
    void SetHalfSideExtent(float32 newHalfSizeExtent);

private:
    friend class PhysicsSystem;

    void CopyFieldsToComponent(CharacterControllerComponent* dest) override;

    float32 halfHeight = 1.0f;
    float32 halfForwardExtent = 0.5f;
    float32 halfSideExtent = 0.5f;

    DAVA_VIRTUAL_REFLECTION(BoxCharacterControllerComponent, CharacterControllerComponent);
};
} // namespace DAVA