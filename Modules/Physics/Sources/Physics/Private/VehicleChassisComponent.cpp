#include "Physics/VehicleChassisComponent.h"

#include <Scene3D/Entity.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
Vector3 VehicleChassisComponent::GetCenterOfMassOffset() const
{
    return centerOfMassOffset;
}

void VehicleChassisComponent::SetCenterOfMassOffset(const Vector3& value)
{
    centerOfMassOffset = value;
}

void VehicleChassisComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
    archive->SetVector3("vehicleChassis.centerOfMassOffset", centerOfMassOffset);
}

void VehicleChassisComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);
    centerOfMassOffset = archive->GetVector3("vehicleChassis.centerOfMassOffset", Vector3::Zero);
}

Component* VehicleChassisComponent::Clone(Entity* toEntity)
{
    VehicleChassisComponent* result = new VehicleChassisComponent();
    result->SetCenterOfMassOffset(centerOfMassOffset);
    result->SetEntity(toEntity);

    return result;
}

DAVA_VIRTUAL_REFLECTION_IMPL(VehicleChassisComponent)
{
    ReflectionRegistrator<VehicleChassisComponent>::Begin()
    .ConstructorByPointer()
    .Field("Center of mass offset", &VehicleChassisComponent::GetCenterOfMassOffset, &VehicleChassisComponent::SetCenterOfMassOffset)[M::Range(Any(), Any(), Vector3::Zero)]
    .End();
}
}