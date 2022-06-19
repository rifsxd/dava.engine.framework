#pragma once

#include "REPlatform/Commands/RECommand.h"
#include <Reflection/Reflection.h>

namespace DAVA
{
class Entity;

class EntityLockCommand : public RECommand
{
public:
    EntityLockCommand(Entity* entity, bool lock);
    ~EntityLockCommand();

    void Undo() override;
    void Redo() override;
    Entity* GetEntity() const;

    Entity* entity;
    bool oldState;
    bool newState;

private:
    DAVA_VIRTUAL_REFLECTION(EntityLockCommand, RECommand);
};
} // namespace DAVA
