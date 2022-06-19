#include "REPlatform/Scene/Private/Systems/HoodSystem/ScaleHood.h"
#include "REPlatform/Scene/Systems/TextDrawSystem.h"

#include <Render/RenderHelper.h>

namespace DAVA
{
std::unique_ptr<ScaleHood> ScaleHood::Create()
{
    std::unique_ptr<ScaleHood> object(new ScaleHood);
    object->CreateHoodCollisionObjects();
    return object;
}

ScaleHood::ScaleHood()
    : HoodObject(4.0f)
    , modifScale(0)
{
}

void ScaleHood::CreateHoodCollisionObjects()
{
    float32 c = 2 * baseSize / 3;

    axisXLine = CreateLine(Vector3(0, 0, 0), axisX * baseSize);
    axisXLine->axis = ST_AXIS_X;

    axisYLine = CreateLine(Vector3(0, 0, 0), axisY * baseSize);
    axisYLine->axis = ST_AXIS_Y;

    axisZLine = CreateLine(Vector3(0, 0, 0), axisZ * baseSize);
    axisZLine->axis = ST_AXIS_Z;

    axisXYLine = CreateLine(axisX * c, axisY * c);
    axisXYLine->axis = ST_AXIS_XY;

    axisXZLine = CreateLine(axisX * c, axisZ * c);
    axisXZLine->axis = ST_AXIS_XZ;

    axisYZLine = CreateLine(axisY * c, axisZ * c);
    axisYZLine->axis = ST_AXIS_YZ;
}

void ScaleHood::Draw(ST_Axis selectedAxis, ST_Axis mouseOverAxis, RenderHelper* drawer, TextDrawSystem* textDrawSystem)
{
    // x
    if (mouseOverAxis)
        drawer->DrawLine(axisXLine->curFrom, axisXLine->curTo, colorS, RenderHelper::DRAW_WIRE_NO_DEPTH);
    else
        drawer->DrawLine(axisXLine->curFrom, axisXLine->curTo, colorX, RenderHelper::DRAW_WIRE_NO_DEPTH);

    // y
    if (mouseOverAxis)
        drawer->DrawLine(axisYLine->curFrom, axisYLine->curTo, colorS, RenderHelper::DRAW_WIRE_NO_DEPTH);
    else
        drawer->DrawLine(axisYLine->curFrom, axisYLine->curTo, colorY, RenderHelper::DRAW_WIRE_NO_DEPTH);

    // z
    if (mouseOverAxis)
        drawer->DrawLine(axisZLine->curFrom, axisZLine->curTo, colorS, RenderHelper::DRAW_WIRE_NO_DEPTH);
    else
        drawer->DrawLine(axisZLine->curFrom, axisZLine->curTo, colorZ, RenderHelper::DRAW_WIRE_NO_DEPTH);

    // xy xz yz axis
    drawer->DrawLine(axisXYLine->curFrom, axisXYLine->curTo, colorS, RenderHelper::DRAW_WIRE_NO_DEPTH);
    drawer->DrawLine(axisXZLine->curFrom, axisXZLine->curTo, colorS, RenderHelper::DRAW_WIRE_NO_DEPTH);
    drawer->DrawLine(axisYZLine->curFrom, axisYZLine->curTo, colorS, RenderHelper::DRAW_WIRE_NO_DEPTH);

    // xy xz yz plane
    if (mouseOverAxis)
    {
        Color colorSBlend(colorS.r, colorS.g, colorS.b, 0.3f);

        Polygon3 poly;
        poly.AddPoint(axisXYLine->curFrom);
        poly.AddPoint(axisXYLine->curTo);
        poly.AddPoint(axisYZLine->curTo);
        drawer->DrawPolygon(poly, colorSBlend, RenderHelper::DRAW_SOLID_NO_DEPTH);
    }

    // draw axis spheres
    float32 boxSize = axisXLine->curScale * baseSize / 12;

    drawer->DrawAABox(AABBox3(axisXLine->curTo, boxSize), colorX, RenderHelper::DRAW_SOLID_NO_DEPTH);

    drawer->DrawAABox(AABBox3(axisYLine->curTo, boxSize), colorY, RenderHelper::DRAW_SOLID_NO_DEPTH);

    drawer->DrawAABox(AABBox3(axisZLine->curTo, boxSize), colorZ, RenderHelper::DRAW_SOLID_NO_DEPTH);

    Rect r = DrawAxisText(textDrawSystem, axisXLine, axisYLine, axisZLine);

    if (0 != modifScale)
    {
        char tmp[255];
        Vector2 topPos = Vector2((r.x + r.dx) / 2, r.y - 20);

        sprintf(tmp, "[%.2f, %.2f, %.2f]", modifScale, modifScale, modifScale);
        textDrawSystem->DrawText(topPos, tmp, Color(1.0f, 1.0f, 0.0f, 1.0f));
    }
}
} // namespace DAVA
