#pragma once

#include <TArc/DataProcessing/TArcDataNode.h>
#include <TArc/Utils/ReflectedPairsVector.h>

#include <Base/Map.h>
#include <Base/String.h>
#include <Math/Color.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
using CollisionTypesMap = ReflectedPairsVector<int32, String>;

class DebugDrawModuleData final : public TArcDataNode
{
public:
    void SetCollisionTypes(const CollisionTypesMap& newMap);
    const CollisionTypesMap& GetCollisionTypes() const;
    void SetCollisionTypesCrashed(const CollisionTypesMap& newMap);
    const CollisionTypesMap& GetCollisionTypesCrashed() const;

private:
    // These maps has 2 special values, defined in CollisionTypeComponent.h:
    // COLLISION_TYPE_UNDEFINED = -2 and COLLISION_TYPE_OFF = -1
    CollisionTypesMap collisionTypes;
    CollisionTypesMap collisionTypesCrashed;

    DAVA_VIRTUAL_REFLECTION(DebugDrawModuleData, TArcDataNode);
};
} //namespace DAVA
