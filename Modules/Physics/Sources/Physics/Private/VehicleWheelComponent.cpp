#include "Physics/VehicleWheelComponent.h"

#include <Scene3D/Entity.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
float32 VehicleWheelComponent::GetRadius() const
{
    return radius;
}

void VehicleWheelComponent::SetRadius(float32 value)
{
    DVASSERT(value > 0.0f);
    radius = value;
}

float32 VehicleWheelComponent::GetWidth() const
{
    return width;
}

void VehicleWheelComponent::SetWidth(float32 value)
{
    DVASSERT(value > 0.0f);
    width = value;
}

float32 VehicleWheelComponent::GetMaxHandbrakeTorque() const
{
    return maxHandbrakeTorque;
}

void VehicleWheelComponent::SetMaxHandbrakeTorque(float32 value)
{
    DVASSERT(value >= 0.0f);
    maxHandbrakeTorque = value;
}

float32 VehicleWheelComponent::GetMaxSteerAngle() const
{
    return maxSteerAngle;
}

void VehicleWheelComponent::SetMaxSteerAngle(float32 value)
{
    DVASSERT(value >= 0.0f);
    maxSteerAngle = value;
}

void VehicleWheelComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
    archive->SetFloat("vehicleWheel.radius", radius);
    archive->SetFloat("vehicleWheel.width", width);
    archive->SetFloat("vehicleWheel.maxHandbrakeTorque", maxHandbrakeTorque);
    archive->SetFloat("vehicleWheel.maxSteerAngle", maxSteerAngle);
}

void VehicleWheelComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);
    radius = archive->GetFloat("vehicleWheel.radius", 0.5f);
    width = archive->GetFloat("vehicleWheel.width", 0.4f);
    maxHandbrakeTorque = archive->GetFloat("vehicleWheel.maxHandbrakeTorque", 0.0f);
    maxSteerAngle = archive->GetFloat("vehicleWheel.maxSteerAngle", 0.0f);
}

Component* VehicleWheelComponent::Clone(Entity* toEntity)
{
    VehicleWheelComponent* result = new VehicleWheelComponent();
    result->SetRadius(radius);
    result->SetWidth(width);
    result->SetMaxHandbrakeTorque(maxHandbrakeTorque);
    result->SetMaxSteerAngle(maxSteerAngle);
    result->SetEntity(toEntity);

    return result;
}

DAVA_VIRTUAL_REFLECTION_IMPL(VehicleWheelComponent)
{
    ReflectionRegistrator<VehicleWheelComponent>::Begin()
    .ConstructorByPointer()
    .Field("Radius", &VehicleWheelComponent::GetRadius, &VehicleWheelComponent::SetRadius)[M::Range(0.01f, Any(), 0.5f)]
    .Field("Width", &VehicleWheelComponent::GetWidth, &VehicleWheelComponent::SetWidth)[M::Range(0.01f, Any(), 0.4f)]
    .Field("Max handbrake torque", &VehicleWheelComponent::GetMaxHandbrakeTorque, &VehicleWheelComponent::SetMaxHandbrakeTorque)[M::Range(0.0f, Any(), 0.0f)]
    .Field("Max steer angle", &VehicleWheelComponent::GetMaxSteerAngle, &VehicleWheelComponent::SetMaxSteerAngle)[M::Range(0.0f, Any(), 0.0f)]
    .End();
}
}