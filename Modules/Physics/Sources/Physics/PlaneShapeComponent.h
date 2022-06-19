#pragma once

#include "Physics/CollisionShapeComponent.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
class PlaneShapeComponent : public CollisionShapeComponent
{
public:
    Component* Clone(Entity* toEntity) override;

    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    const Vector3& GetPoint() const;
    void SetPoint(const Vector3& point);

    const Vector3& GetNormal() const;
    void SetNormal(const Vector3& normal);

protected:
#if defined(__DAVAENGINE_DEBUG__)
    void CheckShapeType() const override;
#endif

    void UpdateLocalProperties() override;

private:
    Vector3 point = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 normal = Vector3(0.0f, 0.0f, 1.0f);

    DAVA_VIRTUAL_REFLECTION(PlaneShapeComponent, CollisionShapeComponent);
};
} // namespace DAVA