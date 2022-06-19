#pragma once

#include "REPlatform/Scene/Private/Systems/HoodSystem/HoodObject.h"

namespace DAVA
{
struct NormalHood : public HoodObject
{
    static std::unique_ptr<NormalHood> Create();

    void Draw(ST_Axis selectedAxis, ST_Axis mouseOverAxis, DAVA::RenderHelper* drawer, TextDrawSystem* textDrawSystem) override;

    HoodCollObject* axisXLine = nullptr;
    HoodCollObject* axisYLine = nullptr;
    HoodCollObject* axisZLine = nullptr;

private:
    NormalHood();
    void CreateHoodCollisionObjects() override;
};
} // namespace DAVA
