#include "REPlatform/Commands/AddComponentCommand.h"

#include <Scene3D/Entity.h>
#include <Entity/Component.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
AddComponentCommand::AddComponentCommand(Entity* _entity, Component* _component)
    : RECommand("Add Component")
    , entity(_entity)
    , component(_component)
{
    DVASSERT(entity);
    DVASSERT(component);
}

AddComponentCommand::~AddComponentCommand()
{
    SafeDelete(backup);
}

void AddComponentCommand::Redo()
{
    entity->AddComponent(component);
    backup = nullptr;
}

void AddComponentCommand::Undo()
{
    backup = component;
    entity->DetachComponent(component);
}

Entity* AddComponentCommand::GetEntity() const
{
    return entity;
}

Component* AddComponentCommand::GetComponent() const
{
    return component;
}

DAVA_VIRTUAL_REFLECTION_IMPL(AddComponentCommand)
{
    ReflectionRegistrator<AddComponentCommand>::Begin()
    .End();
}
} // namespace DAVA
