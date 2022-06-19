#include "REPlatform/Commands/EntityRemoveCommand.h"

#include <Scene3D/Entity.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
EntityRemoveCommand::EntityRemoveCommand(Entity* _entity)
    : RECommand("Remove entity")
    , entity(_entity)
    , before(nullptr)
    , parent(nullptr)
{
    SafeRetain(entity);

    if (nullptr != entity)
    {
        parent = entity->GetParent();
        if (nullptr != parent)
        {
            before = parent->GetNextChild(entity);
        }
    }
}

EntityRemoveCommand::~EntityRemoveCommand()
{
    SafeRelease(entity);
}

void EntityRemoveCommand::Undo()
{
    if (nullptr != entity && nullptr != parent)
    {
        if (nullptr != before)
        {
            parent->InsertBeforeNode(entity, before);
        }
        else
        {
            parent->AddNode(entity);
        }
    }
}

void EntityRemoveCommand::Redo()
{
    if (nullptr != entity && nullptr != parent)
    {
        parent->RemoveNode(entity);
    }
}

Entity* EntityRemoveCommand::GetEntity() const
{
    return entity;
}

DAVA_VIRTUAL_REFLECTION_IMPL(EntityRemoveCommand)
{
    ReflectionRegistrator<EntityRemoveCommand>::Begin()
    .End();
}
} // namespace DAVA
