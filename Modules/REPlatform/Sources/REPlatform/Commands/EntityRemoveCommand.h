#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
class Entity;
class EntityRemoveCommand : public RECommand
{
public:
    EntityRemoveCommand(Entity* entity);
    ~EntityRemoveCommand();

    void Undo() override;
    void Redo() override;
    Entity* GetEntity() const;

    Entity* entity;
    Entity* before;
    Entity* parent;

private:
    DAVA_VIRTUAL_REFLECTION(EntityRemoveCommand, RECommand);
};
} // namespace DAVA
