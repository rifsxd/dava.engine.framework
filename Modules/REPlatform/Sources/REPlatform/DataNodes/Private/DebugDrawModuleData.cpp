#include "REPlatform/DataNodes/DebugDrawModuleData.h"

namespace DAVA
{
void DebugDrawModuleData::SetCollisionTypes(const CollisionTypesMap& newMap)
{
    collisionTypes = newMap;
}

const CollisionTypesMap& DebugDrawModuleData::GetCollisionTypes() const
{
    return collisionTypes;
}

void DebugDrawModuleData::SetCollisionTypesCrashed(const CollisionTypesMap& newMap)
{
    collisionTypesCrashed = newMap;
}

const CollisionTypesMap& DebugDrawModuleData::GetCollisionTypesCrashed() const
{
    return collisionTypesCrashed;
}

DAVA_VIRTUAL_REFLECTION_IMPL(DebugDrawModuleData)
{
    DAVA::ReflectionRegistrator<DebugDrawModuleData>::Begin()
    .Field("collisionTypes", &DebugDrawModuleData::GetCollisionTypes, &DebugDrawModuleData::SetCollisionTypes)
    .Field("collisionTypesCrashed", &DebugDrawModuleData::GetCollisionTypesCrashed, &DebugDrawModuleData::SetCollisionTypesCrashed)
    .End();
}
}; //namespace DAVA
