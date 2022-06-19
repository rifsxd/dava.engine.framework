#include "REPlatform/Commands/EntityLockCommand.h"

#include <Scene3D/Entity.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
EntityLockCommand::EntityLockCommand(Entity* _entity, bool lock)
    : RECommand("Lock entity")
    , entity(_entity)
    , newState(lock)
{
    DVASSERT(NULL != entity);
    oldState = entity->GetLocked();
}

EntityLockCommand::~EntityLockCommand()
{
}

void EntityLockCommand::Undo()
{
    DVASSERT(NULL != entity);
    entity->SetLocked(oldState);
}

void EntityLockCommand::Redo()
{
    DVASSERT(NULL != entity);
    entity->SetLocked(newState);
}

Entity* EntityLockCommand::GetEntity() const
{
    return entity;
}

DAVA_VIRTUAL_REFLECTION_IMPL(EntityLockCommand)
{
    ReflectionRegistrator<EntityLockCommand>::Begin()
    .End();
}
} // namespace DAVA
