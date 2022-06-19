#include "REPlatform/Commands/KeyedArchiveCommand.h"

#include <FileSystem/KeyedArchive.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
KeyedArchiveAddValueCommand::KeyedArchiveAddValueCommand(KeyedArchive* _archive, const String& _key, const VariantType& _val)
    : RECommand("Add key to archive")
    , archive(_archive)
    , key(_key)
    , val(_val)
{
}

KeyedArchiveAddValueCommand::~KeyedArchiveAddValueCommand()
{
}

void KeyedArchiveAddValueCommand::Undo()
{
    if (nullptr != archive)
    {
        archive->DeleteKey(key);
    }
}

void KeyedArchiveAddValueCommand::Redo()
{
    if (nullptr != archive)
    {
        archive->SetVariant(key, val);
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(KeyedArchiveAddValueCommand)
{
    ReflectionRegistrator<KeyedArchiveAddValueCommand>::Begin()
    .End();
}

KeyeadArchiveRemValueCommand::KeyeadArchiveRemValueCommand(KeyedArchive* _archive, const String& _key)
    : RECommand("Rem key from archive")
    , archive(_archive)
    , key(_key)
{
    if (nullptr != archive)
    {
        VariantType* vPtr = archive->GetVariant(key);

        if (nullptr != vPtr)
        {
            val = *vPtr;
        }
    }
}

KeyeadArchiveRemValueCommand::~KeyeadArchiveRemValueCommand()
{
}

void KeyeadArchiveRemValueCommand::Undo()
{
    if (nullptr != archive)
    {
        archive->SetVariant(key, val);
    }
}

void KeyeadArchiveRemValueCommand::Redo()
{
    if (nullptr != archive)
    {
        archive->DeleteKey(key);
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(KeyeadArchiveRemValueCommand)
{
    ReflectionRegistrator<KeyeadArchiveRemValueCommand>::Begin()
    .End();
}

KeyeadArchiveSetValueCommand::KeyeadArchiveSetValueCommand(KeyedArchive* _archive, const String& _key, const VariantType& _val)
    : RECommand("Set archive value")
    , archive(_archive)
    , key(_key)
    , newVal(_val)
{
    if (nullptr != archive)
    {
        oldVal = *archive->GetVariant(key);
    }
}

KeyeadArchiveSetValueCommand::~KeyeadArchiveSetValueCommand()
{
}

void KeyeadArchiveSetValueCommand::Undo()
{
    if (nullptr != archive && archive->IsKeyExists(key))
    {
        archive->SetVariant(key, oldVal);
    }
}

void KeyeadArchiveSetValueCommand::Redo()
{
    if (nullptr != archive && archive->IsKeyExists(key))
    {
        archive->SetVariant(key, newVal);
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(KeyeadArchiveSetValueCommand)
{
    ReflectionRegistrator<KeyeadArchiveSetValueCommand>::Begin()
    .End();
}
} // namespace DAVA
