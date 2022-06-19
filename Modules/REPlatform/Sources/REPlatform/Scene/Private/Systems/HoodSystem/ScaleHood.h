#pragma once

#include "REPlatform/Scene/Private/Systems/HoodSystem/HoodObject.h"

namespace DAVA
{
struct ScaleHood : public HoodObject
{
    static std::unique_ptr<ScaleHood> Create();

    HoodCollObject* axisXLine = nullptr;
    HoodCollObject* axisYLine = nullptr;
    HoodCollObject* axisZLine = nullptr;

    HoodCollObject* axisXYLine = nullptr;
    HoodCollObject* axisXZLine = nullptr;
    HoodCollObject* axisYZLine = nullptr;

    float32 modifScale;

private:
    ScaleHood();

    void CreateHoodCollisionObjects() override;
    void Draw(ST_Axis selectedAxis, ST_Axis mouseOverAxis, DAVA::RenderHelper* drawer, TextDrawSystem* textDrawSystem) override;
};
} // namespace DAVA
