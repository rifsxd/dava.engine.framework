#include "REPlatform/Commands/Private/RECommandStack.h"
#include "REPlatform/Commands/CommandAction.h"
#include "REPlatform/Commands/RECommand.h"
#include "REPlatform/Commands/RECommandBatch.h"
#include "REPlatform/Commands/RECommandNotificationObject.h"

#include <Command/Command.h>
#include <memory>

namespace DAVA
{
RECommandStack::RECommandStack()
    : CommandStack()
{
    commandExecuted.Connect(this, &RECommandStack::OnCommandExecuted);
}

RECommandStack::~RECommandStack() = default;

void RECommandStack::Clear()
{
    commands.clear();
    cleanIndex = EMPTY_INDEX;
    SetCurrentIndex(EMPTY_INDEX);
}

void RECommandStack::Exec(std::unique_ptr<Command>&& command)
{
    RECommandNotificationObject notifyObject;
    notifyObject.command = command.get();
    notifyObject.redo = true;

    REDependentCommandsHolder holder(notifyObject);
    AccumulateDependentCommands(holder);

    bool hasDependentCommands = (holder.preCommands.empty() == false) || (holder.postCommands.empty() == false);
    bool singleCommandBatch = (commandBatch == nullptr) && hasDependentCommands;

    if (singleCommandBatch == true)
    {
        BeginBatch(command->GetDescription(), 1);
    }

    for (std::unique_ptr<Command>& cmd : holder.preCommands)
    {
        Exec(std::move(cmd));
    }

    CommandStack::Exec(std::move(command));

    for (std::unique_ptr<Command>& cmd : holder.postCommands)
    {
        Exec(std::move(cmd));
    }

    if (singleCommandBatch == true)
    {
        EndBatch();
    }
}

bool RECommandStack::IsClean() const
{
    return CommandStack::IsClean() && forceChanged == false;
}

void RECommandStack::SetClean()
{
    forceChanged = false;
    CommandStack::SetClean();
}

void RECommandStack::SetChanged()
{
    CommandStack::SetCleanState(false);
    forceChanged = true;
}

CommandBatch* RECommandStack::CreateCommmandBatch(const String& name, uint32 commandsCount) const
{
    return new RECommandBatch(name, commandsCount);
}

void RECommandStack::RemoveCommand(uint32 index)
{
    DVASSERT(index < commands.size());
    if (cleanIndex > static_cast<int32>(index))
    {
        cleanIndex--;
    }
    commands.erase(commands.begin() + index);
    if (currentIndex > static_cast<int32>(index))
    {
        SetCurrentIndex(currentIndex - 1);
    }
}

void RECommandStack::OnCommandExecuted(const Command* command, bool redo)
{
    RECommandNotificationObject notification;
    notification.command = static_cast<const RECommand*>(command);
    notification.redo = redo;
    EmitNotify(notification);
}

void RECommandStack::ExecInternal(std::unique_ptr<Command>&& command, bool isSingleCommand)
{
    if (command->Cast<CommandAction>() != nullptr)
    {
        //get ownership of the given command;
        std::unique_ptr<Command> commandAction(std::move(command));
        commandAction->Redo();
        OnCommandExecuted(commandAction.get(), true);

        if (!commandAction->IsClean())
        {
            SetChanged();
        }
    }
    else
    {
        CommandStack::ExecInternal(std::move(command), isSingleCommand);
    }
}
} // namespace DAVA
