#pragma once

#include <Command/Command.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class RECommand : public Command
{
public:
    RECommand(const String& description = "");

    DAVA_VIRTUAL_REFLECTION(RECommand, Command);
};
} // namespace DAVA
