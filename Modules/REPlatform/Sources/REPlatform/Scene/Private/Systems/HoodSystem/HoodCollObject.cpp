#include "REPlatform/Scene/Private/Systems/HoodSystem/HoodCollObject.h"

namespace DAVA
{
HoodCollObject::HoodCollObject()
    : btObject(nullptr)
    , btShape(nullptr)
    , curScale(1.0f)
{
}

HoodCollObject::~HoodCollObject()
{
    if (nullptr != btObject)
    {
        delete btObject;
    }

    if (nullptr != btShape)
    {
        delete btShape;
    }
}

void HoodCollObject::UpdatePos(const Vector3& pos)
{
    curPos = pos;
    curFrom = (baseFrom * curScale) + curPos;
    curTo = (baseTo * curScale) + curPos;

    // move bullet object to new pos
    btTransform transf = btObject->getWorldTransform();
    transf.setOrigin(btVector3(scaledOffset.x + curFrom.x, scaledOffset.y + curFrom.y, scaledOffset.z + curFrom.z));
    btObject->setWorldTransform(transf);
}

void HoodCollObject::UpdateScale(const float32& scale)
{
    curScale = scale;

    btShape->setLocalScaling(btVector3(curScale, curScale, curScale));
    scaledOffset = MultiplyVectorMat3x3(baseOffset * curScale, baseRotate);

    UpdatePos(curPos);
}
} // namespace DAVA
