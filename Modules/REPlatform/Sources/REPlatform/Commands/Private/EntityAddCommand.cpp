#include "REPlatform/Commands/EntityAddCommand.h"

#include <Scene3D/Entity.h>
#include <Scene3D/Components/SwitchComponent.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Utils/StringFormat.h>

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
EntityAddCommand::EntityAddCommand(Entity* entityToAdd, Entity* parentToAdd, Entity* insertBefore)
    : RECommand(Format("Add Entity %s", entityToAdd->GetName().c_str()))
    , entityToAdd(entityToAdd)
    , parentToAdd(parentToAdd)
    , insertBefore(insertBefore)
{
    SafeRetain(entityToAdd);
    DVASSERT(parentToAdd != nullptr);
    DVASSERT(entityToAdd != nullptr);
}

EntityAddCommand::~EntityAddCommand()
{
    SafeRelease(entityToAdd);
}

void EntityAddCommand::Undo()
{
    parentToAdd->RemoveNode(entityToAdd);
}

void EntityAddCommand::Redo()
{
    if (insertBefore != nullptr)
    {
        parentToAdd->InsertBeforeNode(entityToAdd, insertBefore);
    }
    else
    {
        parentToAdd->AddNode(entityToAdd);
    }

    //Workaround for correctly adding of switch
    SwitchComponent* sw = GetSwitchComponent(entityToAdd);
    if (sw != nullptr)
    {
        sw->SetSwitchIndex(sw->GetSwitchIndex());
    }
}

Entity* EntityAddCommand::GetEntity() const
{
    return entityToAdd;
}

DAVA_VIRTUAL_REFLECTION_IMPL(EntityAddCommand)
{
    ReflectionRegistrator<EntityAddCommand>::Begin()
    .End();
}
} // namespace DAVA
