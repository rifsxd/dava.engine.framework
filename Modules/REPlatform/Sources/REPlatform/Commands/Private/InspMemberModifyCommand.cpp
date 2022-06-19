#include "REPlatform/Commands/InspMemberModifyCommand.h"

#include <Base/Introspection.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
InspMemberModifyCommand::InspMemberModifyCommand(const InspMember* _member, void* _object, const VariantType& _newValue)
    : RECommand("Modify value")
    , member(_member)
    , object(_object)
    , newValue(_newValue)
{
    if (nullptr != member && nullptr != object)
    {
        oldValue = member->Value(object);
    }
}

void InspMemberModifyCommand::Undo()
{
    if (nullptr != member && nullptr != object)
    {
        member->SetValue(object, oldValue);
    }
}

void InspMemberModifyCommand::Redo()
{
    if (nullptr != member && nullptr != object)
    {
        member->SetValue(object, newValue);
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(InspMemberModifyCommand)
{
    ReflectionRegistrator<InspMemberModifyCommand>::Begin()
    .End();
}
} // namespace DAVA
