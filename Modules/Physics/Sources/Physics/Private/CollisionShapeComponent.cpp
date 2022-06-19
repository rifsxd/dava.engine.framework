#include "Physics/CollisionShapeComponent.h"
#include "Physics/Private/PhysicsMath.h"
#include "Physics/PhysicsSystem.h"

#include <Scene3D/Scene.h>
#include <Reflection/ReflectionRegistrator.h>

#include <physx/PxShape.h>

namespace DAVA
{
namespace CollisionShapeComponentDetails
{
enum
{
    // additional flags of shape that are used in physx shader function for collision detection
    // values are mapped on physx::PxFilterData::word0
    CCD_FLAG = 1 // first bit of physx::PxFilterData::word0 signals is CCD enabled for shape of not
};
} // namespace CollisionShapeComponentDetail

void CollisionShapeComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
    archive->SetFastName("shape.name", name);
    archive->SetMatrix4("shape.localPose", localPose);
    archive->SetBool("shape.overrideMass", overrideMass);
    archive->SetFloat("shape.mass", mass);
    archive->SetUInt32("shape.typeMask", typeMask);
    archive->SetUInt32("shape.typeMaskToCollideWith", typeMaskToCollideWith);
    if (materialName.IsValid())
    {
        archive->SetFastName("shape.materialName", materialName);
    }
}

void CollisionShapeComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);
    name = archive->GetFastName("shape.name", FastName(""));
    localPose = archive->GetMatrix4("shape.localPose");
    overrideMass = archive->GetBool("shape.overrideMass", overrideMass);
    mass = archive->GetFloat("shape.mass", mass);
    typeMask = archive->GetUInt32("shape.typeMask", typeMask);
    typeMaskToCollideWith = archive->GetUInt32("shape.typeMaskToCollideWith", typeMaskToCollideWith);
    materialName = archive->GetFastName("shape.materialName", materialName);
}

physx::PxShape* CollisionShapeComponent::GetPxShape() const
{
    return shape;
}

const FastName& CollisionShapeComponent::GetName() const
{
    return name;
}

void CollisionShapeComponent::SetName(const FastName& name_)
{
    DVASSERT(name_.IsValid());
    name = name_;
    ScheduleUpdate();
}

const Matrix4& CollisionShapeComponent::GetLocalPose() const
{
    return localPose;
}

void CollisionShapeComponent::SetLocalPose(const Matrix4& localPose_)
{
    localPose = localPose_;
    ScheduleUpdate();
}

bool CollisionShapeComponent::GetOverrideMass() const
{
    return overrideMass;
}

void CollisionShapeComponent::SetOverrideMass(bool override)
{
    overrideMass = override;
    ScheduleUpdate();
}

float32 CollisionShapeComponent::GetMass() const
{
    return mass;
}

void CollisionShapeComponent::SetMass(float32 mass_)
{
    if (overrideMass == true)
    {
        mass = mass_;
        ScheduleUpdate();
    }
}

void CollisionShapeComponent::SetTypeMask(uint32 typeMask_)
{
    if (typeMask != typeMask_)
    {
        typeMask = typeMask_;
        ScheduleUpdate();
    }
}

uint32 CollisionShapeComponent::GetTypeMask() const
{
    return typeMask;
}

void CollisionShapeComponent::SetTypeMaskToCollideWith(uint32 typeMaskToCollideWith_)
{
    if (typeMaskToCollideWith != typeMaskToCollideWith_)
    {
        typeMaskToCollideWith = typeMaskToCollideWith_;
        ScheduleUpdate();
    }
}

uint32 CollisionShapeComponent::GetTypeMaskToCollideWith() const
{
    return typeMaskToCollideWith;
}

const FastName& CollisionShapeComponent::GetMaterialName() const
{
    return materialName;
}

void CollisionShapeComponent::SetMaterialName(const FastName& materialName_)
{
    if (materialName != materialName_)
    {
        materialName = materialName_;
        ScheduleUpdate();
    }
}

CollisionShapeComponent* CollisionShapeComponent::GetComponent(physx::PxShape* shape)
{
    DVASSERT(shape != nullptr);
    return reinterpret_cast<CollisionShapeComponent*>(shape->userData);
}

void CollisionShapeComponent::SetCCDEnabled(physx::PxShape* shape, bool isCCDActive)
{
    DVASSERT(shape != nullptr);
    physx::PxFilterData fd = shape->getSimulationFilterData();
    physx::PxU32 ccdFlag = static_cast<physx::PxU32>(CollisionShapeComponentDetails::CCD_FLAG);
    if (isCCDActive == true)
    {
        fd.word0 |= ccdFlag;
    }
    else
    {
        fd.word0 = fd.word0 & (~ccdFlag);
    }
    shape->setSimulationFilterData(fd);
}

bool CollisionShapeComponent::IsCCDEnabled(const physx::PxFilterData& filterData)
{
    using namespace CollisionShapeComponentDetails;
    return (filterData.word0 & CCD_FLAG) == CCD_FLAG;
}

void CollisionShapeComponent::SetPxShape(physx::PxShape* shape_)
{
    DVASSERT(shape_ != nullptr);
    DVASSERT(shape == nullptr);
    shape = shape_;
    shape->userData = this;
    DVASSERT(name.IsValid());
    shape->setName(name.c_str());
    shape->setLocalPose(physx::PxTransform(PhysicsMath::Matrix4ToPxMat44(localPose)));
    shape->acquireReference();

#if defined(__DAVAENGINE_DEBUG__)
    CheckShapeType();
#endif

    ScheduleUpdate();
}

void CollisionShapeComponent::CopyFieldsIntoClone(CollisionShapeComponent* component) const
{
    component->name = name;
    component->localPose = localPose;
    component->overrideMass = overrideMass;
    component->mass = mass;
    component->typeMask = typeMask;
    component->typeMaskToCollideWith = typeMaskToCollideWith;
    component->materialName = materialName;
}

void CollisionShapeComponent::ScheduleUpdate()
{
    if (shape != nullptr)
    {
        Entity* entity = GetEntity();
        DVASSERT(entity != nullptr);
        Scene* scene = entity->GetScene();
        DVASSERT(scene != nullptr);
        scene->physicsSystem->ScheduleUpdate(this);
    }
}

void CollisionShapeComponent::UpdateLocalProperties()
{
    DVASSERT(shape != nullptr);
    shape->setName(name.c_str());
    shape->setLocalPose(physx::PxTransform(PhysicsMath::Matrix4ToPxMat44(localPose)));
    if (overrideMass == false)
    {
        physx::PxMassProperties massProperties = physx::PxRigidBodyExt::computeMassPropertiesFromShapes(&shape, 1);
        mass = massProperties.mass;
    }

    physx::PxFilterData filterData = shape->getSimulationFilterData();
    filterData.word1 = typeMask;
    filterData.word2 = typeMaskToCollideWith;
    // be careful and do not change first bit in filterData.word0 (CCD flag) as this flag is setting
    // directly from PhysicsSystem::UpdateComponents and should be synchronized with CCD flag of actor.
    shape->setSimulationFilterData(filterData);
}

void CollisionShapeComponent::ReleasePxShape()
{
    DVASSERT(shape != nullptr);
    shape->release();
    shape = nullptr;
}

DAVA_VIRTUAL_REFLECTION_IMPL(CollisionShapeComponent)
{
    ReflectionRegistrator<CollisionShapeComponent>::Begin()
    .Field("Name", &CollisionShapeComponent::GetName, &CollisionShapeComponent::SetName)
    .Field("material", &CollisionShapeComponent::GetMaterialName, &CollisionShapeComponent::SetMaterialName)[M::DisplayName("Material name")]
    .Field("Local pose", &CollisionShapeComponent::localPose)
    .Field("Override mass", &CollisionShapeComponent::GetOverrideMass, &CollisionShapeComponent::SetOverrideMass)
    .Field("Mass", &CollisionShapeComponent::GetMass, &CollisionShapeComponent::SetMass)
    .Field("Type", &CollisionShapeComponent::GetTypeMask, &CollisionShapeComponent::SetTypeMask)
    .Field("Types to collide with", &CollisionShapeComponent::GetTypeMaskToCollideWith, &CollisionShapeComponent::SetTypeMaskToCollideWith)
    .End();
}

} // namespace DAVA