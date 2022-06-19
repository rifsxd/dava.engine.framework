#include "Physics/MeshShapeComponent.h"
#include "Physics/Private/PhysicsMath.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>
#include <Debug/DVAssert.h>

#include <physx/PxShape.h>

namespace DAVA
{
Component* MeshShapeComponent::Clone(Entity* toEntity)
{
    MeshShapeComponent* result = new MeshShapeComponent();
    result->SetEntity(toEntity);

    CopyFieldsIntoClone(result);

    return result;
}

#if defined(__DAVAENGINE_DEBUG__)
void MeshShapeComponent::CheckShapeType() const
{
    DVASSERT(GetPxShape()->getGeometryType() == physx::PxGeometryType::eTRIANGLEMESH);
}
#endif

void MeshShapeComponent::UpdateLocalProperties()
{
    physx::PxShape* shape = GetPxShape();
    DVASSERT(shape != nullptr);
    physx::PxTriangleMeshGeometry geom;
    shape->getTriangleMeshGeometry(geom);
    geom.scale.scale = PhysicsMath::Vector3ToPxVec3(scale);
    shape->setGeometry(geom);

    CollisionShapeComponent::UpdateLocalProperties();
}

DAVA_VIRTUAL_REFLECTION_IMPL(MeshShapeComponent)
{
    ReflectionRegistrator<MeshShapeComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

} // namespace DAVA
