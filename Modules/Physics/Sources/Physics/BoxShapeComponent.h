#pragma once

#include "Physics/CollisionShapeComponent.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
class BoxShapeComponent : public CollisionShapeComponent
{
public:
    Component* Clone(Entity* toEntity) override;

    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    const Vector3& GetHalfSize() const;
    void SetHalfSize(const Vector3& size);

protected:
#if defined(__DAVAENGINE_DEBUG__)
    void CheckShapeType() const override;
#endif

    void UpdateLocalProperties() override;

private:
    Vector3 halfExtents = Vector3(1.0f, 1.0f, 1.0f);

    DAVA_VIRTUAL_REFLECTION(BoxShapeComponent, CollisionShapeComponent);
};
} // namespace DAVA