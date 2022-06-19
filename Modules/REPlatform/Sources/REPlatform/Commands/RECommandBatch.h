#pragma once

#include "REPlatform/Commands/Private/CommandMatchHelper.h"

#include <Base/BaseTypes.h>
#include <Command/CommandBatch.h>

namespace DAVA
{
class RECommandBatch final : public CommandBatch
{
public:
    RECommandBatch(const String& description, uint32 commandsCount);

    template <typename... Args>
    void RemoveCommands();

    DAVA_VIRTUAL_REFLECTION(RECommandBatch, CommandBatch);
};

template <typename... Args>
void RECommandBatch::RemoveCommands()
{
    auto it = std::remove_if(commandList.begin(), commandList.end(), [](const std::unique_ptr<Command>& cmd) {
        return CommandMatchHelper::IsMatch<Args...>(cmd.get());
    });

    commandList.erase(it, commandList.end());

    for (const std::unique_ptr<Command>& command : commandList)
    {
        RECommandBatch* batch = command->Cast<RECommandBatch>();
        if (batch != nullptr)
        {
            batch->RemoveCommands<Args...>();
        }
    }
}
} // namespace DAVA
