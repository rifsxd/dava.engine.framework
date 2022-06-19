#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
class Entity;
class SoundEvent;
class SoundComponent;
class AddSoundEventCommand : public RECommand
{
public:
    AddSoundEventCommand(Entity* entity, SoundEvent* sEvent);
    ~AddSoundEventCommand();

    void Undo() override;
    void Redo() override;

    Entity* GetEntity() const;

private:
    Entity* entity;
    SoundEvent* savedEvent;

    DAVA_VIRTUAL_REFLECTION(AddSoundEventCommand, RECommand);
};

class RemoveSoundEventCommand : public RECommand
{
public:
    RemoveSoundEventCommand(Entity* entity, SoundEvent* sEvent);
    ~RemoveSoundEventCommand();

    void Undo() override;
    void Redo() override;

    Entity* GetEntity() const;

private:
    Entity* entity;
    SoundEvent* savedEvent;

    DAVA_VIRTUAL_REFLECTION(AddSoundEventCommand, RECommand);
};

class SetSoundEventFlagsCommand : public RECommand
{
public:
    SetSoundEventFlagsCommand(Entity* entity, uint32 eventIndex, uint32 flags);
    ~SetSoundEventFlagsCommand();

    void Undo() override;
    void Redo() override;

    Entity* GetEntity() const;

private:
    Entity* entity;
    SoundComponent* affectComponent;

    uint32 index;
    uint32 oldFlags;
    uint32 newFlags;

    DAVA_VIRTUAL_REFLECTION(AddSoundEventCommand, RECommand);
};
} // namespace DAVA
