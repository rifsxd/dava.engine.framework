#pragma once

#include <ctime>

#include <Base/Noncopyable.h>
#include <Logger/Logger.h>
#include <Time/DateTime.h>
#include <Concurrency/Mutex.h>
#include <Network/NetService.h>

namespace DAVA
{
namespace Net
{
/*
 This is network logger
*/
class NetLogger : public NetService,
                  public LoggerOutput,
                  private Noncopyable
{
private:
    struct LogRecord
    {
        LogRecord()
            : timestamp()
            , level()
            , message()
        {
        }
        LogRecord(time_t tstamp, Logger::eLogLevel ll, const char8* text)
            : timestamp(tstamp)
            , level(ll)
            , message(text)
        {
        }

        time_t timestamp;
        Logger::eLogLevel level;
        String message;
    };

public:
    NetLogger(bool selfInstallFlag = true, size_t queueSize = 100);
    virtual ~NetLogger();

    void Install();
    void Uninstall();
    size_t GetMessageQueueSize() const;

private:
    // IChannelListener
    void OnPacketSent(const std::shared_ptr<IChannel>& channel, const void* buffer, size_t length) override;
    void OnPacketDelivered(const std::shared_ptr<IChannel>& channel, uint32 packetId) override;

    // LoggerOutput
    void Output(Logger::eLogLevel ll, const char8* text) override;

    void ChannelOpen() override;

    void DoOutput(Logger::eLogLevel ll, const char8* text);
    void SendNextRecord();

    bool EnqueueMessage(Logger::eLogLevel ll, const char8* message);
    bool GetFirstMessage(LogRecord& record);
    void RemoveFirstMessage();

    String TimestampToString(time_t timestamp) const;

    bool selfInstall;
    bool isInstalled;
    size_t maxQueueSize;
    mutable Mutex mutex;
    Deque<LogRecord> recordQueue;
};

} // namespace Net
} // namespace DAVA
