#pragma once

#include "REPlatform/Commands/RECommand.h"

namespace DAVA
{
class Entity;
class Component;
class RemoveComponentCommand : public RECommand
{
public:
    RemoveComponentCommand(Entity* entity, Component* component);
    ~RemoveComponentCommand() override;

    void Undo() override;
    void Redo() override;

    Entity* GetEntity() const;
    const Component* GetComponent() const;

private:
    Entity* entity = nullptr;
    Component* component = nullptr;
    Component* backup = nullptr;

    DAVA_VIRTUAL_REFLECTION(RemoveComponentCommand, RECommand);
};
} // namespace DAVA
