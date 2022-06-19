#pragma once

#include "REPlatform/Commands/CommandNotify.h"
#include "REPlatform/Commands/Private/CommandMatchHelper.h"
#include "REPlatform/Commands/RECommandBatch.h"

#include <Base/BaseTypes.h>
#include <Command/CommandStack.h>

namespace DAVA
{
class Command;
class RECommandStack : public CommandStack, public CommandNotifyProvider
{
public:
    RECommandStack();
    ~RECommandStack() override;

    void Exec(std::unique_ptr<Command>&& command) override;
    bool IsClean() const override;
    void SetClean() override;

    void Clear();
    void SetChanged();
    template <typename... Args>
    void RemoveCommands();

    template <typename... Args>
    bool IsUncleanCommandExists() const;

private:
    CommandBatch* CreateCommmandBatch(const String& name, uint32 commandsCount) const override;

    void RemoveCommand(uint32 index);

    void OnCommandExecuted(const Command* cmd, bool redo);
    void ExecInternal(std::unique_ptr<Command>&& command, bool isSingleCommand) override;

    bool forceChanged = false;
};

template <typename... Args>
void RECommandStack::RemoveCommands()
{
    for (int32 index = static_cast<int32>(commands.size() - 1); index >= 0; --index)
    {
        Command* commandPtr = commands[index].get();
        RECommandBatch* batch = commandPtr->Cast<RECommandBatch>();
        if (batch != nullptr)
        {
            batch->RemoveCommands<Args...>();
            if (batch->IsEmpty())
            {
                RemoveCommand(index);
            }
        }
        else if (CommandMatchHelper::IsMatch<Args...>(commandPtr))
        {
            RemoveCommand(index);
        }
    }
}

template <typename... Args>
bool RECommandStack::IsUncleanCommandExists() const
{
    uint32 size = static_cast<uint32>(commands.size());
    for (uint32 index = std::max(cleanIndex, 0); index < size; ++index)
    {
        const Command* commandPtr = commands.at(index).get();
        const RECommandBatch* batch = commandPtr->Cast<RECommandBatch>();
        if (batch != nullptr)
        {
            for (uint32 i = 0; i < batch->Size(); ++i)
            {
                if (CommandMatchHelper::IsMatch<Args...>(batch->GetCommand(i)) == true)
                {
                    return true;
                }
            }
        }
        else if (CommandMatchHelper::IsMatch<Args...>(commandPtr) == true)
        {
            return true;
        }
    }

    return false;
}
} // namespace DAVA
