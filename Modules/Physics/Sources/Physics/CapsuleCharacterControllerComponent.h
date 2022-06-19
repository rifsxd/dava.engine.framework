#pragma once

#include <Entity/Component.h>
#include <Scene3D/Entity.h>

#include <Reflection/Reflection.h>

#include <Physics/CharacterControllerComponent.h>

namespace DAVA
{
class CapsuleCharacterControllerComponent final : public CharacterControllerComponent
{
public:
    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    float32 GetRadius() const;
    void SetRadius(float32 newRadius);

    float32 GetHeight() const;
    void SetHeight(float32 newHeight);

private:
    friend class PhysicsSystem;

    void CopyFieldsToComponent(CharacterControllerComponent* dest) override;

    float32 radius = 0.1f;
    float32 height = 1.0f;

    DAVA_VIRTUAL_REFLECTION(CapsuleCharacterControllerComponent, CharacterControllerComponent);
};
} // namespace DAVA