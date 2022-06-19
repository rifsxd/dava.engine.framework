#pragma once

#include "Physics/CollisionShapeComponent.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
class MeshShapeComponent : public CollisionShapeComponent
{
public:
    Component* Clone(Entity* toEntity) override;

protected:
#if defined(__DAVAENGINE_DEBUG__)
    void CheckShapeType() const override;
#endif

    void UpdateLocalProperties() override;

private:
    DAVA_VIRTUAL_REFLECTION(MeshShapeComponent, CollisionShapeComponent);
};
} // namespace DAVA