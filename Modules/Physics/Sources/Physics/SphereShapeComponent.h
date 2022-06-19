#pragma once

#include "Physics/CollisionShapeComponent.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
class SphereShapeComponent : public CollisionShapeComponent
{
public:
    Component* Clone(Entity* toEntity) override;

    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    float32 GetRadius() const;
    void SetRadius(float32 radius_);

protected:
#if defined(__DAVAENGINE_DEBUG__)
    void CheckShapeType() const override;
#endif

    void UpdateLocalProperties() override;

private:
    float32 radius = 1.0f;

    DAVA_VIRTUAL_REFLECTION(SphereShapeComponent, CollisionShapeComponent);
};
} // namespace DAVA