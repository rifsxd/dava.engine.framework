#include "REPlatform/Commands/SoundComponentEditCommands.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/SoundComponent.h>

namespace DAVA
{
AddSoundEventCommand::AddSoundEventCommand(Entity* _entity, SoundEvent* _event)
    : RECommand("Add Sound Event")
{
    DVASSERT(_entity);
    DVASSERT(_event);

    savedEvent = SafeRetain(_event);
    entity = SafeRetain(_entity);
}

AddSoundEventCommand::~AddSoundEventCommand()
{
    SafeRelease(savedEvent);
    SafeRelease(entity);
}

void AddSoundEventCommand::Redo()
{
    SoundComponent* component = GetSoundComponent(entity);
    DVASSERT(component);
    component->AddSoundEvent(savedEvent);
}

void AddSoundEventCommand::Undo()
{
    savedEvent->Stop();
    SoundComponent* component = GetSoundComponent(entity);
    DVASSERT(component);
    component->RemoveSoundEvent(savedEvent);
}

Entity* AddSoundEventCommand::GetEntity() const
{
    return entity;
}

DAVA_VIRTUAL_REFLECTION_IMPL(AddSoundEventCommand)
{
    ReflectionRegistrator<AddSoundEventCommand>::Begin()
    .End();
}

RemoveSoundEventCommand::RemoveSoundEventCommand(Entity* _entity, SoundEvent* _event)
    : RECommand("Remove Sound Event")
{
    DVASSERT(_entity);
    DVASSERT(_event);

    savedEvent = SafeRetain(_event);
    entity = SafeRetain(_entity);
}

RemoveSoundEventCommand::~RemoveSoundEventCommand()
{
    SafeRelease(savedEvent);
    SafeRelease(entity);
}

void RemoveSoundEventCommand::Redo()
{
    savedEvent->Stop();
    SoundComponent* component = GetSoundComponent(entity);
    DVASSERT(component);
    component->RemoveSoundEvent(savedEvent);
}

void RemoveSoundEventCommand::Undo()
{
    SoundComponent* component = GetSoundComponent(entity);
    DVASSERT(component);
    component->AddSoundEvent(savedEvent);
}

Entity* RemoveSoundEventCommand::GetEntity() const
{
    return entity;
}

DAVA_VIRTUAL_REFLECTION_IMPL(RemoveSoundEventCommand)
{
    ReflectionRegistrator<RemoveSoundEventCommand>::Begin()
    .End();
}

SetSoundEventFlagsCommand::SetSoundEventFlagsCommand(Entity* _entity, uint32 eventIndex, uint32 flags)
    : RECommand("Set Sound Event Flags")
    , index(eventIndex)
    , newFlags(flags)
{
    entity = SafeRetain(_entity);
    DVASSERT(entity);

    affectComponent = GetSoundComponent(entity);
    DVASSERT(affectComponent);

    oldFlags = affectComponent->GetSoundEventFlags(index);
}

SetSoundEventFlagsCommand::~SetSoundEventFlagsCommand()
{
    SafeRelease(entity);
}

void SetSoundEventFlagsCommand::Redo()
{
    affectComponent->SetSoundEventFlags(index, newFlags);
}

void SetSoundEventFlagsCommand::Undo()
{
    affectComponent->SetSoundEventFlags(index, oldFlags);
}

Entity* SetSoundEventFlagsCommand::GetEntity() const
{
    return entity;
}

DAVA_VIRTUAL_REFLECTION_IMPL(SetSoundEventFlagsCommand)
{
    ReflectionRegistrator<SetSoundEventFlagsCommand>::Begin()
    .End();
}
} // namespace DAVA
