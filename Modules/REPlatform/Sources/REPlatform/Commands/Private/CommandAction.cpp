#include "REPlatform/Commands/CommandAction.h"

#include <Debug/DVAssert.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
CommandAction::CommandAction(const DAVA::String& text)
    : RECommand(text)
{
}

void CommandAction::Undo()
{
    DVASSERT(false);
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandAction)
{
    ReflectionRegistrator<CommandAction>::Begin()
    .End();
}
} // namespace DAVA