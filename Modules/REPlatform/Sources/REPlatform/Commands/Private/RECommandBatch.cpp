#include "REPlatform/Commands/RECommandBatch.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
RECommandBatch::RECommandBatch(const String& description, uint32 commandsCount)
    : CommandBatch(description, commandsCount)
{
}

DAVA_VIRTUAL_REFLECTION_IMPL(RECommandBatch)
{
    ReflectionRegistrator<RECommandBatch>::Begin()
    .End();
}
} // namespace DAVA