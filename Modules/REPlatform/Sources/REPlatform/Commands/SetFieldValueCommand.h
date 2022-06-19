#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Base/Any.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class SetFieldValueCommand : public RECommand
{
public:
    SetFieldValueCommand(const Reflection::Field& field, const Any& newValue);

    void Redo() override;
    void Undo() override;

    const Any& GetOldValue() const;
    const Any& GetNewValue() const;
    const Reflection::Field& GetField() const;

private:
    Any oldValue;
    Any newValue;
    Reflection::Field field;

    DAVA_VIRTUAL_REFLECTION(SetFieldValueCommand, RECommand);
};
} // namespace DAVA
