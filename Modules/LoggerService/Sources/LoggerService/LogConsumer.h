#pragma once

#include <Functional/Signal.h>
#include <Network/NetService.h>

namespace DAVA
{
class File;

namespace Net
{
/*
 This is a simple log consumer: each log message is treated as string
*/
class LogConsumer : public NetService
{
public:
    LogConsumer() = default;
    ~LogConsumer() override = default;

    LogConsumer(const LogConsumer&) = delete;
    LogConsumer& operator=(const LogConsumer&) = delete;

    //NetService method implementation
    void OnPacketReceived(const std::shared_ptr<IChannel>& channel, const void* buffer, size_t length) override;

    Signal<const String&> newDataNotifier;
};

} // namespace Net
} // namespace DAVA
