#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Reflection/Reflection.h>
#include <Math/Transform.h>

namespace DAVA
{
class Entity;
class EntityParentChangeCommand : public RECommand
{
public:
    EntityParentChangeCommand(Entity* entity, Entity* newParent, bool saveEntityPosition, Entity* newBefore = nullptr);
    ~EntityParentChangeCommand();

    void Undo() override;
    void Redo() override;
    Entity* GetEntity() const;

    Entity* entity;
    Entity* oldParent;
    Entity* oldBefore;
    Entity* newParent;
    Entity* newBefore;
    bool saveEntityPosition;

    Transform oldTransform;
    Transform newTransform;

private:
    DAVA_VIRTUAL_REFLECTION(EntityParentChangeCommand, RECommand);
};
} // namespace DAVA
