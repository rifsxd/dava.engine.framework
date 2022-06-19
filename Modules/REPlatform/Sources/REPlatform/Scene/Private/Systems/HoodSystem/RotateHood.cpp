#include "REPlatform/Scene/Private/Systems/HoodSystem/RotateHood.h"
#include "REPlatform/Scene/Systems/TextDrawSystem.h"

#include <Render/RenderHelper.h>

namespace DAVA
{
std::unique_ptr<RotateHood> RotateHood::Create()
{
    std::unique_ptr<RotateHood> object(new RotateHood);
    object->CreateHoodCollisionObjects();
    return object;
}

RotateHood::RotateHood()
    : HoodObject(4.0f)
    , modifRotate(0)
{
}

void RotateHood::CreateHoodCollisionObjects()
{
    float32 b = baseSize / 4;
    radius = 2 * baseSize / 3;

    float32 step = PI_05 / ROTATE_HOOD_CIRCLE_PARTS_COUNT;
    float32 x, y, lx = radius, ly = 0;

    axisXLine = CreateLine(axisX * b, axisX * baseSize);
    axisXLine->axis = ST_AXIS_X;
    axisYLine = CreateLine(axisY * b, axisY * baseSize);
    axisYLine->axis = ST_AXIS_Y;
    axisZLine = CreateLine(axisZ * b, axisZ * baseSize);
    axisZLine->axis = ST_AXIS_Z;

    for (int i = 0; i < ROTATE_HOOD_CIRCLE_PARTS_COUNT; ++i)
    {
        float32 angle = step * (i + 1);

        x = radius * cosf(angle);
        y = radius * sinf(angle);

        axisXc[i] = CreateLine(axisY * lx + axisZ * ly, axisY * x + axisZ * y);
        axisXc[i]->axis = ST_AXIS_X;
        axisYc[i] = CreateLine(axisX * lx + axisZ * ly, axisX * x + axisZ * y);
        axisYc[i]->axis = ST_AXIS_Y;
        axisZc[i] = CreateLine(axisX * lx + axisY * ly, axisX * x + axisY * y);
        axisZc[i]->axis = ST_AXIS_Z;

        lx = x;
        ly = y;
    }
}

void RotateHood::Draw(ST_Axis selectedAxis, ST_Axis mouseOverAxis, RenderHelper* drawer, TextDrawSystem* textDrawSystem)
{
    Color colorSBlend(colorS.r, colorS.g, colorS.b, 0.3f);
    Vector3 curPos = axisXLine->curPos;

    Color lineColor = colorX;

    // x
    if (selectedAxis == ST_AXIS_X || selectedAxis == ST_AXIS_YZ)
    {
        if (0 == modifRotate)
        {
            Polygon3 poly;
            poly.AddPoint(curPos);
            for (int i = 0; i < ROTATE_HOOD_CIRCLE_PARTS_COUNT; ++i)
            {
                poly.AddPoint(axisXc[i]->curFrom);
            }
            poly.AddPoint(axisXc[ROTATE_HOOD_CIRCLE_PARTS_COUNT - 1]->curTo);
            drawer->DrawPolygon(poly, colorSBlend, RenderHelper::DRAW_SOLID_NO_DEPTH);
        }
        // draw rotate circle
        else
        {
            float32 step = modifRotate / 24;
            Color modifColor = colorX;
            modifColor.a = 0.3f;

            Polygon3 poly;
            float32 y;
            float32 z;

            poly.AddPoint(curPos);
            for (float32 a = 0; fabs(a) < fabs(modifRotate); a += step)
            {
                y = radius * sinf(a) * objScale;
                z = radius * cosf(a) * objScale;

                poly.AddPoint(Vector3(curPos.x, curPos.y + y, curPos.z + z));
            }

            y = radius * sinf(modifRotate) * objScale;
            z = radius * cosf(modifRotate) * objScale;
            poly.AddPoint(Vector3(curPos.x, curPos.y + y, curPos.z + z));
            drawer->DrawPolygon(poly, modifColor, RenderHelper::DRAW_SOLID_NO_DEPTH);
        }

        lineColor = colorS;
    }

    drawer->DrawLine(axisXLine->curFrom, axisXLine->curTo, lineColor, RenderHelper::DRAW_WIRE_NO_DEPTH);
    for (int i = 0; i < ROTATE_HOOD_CIRCLE_PARTS_COUNT; ++i)
    {
        drawer->DrawLine(axisXc[i]->curFrom, axisXc[i]->curTo, lineColor, RenderHelper::DRAW_WIRE_NO_DEPTH);
    }

    lineColor = colorY;
    // y
    if (selectedAxis == ST_AXIS_Y || selectedAxis == ST_AXIS_XZ)
    {
        if (0 == modifRotate)
        {
            Polygon3 poly;
            poly.AddPoint(curPos);
            for (int i = 0; i < ROTATE_HOOD_CIRCLE_PARTS_COUNT; ++i)
            {
                poly.AddPoint(axisYc[i]->curFrom);
            }
            poly.AddPoint(axisYc[ROTATE_HOOD_CIRCLE_PARTS_COUNT - 1]->curTo);
            drawer->DrawPolygon(poly, colorSBlend, RenderHelper::DRAW_SOLID_NO_DEPTH);
        }
        // draw rotate circle
        else
        {
            float32 step = modifRotate / 24;
            Color modifColor = colorY;
            modifColor.a = 0.3f;

            Polygon3 poly;
            float32 x;
            float32 z;

            poly.AddPoint(curPos);
            for (float32 a = 0; fabs(a) < fabs(modifRotate); a += step)
            {
                x = radius * cosf(a) * objScale;
                z = radius * sinf(a) * objScale;

                poly.AddPoint(Vector3(curPos.x + x, curPos.y, curPos.z + z));
            }

            x = radius * cosf(modifRotate) * objScale;
            z = radius * sinf(modifRotate) * objScale;
            poly.AddPoint(Vector3(curPos.x + x, curPos.y, curPos.z + z));
            drawer->DrawPolygon(poly, modifColor, RenderHelper::DRAW_SOLID_NO_DEPTH);
        }

        lineColor = colorS;
    }

    drawer->DrawLine(axisYLine->curFrom, axisYLine->curTo, lineColor, RenderHelper::DRAW_WIRE_NO_DEPTH);
    for (int i = 0; i < ROTATE_HOOD_CIRCLE_PARTS_COUNT; ++i)
    {
        drawer->DrawLine(axisYc[i]->curFrom, axisYc[i]->curTo, lineColor, RenderHelper::DRAW_WIRE_NO_DEPTH);
    }

    lineColor = colorZ;
    // z
    if (selectedAxis == ST_AXIS_Z || selectedAxis == ST_AXIS_XY)
    {
        if (0 == modifRotate)
        {
            Polygon3 poly;
            poly.AddPoint(curPos);
            for (int i = 0; i < ROTATE_HOOD_CIRCLE_PARTS_COUNT; ++i)
            {
                poly.AddPoint(axisZc[i]->curFrom);
            }
            poly.AddPoint(axisZc[ROTATE_HOOD_CIRCLE_PARTS_COUNT - 1]->curTo);
            drawer->DrawPolygon(poly, colorSBlend, RenderHelper::DRAW_SOLID_NO_DEPTH);
        }
        // draw rotate circle
        else
        {
            float32 step = modifRotate / 24;
            Color modifColor = colorZ;
            modifColor.a = 0.3f;

            Polygon3 poly;
            float32 x;
            float32 y;

            poly.AddPoint(curPos);
            for (float32 a = 0; fabs(a) < fabs(modifRotate); a += step)
            {
                x = radius * sinf(a) * objScale;
                y = radius * cosf(a) * objScale;

                poly.AddPoint(Vector3(curPos.x + x, curPos.y + y, curPos.z));
            }

            x = radius * sinf(modifRotate) * objScale;
            y = radius * cosf(modifRotate) * objScale;
            poly.AddPoint(Vector3(curPos.x + x, curPos.y + y, curPos.z));
            drawer->DrawPolygon(poly, modifColor, RenderHelper::DRAW_SOLID_NO_DEPTH);
        }

        lineColor = colorS;
    }

    drawer->DrawLine(axisZLine->curFrom, axisZLine->curTo, lineColor, RenderHelper::DRAW_WIRE_NO_DEPTH);
    for (int i = 0; i < ROTATE_HOOD_CIRCLE_PARTS_COUNT; ++i)
    {
        drawer->DrawLine(axisZc[i]->curFrom, axisZc[i]->curTo, lineColor, RenderHelper::DRAW_WIRE_NO_DEPTH);
    }

    // draw axis spheres
    float32 radius = axisXLine->curScale * baseSize / 24;

    drawer->DrawIcosahedron(axisXLine->curTo, radius, colorX, RenderHelper::DRAW_SOLID_NO_DEPTH);
    drawer->DrawIcosahedron(axisYLine->curTo, radius, colorY, RenderHelper::DRAW_SOLID_NO_DEPTH);
    drawer->DrawIcosahedron(axisZLine->curTo, radius, colorZ, RenderHelper::DRAW_SOLID_NO_DEPTH);

    Rect r = DrawAxisText(textDrawSystem, axisXLine, axisYLine, axisZLine);

    if (0 != modifRotate)
    {
        char tmp[255];
        tmp[0] = 0;

        if (selectedAxis == ST_AXIS_X || selectedAxis == ST_AXIS_YZ)
        {
            sprintf(tmp, "[%.2f, 0.00, 0.00]", RadToDeg(modifRotate));
        }
        if (selectedAxis == ST_AXIS_Y || selectedAxis == ST_AXIS_XZ)
        {
            sprintf(tmp, "[0.00, %.2f, 0.00]", RadToDeg(modifRotate));
        }
        if (selectedAxis == ST_AXIS_Z || selectedAxis == ST_AXIS_XY)
        {
            sprintf(tmp, "[0.00, 0.00, %.2f]", RadToDeg(modifRotate));
        }

        if (0 != tmp[0])
        {
            Vector2 topPos = Vector2((r.x + r.dx) / 2, r.y - 20);
            textDrawSystem->DrawText(topPos, tmp, Color(1.0f, 1.0f, 0.0f, 1.0f));
        }
    }
}
} // namespace DAVA
