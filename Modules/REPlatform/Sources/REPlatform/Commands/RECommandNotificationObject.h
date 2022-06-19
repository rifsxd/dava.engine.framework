#pragma once

#include "REPlatform/Commands/Private/CommandMatchHelper.h"

#include <Command/CommandBatch.h>
#include <Functional/Function.h>
#include <Reflection/ReflectedType.h>

namespace DAVA
{
class Command;
class RECommandNotificationObject
{
public:
    bool IsEmpty() const;

    template <typename... Args>
    bool MatchCommandTypes() const;

    template <typename T>
    void ForEach(const Function<void(const T*)>& callback) const;

    bool IsRedo() const;

private:
    friend class RECommandStack;
    const Command* command = nullptr;
    bool redo = true;
};

template <typename... Args>
bool RECommandNotificationObject::MatchCommandTypes() const
{
    bool result = false;
    const CommandBatch* batch = command->Cast<CommandBatch>();
    if (batch != nullptr)
    {
        for (uint32 i = 0; i < batch->Size(); ++i)
        {
            result = CommandMatchHelper::IsMatch<Args...>(batch->GetCommand(i));
        }
    }
    else
    {
        result = CommandMatchHelper::IsMatch<Args...>(command);
    }

    return result;
}

template <typename T>
void RECommandNotificationObject::ForEach(const Function<void(const T*)>& callback) const
{
    auto unpackCommand = [&](const Command* inputCommand) {
        const T* castResult = inputCommand->Cast<T>();
        if (castResult != nullptr)
        {
            callback(castResult);
            return;
        }

        const CommandBatch* batch = inputCommand->Cast<CommandBatch>();
        if (batch != nullptr)
        {
            for (uint32 i = 0; i < batch->Size(); ++i)
            {
                const T* batchedCommand = batch->GetCommand(i)->Cast<T>();
                if (batchedCommand != nullptr)
                {
                    callback(batchedCommand);
                }
            }
        }
    };

    unpackCommand(command);
}

class REDependentCommandsHolder
{
    friend class RECommandStack;

public:
    REDependentCommandsHolder(const RECommandNotificationObject& notifyObject);

    void AddPreCommand(std::unique_ptr<Command>&& command);
    void AddPostCommand(std::unique_ptr<Command>&& command);
    const RECommandNotificationObject& GetMasterCommandInfo() const;

private:
    RECommandNotificationObject notifyObject;
    Vector<std::unique_ptr<Command>> preCommands;
    Vector<std::unique_ptr<Command>> postCommands;
};
} // namespace DAVA
