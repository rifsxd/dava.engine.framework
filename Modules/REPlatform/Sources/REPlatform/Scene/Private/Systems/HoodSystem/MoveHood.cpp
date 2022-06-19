#include "REPlatform/Scene/Private/Systems/HoodSystem/MoveHood.h"
#include "REPlatform/Scene/Systems/TextDrawSystem.h"

// framework
#include <Render/RenderHelper.h>

namespace DAVA
{
std::unique_ptr<MoveHood> MoveHood::Create()
{
    std::unique_ptr<MoveHood> object(new MoveHood);
    object->CreateHoodCollisionObjects();
    return object;
}

MoveHood::MoveHood()
    : HoodObject(4.0f)
{
}

void MoveHood::CreateHoodCollisionObjects()
{
    float32 b = baseSize / 5;
    float32 c = baseSize / 3;

    axisXLine = CreateLine(axisX * b, axisX * baseSize);
    axisXLine->axis = ST_AXIS_X;

    axisYLine = CreateLine(axisY * b, axisY * baseSize);
    axisYLine->axis = ST_AXIS_Y;

    axisZLine = CreateLine(axisZ * b, axisZ * baseSize);
    axisZLine->axis = ST_AXIS_Z;

    axisXY1Line = CreateLine(axisX * c, (axisX + axisY) * c);
    axisXY1Line->axis = ST_AXIS_XY;

    axisXY2Line = CreateLine(axisY * c, (axisX + axisY) * c);
    axisXY2Line->axis = ST_AXIS_XY;

    axisXZ1Line = CreateLine(axisX * c, (axisX + axisZ) * c);
    axisXZ1Line->axis = ST_AXIS_XZ;

    axisXZ2Line = CreateLine(axisZ * c, (axisX + axisZ) * c);
    axisXZ2Line->axis = ST_AXIS_XZ;

    axisYZ1Line = CreateLine(axisY * c, (axisY + axisZ) * c);
    axisYZ1Line->axis = ST_AXIS_YZ;

    axisYZ2Line = CreateLine(axisZ * c, (axisY + axisZ) * c);
    axisYZ2Line->axis = ST_AXIS_YZ;
}

void MoveHood::Draw(ST_Axis selectedAxis, ST_Axis mouseOverAxis, RenderHelper* drawer, TextDrawSystem* textDrawSystem)
{
    Color colorSBlend(colorS.r, colorS.g, colorS.b, 0.3f);
    Vector3 curPos = axisXLine->curPos;

    // arrow length
    float32 arrowLen = axisXLine->curScale * baseSize / 4;

    // arrow x
    drawer->DrawArrow(axisXLine->curFrom, axisXLine->curTo, arrowLen, colorX, RenderHelper::DRAW_SOLID_NO_DEPTH);

    // arrow y
    drawer->DrawArrow(axisYLine->curFrom, axisYLine->curTo, arrowLen, colorY, RenderHelper::DRAW_SOLID_NO_DEPTH);

    // arrow z
    drawer->DrawArrow(axisZLine->curFrom, axisZLine->curTo, arrowLen, colorZ, RenderHelper::DRAW_SOLID_NO_DEPTH);

    // x
    if (selectedAxis & ST_AXIS_X)
        drawer->DrawLine(axisXLine->curFrom, axisXLine->curTo, colorS, RenderHelper::DRAW_WIRE_NO_DEPTH);
    else
        drawer->DrawLine(axisXLine->curFrom, axisXLine->curTo, colorX, RenderHelper::DRAW_WIRE_NO_DEPTH);

    // y
    if (selectedAxis & ST_AXIS_Y)
        drawer->DrawLine(axisYLine->curFrom, axisYLine->curTo, colorS, RenderHelper::DRAW_WIRE_NO_DEPTH);
    else
        drawer->DrawLine(axisYLine->curFrom, axisYLine->curTo, colorY, RenderHelper::DRAW_WIRE_NO_DEPTH);

    // z
    if (selectedAxis & ST_AXIS_Z)
        drawer->DrawLine(axisZLine->curFrom, axisZLine->curTo, colorS, RenderHelper::DRAW_WIRE_NO_DEPTH);
    else
        drawer->DrawLine(axisZLine->curFrom, axisZLine->curTo, colorZ, RenderHelper::DRAW_WIRE_NO_DEPTH);

    // xy
    if (selectedAxis == ST_AXIS_XY)
    {
        drawer->DrawLine(axisXY1Line->curFrom, axisXY1Line->curTo, colorS, RenderHelper::DRAW_WIRE_NO_DEPTH);
        drawer->DrawLine(axisXY2Line->curFrom, axisXY2Line->curTo, colorS, RenderHelper::DRAW_WIRE_NO_DEPTH);

        Polygon3 poly;
        poly.AddPoint(curPos);
        poly.AddPoint(axisXY1Line->curFrom);
        poly.AddPoint(axisXY1Line->curTo);
        poly.AddPoint(axisXY2Line->curFrom);
        drawer->DrawPolygon(poly, colorSBlend, RenderHelper::DRAW_SOLID_NO_DEPTH);
    }
    else
    {
        drawer->DrawLine(axisXY1Line->curFrom, axisXY1Line->curTo, colorX, RenderHelper::DRAW_WIRE_NO_DEPTH);
        drawer->DrawLine(axisXY2Line->curFrom, axisXY2Line->curTo, colorY, RenderHelper::DRAW_WIRE_NO_DEPTH);
    }

    // xz
    if (selectedAxis == ST_AXIS_XZ)
    {
        drawer->DrawLine(axisXZ1Line->curFrom, axisXZ1Line->curTo, colorS, RenderHelper::DRAW_WIRE_NO_DEPTH);
        drawer->DrawLine(axisXZ2Line->curFrom, axisXZ2Line->curTo, colorS, RenderHelper::DRAW_WIRE_NO_DEPTH);

        Polygon3 poly;
        poly.AddPoint(curPos);
        poly.AddPoint(axisXZ1Line->curFrom);
        poly.AddPoint(axisXZ1Line->curTo);
        poly.AddPoint(axisXZ2Line->curFrom);
        drawer->DrawPolygon(poly, colorSBlend, RenderHelper::DRAW_SOLID_NO_DEPTH);
    }
    else
    {
        drawer->DrawLine(axisXZ1Line->curFrom, axisXZ1Line->curTo, colorX, RenderHelper::DRAW_WIRE_NO_DEPTH);
        drawer->DrawLine(axisXZ2Line->curFrom, axisXZ2Line->curTo, colorX, RenderHelper::DRAW_WIRE_NO_DEPTH);
    }

    // yz
    if (selectedAxis == ST_AXIS_YZ)
    {
        drawer->DrawLine(axisYZ1Line->curFrom, axisYZ1Line->curTo, colorS, RenderHelper::DRAW_WIRE_NO_DEPTH);
        drawer->DrawLine(axisYZ2Line->curFrom, axisYZ2Line->curTo, colorS, RenderHelper::DRAW_WIRE_NO_DEPTH);

        Polygon3 poly;
        poly.AddPoint(curPos);
        poly.AddPoint(axisYZ1Line->curFrom);
        poly.AddPoint(axisYZ1Line->curTo);
        poly.AddPoint(axisYZ2Line->curFrom);
        drawer->DrawPolygon(poly, colorSBlend, RenderHelper::DRAW_SOLID_NO_DEPTH);
    }
    else
    {
        drawer->DrawLine(axisYZ1Line->curFrom, axisYZ1Line->curTo, colorY, RenderHelper::DRAW_WIRE_NO_DEPTH);
        drawer->DrawLine(axisYZ2Line->curFrom, axisYZ2Line->curTo, colorZ, RenderHelper::DRAW_WIRE_NO_DEPTH);
    }

    Rect r = DrawAxisText(textDrawSystem, axisXLine, axisYLine, axisZLine);

    if (!modifOffset.IsZero())
    {
        char tmp[255];
        Vector2 topPos = Vector2((r.x + r.dx) / 2, r.y - 20);

        sprintf(tmp, "[%.2f, %.2f, %.2f]", modifOffset.x, modifOffset.y, modifOffset.z);
        textDrawSystem->DrawText(topPos, tmp, Color(1.0f, 1.0f, 0.0f, 1.0f));
    }
}
} // namespace DAVA
