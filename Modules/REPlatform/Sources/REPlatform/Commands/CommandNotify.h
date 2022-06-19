#pragma once

#include <Base/BaseObject.h>
#include <Base/BaseTypes.h>

namespace DAVA
{
class RECommandNotificationObject;
class REDependentCommandsHolder;
class CommandNotify : public BaseObject
{
public:
    virtual void AccumulateDependentCommands(REDependentCommandsHolder& holder) = 0;
    virtual void Notify(const RECommandNotificationObject& commandNotification) = 0;
};

class CommandNotifyProvider
{
public:
    virtual ~CommandNotifyProvider();

    void SetNotify(CommandNotify* notify);
    CommandNotify* GetNotify() const;

    void AccumulateDependentCommands(REDependentCommandsHolder& holder);
    void EmitNotify(const RECommandNotificationObject& commandNotification);

protected:
    CommandNotify* curNotify = nullptr;
};

inline CommandNotify* CommandNotifyProvider::GetNotify() const
{
    return curNotify;
}
} // namespace DAVA
