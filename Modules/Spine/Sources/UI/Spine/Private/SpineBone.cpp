#include "UI/Spine/Private/SpineBone.h"

#include <Debug/DVAssert.h>

#include <spine/spine.h>

namespace DAVA
{
SpineBone::SpineBone(spBone* bone)
    : bonePtr(bone)
{
    DVASSERT(bonePtr);
}

Vector2 SpineBone::GetPosition() const
{
    return Vector2(bonePtr->worldX, -bonePtr->worldY);
}

Vector2 SpineBone::GetScale() const
{
    return Vector2(bonePtr->scaleX, bonePtr->scaleY);
}

float32 SpineBone::GetAngle() const
{
    return float32(-bonePtr->rotation);
}
}