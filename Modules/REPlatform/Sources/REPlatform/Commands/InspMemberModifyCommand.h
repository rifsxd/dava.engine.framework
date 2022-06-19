#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <FileSystem/VariantType.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class InspMember;
class InspMemberModifyCommand : public RECommand
{
public:
    InspMemberModifyCommand(const InspMember* member, void* object, const VariantType& value);

    void Undo() override;
    void Redo() override;

    const InspMember* member;
    void* object;

    VariantType oldValue;
    VariantType newValue;

private:
    DAVA_VIRTUAL_REFLECTION(InspMemberModifyCommand, RECommand);
};
} // namespace DAVA
