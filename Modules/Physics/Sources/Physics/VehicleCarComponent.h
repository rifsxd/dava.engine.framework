#pragma once

#include "Physics/VehicleComponent.h"

#include <Entity/Component.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class Entity;

class VehicleCarComponent final : public VehicleComponent
{
public:
    void SetAnalogAcceleration(float32 value);
    void SetAnalogSteer(float32 value);
    void SetAnalogBrake(float32 value);

private:
    void ResetInputData();

    virtual Component* Clone(Entity* toEntity) override;

    DAVA_VIRTUAL_REFLECTION(VehicleCarComponent, VehicleComponent);

private:
    friend class PhysicsVehiclesSubsystem;

    float32 analogAcceleration = 0.0f;
    float32 analogSteer = 0.0f;
    float32 analogBrake = 0.0f;
};
}