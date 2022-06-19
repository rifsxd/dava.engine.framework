#pragma once

#include "REPlatform/Scene/Private/Systems/HoodSystem/HoodCollObject.h"
#include "REPlatform/Scene/SceneTypes.h"

#include <Render/RenderHelper.h>
#include <Render/UniqueStateSet.h>

namespace DAVA
{
class TextDrawSystem;
struct HoodObject
{
    HoodObject(float32 baseSize);
    virtual ~HoodObject() = default;

    float32 baseSize;
    float32 objScale;
    Color colorX; // axis X
    Color colorY; // axis Y
    Color colorZ; // axis Z
    Color colorS; // axis selected

    void SetAxes(const DAVA::Vector3& axisX, const DAVA::Vector3& axisY, const DAVA::Vector3& axisZ);
    virtual void UpdatePos(const DAVA::Vector3& pos);
    virtual void UpdateScale(const DAVA::float32& scale);
    virtual void Draw(ST_Axis selectedAxis, ST_Axis mouseOverAxis, DAVA::RenderHelper* drawer, TextDrawSystem* textDrawSystem) = 0;

    HoodCollObject* CreateLine(const Vector3& from, const Vector3& to);
    Rect DrawAxisText(TextDrawSystem* textDrawSystem, HoodCollObject* x, HoodCollObject* y, HoodCollObject* z);

    Vector<std::unique_ptr<HoodCollObject>> collObjects;

protected:
    virtual void CreateHoodCollisionObjects() = 0;
    Vector3 GetAxisTextPos(HoodCollObject* axis);

    Vector3 axisX = { 1, 0, 0 };
    Vector3 axisY = { 0, 1, 0 };
    Vector3 axisZ = { 0, 0, 1 };
};
} // namespace DAVA
