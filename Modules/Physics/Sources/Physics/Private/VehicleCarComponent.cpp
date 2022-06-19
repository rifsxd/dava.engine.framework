#include "Physics/VehicleCarComponent.h"

#include <Scene3D/Entity.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
void VehicleCarComponent::SetAnalogAcceleration(float32 value)
{
    DVASSERT(value >= 0.0f && value <= 1.0f);
    analogAcceleration = value;
}

void VehicleCarComponent::SetAnalogSteer(float32 value)
{
    DVASSERT(value >= -1.0f && value <= 1.0f);
    analogSteer = value;
}

void VehicleCarComponent::SetAnalogBrake(float32 value)
{
    DVASSERT(value >= 0.0f && value <= 1.0f);
    analogBrake = value;
}

void VehicleCarComponent::ResetInputData()
{
    analogAcceleration = 0.0f;
    analogSteer = 0.0f;
    analogBrake = 0.0f;
}

Component* VehicleCarComponent::Clone(Entity* toEntity)
{
    VehicleComponent* result = new VehicleCarComponent();
    result->SetEntity(toEntity);

    return result;
}

DAVA_VIRTUAL_REFLECTION_IMPL(VehicleCarComponent)
{
    ReflectionRegistrator<VehicleCarComponent>::Begin()
    .ConstructorByPointer()
    .End();
}
}