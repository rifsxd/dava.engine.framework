#pragma once

#include "REPlatform/Scene/SceneTypes.h"

// bullet
#include <bullet/btBulletCollisionCommon.h>

// framework
#include <Base/BaseMath.h>
#include <Base/BaseTypes.h>

namespace DAVA
{
struct HoodCollObject
{
    HoodCollObject();
    ~HoodCollObject();

    btCollisionObject* btObject;
    btCollisionShape* btShape;

    Vector3 baseFrom;
    Vector3 baseTo;
    Vector3 baseOffset;
    Matrix4 baseRotate;
    Vector3 scaledOffset;

    Vector3 curFrom;
    Vector3 curTo;

    Vector3 curPos;
    float32 curScale;

    ST_Axis axis;

    void UpdatePos(const Vector3& pos);
    void UpdateScale(const float32& scale);
};
} // namespace DAVA
