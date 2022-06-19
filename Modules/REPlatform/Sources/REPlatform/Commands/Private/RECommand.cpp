#include "REPlatform/Commands/RECommand.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedTypeDB.h>

namespace DAVA
{
RECommand::RECommand(const String& description_)
    : Command(description_)
{
}

DAVA_VIRTUAL_REFLECTION_IMPL(RECommand)
{
    ReflectionRegistrator<RECommand>::Begin()
    .End();
}
} // namespace DAVA
