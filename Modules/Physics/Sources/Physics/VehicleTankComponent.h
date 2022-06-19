#pragma once

#include "Physics/VehicleComponent.h"

#include <Entity/Component.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class Entity;

class VehicleTankComponent final : public VehicleComponent
{
public:
    void SetAnalogAcceleration(float32 value);
    void SetAnalogLeftThrust(float32 value);
    void SetAnalogRightThrust(float32 value);
    void SetAnalogLeftBrake(float32 value);
    void SetAnalogRightBrake(float32 value);

private:
    void ResetInputData();

    virtual Component* Clone(Entity* toEntity) override;

    DAVA_VIRTUAL_REFLECTION(VehicleTankComponent, VehicleComponent);

private:
    friend class PhysicsVehiclesSubsystem;

    float32 analogAcceleration = 0.0f;
    float32 analogLeftThrust = 0.0f;
    float32 analogRightThrust = 0.0f;
    float32 analogRightBrake = 0.0f;
    float32 analogLeftBrake = 0.0f;
};
}