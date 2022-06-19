#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
class Entity;
class Component;

class AddComponentCommand : public RECommand
{
public:
    AddComponentCommand(Entity* entity, Component* component);
    ~AddComponentCommand() override;

    void Undo() override;
    void Redo() override;

    Entity* GetEntity() const;
    Component* GetComponent() const;

private:
    Entity* entity = nullptr;
    Component* component = nullptr;
    Component* backup = nullptr;

    DAVA_VIRTUAL_REFLECTION(AddComponentCommand, RECommand);
};
} // namespace DAVA
