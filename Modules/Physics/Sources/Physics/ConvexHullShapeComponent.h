#pragma once

#include "Physics/CollisionShapeComponent.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
class ConvexHullShapeComponent : public CollisionShapeComponent
{
public:
    Component* Clone(Entity* toEntity) override;

protected:
#if defined(__DAVAENGINE_DEBUG__)
    void CheckShapeType() const override;
#endif

    void UpdateLocalProperties() override;

private:
    DAVA_VIRTUAL_REFLECTION(ConvexHullShapeComponent, CollisionShapeComponent);
};
} // namespace DAVA