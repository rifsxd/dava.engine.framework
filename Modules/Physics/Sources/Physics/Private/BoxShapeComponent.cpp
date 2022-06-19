#include "Physics/BoxShapeComponent.h"
#include "Physics/Private/PhysicsMath.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>
#include <Debug/DVAssert.h>

#include <physx/PxShape.h>
#include <physx/geometry/PxBoxGeometry.h>

namespace DAVA
{
Component* BoxShapeComponent::Clone(Entity* toEntity)
{
    BoxShapeComponent* result = new BoxShapeComponent();
    result->SetEntity(toEntity);

    result->halfExtents = halfExtents;
    CopyFieldsIntoClone(result);

    return result;
}

void BoxShapeComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    CollisionShapeComponent::Serialize(archive, serializationContext);
    archive->SetVector3("boxShape.halfExtents", halfExtents);
}

void BoxShapeComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    CollisionShapeComponent::Deserialize(archive, serializationContext);
    halfExtents = archive->GetVector3("boxShape.halfExtents", Vector3(1.0f, 1.0f, 1.0f));
}

const Vector3& BoxShapeComponent::GetHalfSize() const
{
    return halfExtents;
}

void BoxShapeComponent::SetHalfSize(const Vector3& size)
{
    halfExtents = size;
    DVASSERT(halfExtents.x > 0.0f);
    DVASSERT(halfExtents.y > 0.0f);
    DVASSERT(halfExtents.z > 0.0f);
    ScheduleUpdate();
}

#if defined(__DAVAENGINE_DEBUG__)
void BoxShapeComponent::CheckShapeType() const
{
    DVASSERT(GetPxShape()->getGeometryType() == physx::PxGeometryType::eBOX);
}
#endif

void BoxShapeComponent::UpdateLocalProperties()
{
    physx::PxShape* shape = GetPxShape();
    DVASSERT(shape != nullptr);
    physx::PxBoxGeometry geom;
    shape->getBoxGeometry(geom);
    geom.halfExtents = PhysicsMath::Vector3ToPxVec3(halfExtents);
    DVASSERT(geom.isValid());
    shape->setGeometry(geom);
    CollisionShapeComponent::UpdateLocalProperties();
}

DAVA_VIRTUAL_REFLECTION_IMPL(BoxShapeComponent)
{
    ReflectionRegistrator<BoxShapeComponent>::Begin()
    .ConstructorByPointer()
    .Field("Half size", &BoxShapeComponent::GetHalfSize, &BoxShapeComponent::SetHalfSize)[M::Range(Vector3(0.00001f, 0.00001f, 0.00001f), Any(), Vector3(1.0f, 1.0f, 1.0f))]
    .End();
}

} // namespace DAVA
