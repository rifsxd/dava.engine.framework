#include "REPlatform/Commands/RECommandNotificationObject.h"

#include <Command/Command.h>

namespace DAVA
{
bool RECommandNotificationObject::IsEmpty() const
{
    return command == nullptr;
}

bool RECommandNotificationObject::IsRedo() const
{
    return redo;
}

REDependentCommandsHolder::REDependentCommandsHolder(const RECommandNotificationObject& notifyObject_)
    : notifyObject(notifyObject_)
{
}

void REDependentCommandsHolder::AddPreCommand(std::unique_ptr<Command>&& command)
{
    preCommands.push_back(std::move(command));
}

void REDependentCommandsHolder::AddPostCommand(std::unique_ptr<Command>&& command)
{
    postCommands.push_back(std::move(command));
}

const RECommandNotificationObject& REDependentCommandsHolder::GetMasterCommandInfo() const
{
    return notifyObject;
}
} // namespace DAVA
