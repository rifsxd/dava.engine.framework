#include "Physics/HeightFieldShapeComponent.h"
#include "Physics/Private/PhysicsMath.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>
#include <Debug/DVAssert.h>

#include <physx/PxShape.h>

namespace DAVA
{
Component* HeightFieldShapeComponent::Clone(Entity* toEntity)
{
    HeightFieldShapeComponent* result = new HeightFieldShapeComponent();
    result->SetEntity(toEntity);

    CopyFieldsIntoClone(result);

    return result;
}

#if defined(__DAVAENGINE_DEBUG__)
void HeightFieldShapeComponent::CheckShapeType() const
{
    DVASSERT(GetPxShape()->getGeometryType() == physx::PxGeometryType::eHEIGHTFIELD);
}
#endif

void HeightFieldShapeComponent::ReleasePxShape()
{
    physx::PxHeightFieldGeometry geom;
    GetPxShape()->getHeightFieldGeometry(geom);
    physx::PxHeightField* heightfield = geom.heightField;
    CollisionShapeComponent::ReleasePxShape();
    heightfield->release();
}

DAVA_VIRTUAL_REFLECTION_IMPL(HeightFieldShapeComponent)
{
    ReflectionRegistrator<HeightFieldShapeComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

} // namespace DAVA
