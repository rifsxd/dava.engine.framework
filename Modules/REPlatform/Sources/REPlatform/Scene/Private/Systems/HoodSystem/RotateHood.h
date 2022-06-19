#pragma once

#include "REPlatform/Scene/Private/Systems/HoodSystem/HoodObject.h"

#define ROTATE_HOOD_CIRCLE_PARTS_COUNT 5

namespace DAVA
{
struct RotateHood : public HoodObject
{
    static std::unique_ptr<RotateHood> Create();

    HoodCollObject* axisXLine = nullptr;
    HoodCollObject* axisYLine = nullptr;
    HoodCollObject* axisZLine = nullptr;

    HoodCollObject* axisXc[ROTATE_HOOD_CIRCLE_PARTS_COUNT];
    HoodCollObject* axisYc[ROTATE_HOOD_CIRCLE_PARTS_COUNT];
    HoodCollObject* axisZc[ROTATE_HOOD_CIRCLE_PARTS_COUNT];

    float32 modifRotate;

private:
    RotateHood();

    void Draw(ST_Axis selectedAxis, ST_Axis mouseOverAxis, DAVA::RenderHelper* drawer, TextDrawSystem* textDrawSystem) override;
    void CreateHoodCollisionObjects() override;

    float32 radius;
};
} // namespace DAVA
