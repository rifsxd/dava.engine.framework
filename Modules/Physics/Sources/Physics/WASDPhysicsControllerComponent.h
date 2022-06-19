#pragma once

#include <Entity/Component.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class WASDPhysicsControllerComponent : public Component
{
public:
    Component* Clone(Entity* toEntity) override;

    DAVA_VIRTUAL_REFLECTION(WASDPhysicsControllerComponent, Component);
};
};
