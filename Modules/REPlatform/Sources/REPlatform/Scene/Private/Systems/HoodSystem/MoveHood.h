#pragma once

#include "REPlatform/Scene/Private/Systems/HoodSystem/HoodObject.h"

namespace DAVA
{
struct MoveHood : public HoodObject
{
    static std::unique_ptr<MoveHood> Create();

    Vector3 modifOffset;

    HoodCollObject* axisXLine = nullptr;
    HoodCollObject* axisYLine = nullptr;
    HoodCollObject* axisZLine = nullptr;

    HoodCollObject* axisXY1Line = nullptr;
    HoodCollObject* axisXY2Line = nullptr;
    HoodCollObject* axisXZ1Line = nullptr;
    HoodCollObject* axisXZ2Line = nullptr;
    HoodCollObject* axisYZ1Line = nullptr;
    HoodCollObject* axisYZ2Line = nullptr;

private:
    MoveHood();

    void CreateHoodCollisionObjects() override;
    void Draw(ST_Axis selectedAxis, ST_Axis mouseOverAxis, DAVA::RenderHelper* drawer, TextDrawSystem* textDrawSystem) override;
};
} // namespace DAVA
