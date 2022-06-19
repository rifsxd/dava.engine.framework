#include "REplatform/Commands/CommandNotify.h"

namespace DAVA
{
CommandNotifyProvider::~CommandNotifyProvider()
{
    SafeRelease(curNotify);
}

void CommandNotifyProvider::SetNotify(CommandNotify* notify)
{
    if (curNotify != notify)
    {
        SafeRelease(curNotify);
        curNotify = SafeRetain(notify);
    }
}

void CommandNotifyProvider::AccumulateDependentCommands(REDependentCommandsHolder& holder)
{
    if (nullptr != curNotify)
    {
        curNotify->AccumulateDependentCommands(holder);
    }
}

void CommandNotifyProvider::EmitNotify(const RECommandNotificationObject& commandNotification)
{
    if (nullptr != curNotify)
    {
        curNotify->Notify(commandNotification);
    }
}
} // namespace DAVA
