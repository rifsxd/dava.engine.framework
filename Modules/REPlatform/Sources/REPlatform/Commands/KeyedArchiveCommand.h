#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <FileSystem/VariantType.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class KeyedArchive;
class KeyedArchiveAddValueCommand : public RECommand
{
public:
    KeyedArchiveAddValueCommand(KeyedArchive* _archive, const String& _key, const VariantType& _val);
    ~KeyedArchiveAddValueCommand();

    void Undo() override;
    void Redo() override;

    KeyedArchive* archive = nullptr;
    String key;
    VariantType val;

private:
    DAVA_VIRTUAL_REFLECTION(KeyedArchiveAddValueCommand, RECommand);
};

class KeyeadArchiveRemValueCommand : public RECommand
{
public:
    KeyeadArchiveRemValueCommand(KeyedArchive* _archive, const String& _key);
    ~KeyeadArchiveRemValueCommand();

    void Undo() override;
    void Redo() override;

    KeyedArchive* archive = nullptr;
    String key;
    VariantType val;

private:
    DAVA_VIRTUAL_REFLECTION(KeyeadArchiveRemValueCommand, RECommand);
};

class KeyeadArchiveSetValueCommand : public RECommand
{
public:
    KeyeadArchiveSetValueCommand(KeyedArchive* _archive, const String& _key, const VariantType& _val);
    ~KeyeadArchiveSetValueCommand();

    void Undo() override;
    void Redo() override;

    KeyedArchive* archive = nullptr;
    String key;
    VariantType oldVal;
    VariantType newVal;

private:
    DAVA_VIRTUAL_REFLECTION(KeyeadArchiveSetValueCommand, RECommand);
};
} // namespace DAVA