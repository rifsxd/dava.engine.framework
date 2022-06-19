#pragma once

#include <Entity/Component.h>
#include <Reflection/Reflection.h>

namespace physx
{
class PxVehicleDrive;
};

namespace DAVA
{
// Values match PxVehicleGearsData gear enum
enum class eVehicleGears : uint32
{
    Reverse = 0,
    Neutral,
    First,
    Second,
    Third,
    Fourth,
    Fifth,
    Sixth,
    Seventh,
    Eighth,
    Ninth,
    Tenth
};

class VehicleComponent : public Component
{
public:
    void SetGear(eVehicleGears value);

private:
    DAVA_VIRTUAL_REFLECTION(VehicleComponent, Component);

private:
    friend class PhysicsVehiclesSubsystem;

    physx::PxVehicleDrive* vehicle = nullptr;

    eVehicleGears gear = eVehicleGears::First;
};
}