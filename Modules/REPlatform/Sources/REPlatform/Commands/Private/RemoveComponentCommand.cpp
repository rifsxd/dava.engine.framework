#include "REPlatform/Commands/RemoveComponentCommand.h"

#include <Scene3D/Entity.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
RemoveComponentCommand::RemoveComponentCommand(Entity* _entity, Component* _component)
    : RECommand("Remove Component")
    , entity(_entity)
    , component(_component)
{
    DVASSERT(entity);
    DVASSERT(component);
}

RemoveComponentCommand::~RemoveComponentCommand()
{
    SafeDelete(backup);
}

void RemoveComponentCommand::Redo()
{
    backup = component;
    entity->DetachComponent(component);
}

void RemoveComponentCommand::Undo()
{
    entity->AddComponent(backup);
    backup = nullptr;
}

Entity* RemoveComponentCommand::GetEntity() const
{
    return entity;
}

const Component* RemoveComponentCommand::GetComponent() const
{
    return component;
}

DAVA_VIRTUAL_REFLECTION_IMPL(RemoveComponentCommand)
{
    ReflectionRegistrator<RemoveComponentCommand>::Begin()
    .End();
}
} // namespace DAVA