#include "REPlatform/Scene/Private/Systems/HoodSystem/NormalHood.h"
#include "REPlatform/Scene/Systems/TextDrawSystem.h"

// framework
#include <Render/RenderHelper.h>

namespace DAVA
{
std::unique_ptr<NormalHood> NormalHood::Create()
{
    std::unique_ptr<NormalHood> object(new NormalHood);
    object->CreateHoodCollisionObjects();
    return object;
}

NormalHood::NormalHood()
    : HoodObject(2.0f)
{
}

void NormalHood::CreateHoodCollisionObjects()
{
    axisXLine = CreateLine(Vector3(0, 0, 0), axisX * baseSize);
    axisXLine->axis = ST_AXIS_X;

    axisYLine = CreateLine(Vector3(0, 0, 0), axisY * baseSize);
    axisYLine->axis = ST_AXIS_Y;

    axisZLine = CreateLine(Vector3(0, 0, 0), axisZ * baseSize);
    axisZLine->axis = ST_AXIS_Z;
}

void NormalHood::Draw(ST_Axis selectedAxis, ST_Axis mouseOverAxis, RenderHelper* drawer, TextDrawSystem* textDrawSystem)
{
    // x
    drawer->DrawLine(axisXLine->curFrom, axisXLine->curTo, colorX, RenderHelper::DRAW_WIRE_NO_DEPTH);

    // y
    drawer->DrawLine(axisYLine->curFrom, axisYLine->curTo, colorY, RenderHelper::DRAW_WIRE_NO_DEPTH);

    // z
    drawer->DrawLine(axisZLine->curFrom, axisZLine->curTo, colorZ, RenderHelper::DRAW_WIRE_NO_DEPTH);

    DrawAxisText(textDrawSystem, axisXLine, axisYLine, axisZLine);
}
} // namespace DAVA
