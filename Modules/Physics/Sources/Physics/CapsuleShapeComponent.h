#pragma once

#include "Physics/CollisionShapeComponent.h"

namespace DAVA
{
class CapsuleShapeComponent : public CollisionShapeComponent
{
public:
    Component* Clone(Entity* toEntity) override;

    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    float32 GetRadius() const;
    void SetRadius(float32 r);

    float32 GetHalfHeight() const;
    void SetHalfHeight(float32 halfHeight);

protected:
#if defined(__DAVAENGINE_DEBUG__)
    void CheckShapeType() const override;
#endif

    void UpdateLocalProperties() override;

private:
    float32 radius = 1.0f;
    float32 halfHeight = 10.0f;

    DAVA_VIRTUAL_REFLECTION(CapsuleShapeComponent, CollisionShapeComponent);
};
} // namespace DAVA