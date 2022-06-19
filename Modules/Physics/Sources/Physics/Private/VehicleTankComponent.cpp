#include "Physics/VehicleTankComponent.h"

#include <Scene3D/Entity.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
void VehicleTankComponent::SetAnalogAcceleration(float32 value)
{
    DVASSERT(value >= 0.0f && value <= 1.0f);
    analogAcceleration = value;
}

void VehicleTankComponent::SetAnalogLeftThrust(float32 value)
{
    DVASSERT(value >= -1.0f && value <= 1.0f);
    analogLeftThrust = value;
}

void VehicleTankComponent::SetAnalogRightThrust(float32 value)
{
    DVASSERT(value >= -1.0f && value <= 1.0f);
    analogRightThrust = value;
}

void VehicleTankComponent::SetAnalogLeftBrake(float32 value)
{
    DVASSERT(value >= 0.0f && value <= 1.0f);
    analogLeftBrake = value;
}

void VehicleTankComponent::SetAnalogRightBrake(float32 value)
{
    DVASSERT(value >= 0.0f && value <= 1.0f);
    analogRightBrake = value;
}

void VehicleTankComponent::ResetInputData()
{
    analogAcceleration = 0.0f;
    analogLeftThrust = 0.0f;
    analogRightThrust = 0.0f;
    analogLeftBrake = 0.0f;
    analogRightBrake = 0.0f;
}

Component* VehicleTankComponent::Clone(Entity* toEntity)
{
    VehicleTankComponent* result = new VehicleTankComponent();
    result->SetEntity(toEntity);

    return result;
}

DAVA_VIRTUAL_REFLECTION_IMPL(VehicleTankComponent)
{
    ReflectionRegistrator<VehicleTankComponent>::Begin()
    .ConstructorByPointer()
    .End();
}
}