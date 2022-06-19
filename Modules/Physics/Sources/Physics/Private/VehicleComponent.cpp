#include "Physics/VehicleComponent.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
void VehicleComponent::SetGear(eVehicleGears value)
{
    gear = value;
}

DAVA_VIRTUAL_REFLECTION_IMPL(VehicleComponent)
{
    ReflectionRegistrator<VehicleComponent>::Begin()
    .End();
}
}