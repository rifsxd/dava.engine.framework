#include "REPlatform/Commands/SetFieldValueCommand.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
SetFieldValueCommand::SetFieldValueCommand(const Reflection::Field& field_, const Any& newValue_)
    : RECommand("")
    , newValue(newValue_)
    , field(field_)
{
    oldValue = field.ref.GetValue();
}

void SetFieldValueCommand::Redo()
{
    field.ref.SetValueWithCast(newValue);
}

void SetFieldValueCommand::Undo()
{
    field.ref.SetValueWithCast(oldValue);
}

const Any& SetFieldValueCommand::GetOldValue() const
{
    return oldValue;
}

const Any& SetFieldValueCommand::GetNewValue() const
{
    return newValue;
}

const Reflection::Field& SetFieldValueCommand::GetField() const
{
    return field;
}

DAVA_VIRTUAL_REFLECTION_IMPL(SetFieldValueCommand)
{
    ReflectionRegistrator<SetFieldValueCommand>::Begin()
    .End();
}
} // namespace DAVA
