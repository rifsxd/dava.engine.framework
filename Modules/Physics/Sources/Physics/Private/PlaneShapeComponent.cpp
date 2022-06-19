#include "Physics/PlaneShapeComponent.h"
#include "Physics/Private/PhysicsMath.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Debug/DVAssert.h>

#include <physx/PxShape.h>
#include <physx/geometry/PxBoxGeometry.h>

namespace DAVA
{
Component* PlaneShapeComponent::Clone(Entity* toEntity)
{
    PlaneShapeComponent* result = new PlaneShapeComponent();
    result->SetEntity(toEntity);

    result->point = point;
    result->normal = normal;
    CopyFieldsIntoClone(result);

    return result;
}

void PlaneShapeComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    CollisionShapeComponent::Serialize(archive, serializationContext);
    archive->SetVector3("planeShape.point", point);
    archive->SetVector3("planeShape.normal", normal);
}

void PlaneShapeComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    CollisionShapeComponent::Deserialize(archive, serializationContext);
    point = archive->GetVector3("planeShape.point", point);
    normal = archive->GetVector3("planeShape.normal", normal);
}

const Vector3& PlaneShapeComponent::GetPoint() const
{
    return point;
}

void PlaneShapeComponent::SetPoint(const Vector3& point_)
{
    point = point_;
    ScheduleUpdate();
}

const Vector3& PlaneShapeComponent::GetNormal() const
{
    return normal;
}

void PlaneShapeComponent::SetNormal(const Vector3& normal_)
{
    normal = Normalize(normal_);
    ScheduleUpdate();
}

#if defined(__DAVAENGINE_DEBUG__)
void PlaneShapeComponent::CheckShapeType() const
{
    DVASSERT(GetPxShape()->getGeometryType() == physx::PxGeometryType::ePLANE);
}
#endif

void PlaneShapeComponent::UpdateLocalProperties()
{
    physx::PxPlane plane(PhysicsMath::Vector3ToPxVec3(point), PhysicsMath::Vector3ToPxVec3(normal));
    physx::PxTransform planeTransform = physx::PxTransformFromPlaneEquation(plane);
    localPose = PhysicsMath::PxMat44ToMatrix4(physx::PxMat44(planeTransform));
    CollisionShapeComponent::UpdateLocalProperties();
}

DAVA_VIRTUAL_REFLECTION_IMPL(PlaneShapeComponent)
{
    ReflectionRegistrator<PlaneShapeComponent>::Begin()
    .ConstructorByPointer()
    .Field("Point", &PlaneShapeComponent::GetPoint, &PlaneShapeComponent::SetPoint)
    .Field("Normal", &PlaneShapeComponent::GetNormal, &PlaneShapeComponent::SetNormal)
    .End();
}

} // namespace DAVA
