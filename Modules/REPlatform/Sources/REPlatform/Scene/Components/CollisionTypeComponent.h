#pragma once

#include <Base/BaseTypes.h>
#include <Base/GlobalEnum.h>
#include <Entity/Component.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
enum CollisionTypeValues
{
    COLLISION_TYPE_UNDEFINED = -2,
    COLLISION_TYPE_OFF = -1,
};

class CollisionTypeComponent final : public Component
{
public:
    CollisionTypeComponent() = default;

    int32 GetCollisionType() const;
    void SetCollisionType(int32 newType);

    int32 GetCollisionTypeCrashed() const;
    void SetCollisionTypeCrashed(int32 newType);

    Component* Clone(Entity* toEntity) override;

    static const String CollisionTypeFieldName;
    static const String CollisionTypeCrashedFieldName;

private:
    int32 collisionType = COLLISION_TYPE_UNDEFINED;
    int32 collisionTypeCrashed = COLLISION_TYPE_UNDEFINED;

    DAVA_VIRTUAL_REFLECTION(CollisionTypeComponent, Component);
};

CollisionTypeComponent* GetCollisionTypeComponent(const Entity* const fromEntity);
} // namespace DAVA
