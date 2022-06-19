#pragma once

#include <Base/BaseTypes.h>
#include <Math/Vector.h>

struct spBone;

namespace DAVA
{
/** Public wrapper for Spine bone structure.
Has getters for necessary bone fields.
*/
class SpineBone
{
public:
    /** Constructor with specified pointer to spBone. */
    SpineBone(spBone* bone);

    /** Return position of current bone. */
    Vector2 GetPosition() const;
    /** Return scale of current bone. */
    Vector2 GetScale() const;
    /** Return angle of current bone. */
    float32 GetAngle() const;

private:
    spBone* bonePtr = nullptr;
};
}