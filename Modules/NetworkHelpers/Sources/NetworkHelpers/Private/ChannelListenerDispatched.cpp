#include "NetworkHelpers/ChannelListenerDispatched.h"
#include <Functional/Function.h>

namespace DAVA
{
namespace Net
{
ChannelListenerDispatched::ChannelListenerDispatched(std::weak_ptr<IChannelListener> listenerWeak, Dispatcher<Function<void()>>* netEventsDispatcher)
    : netEventsDispatcher(netEventsDispatcher)
    , targetObjectWeak(listenerWeak)
{
}

void ChannelListenerDispatched::OnChannelOpen(const std::shared_ptr<IChannel>& channel)
{
    std::shared_ptr<IChannel> channelCopy = channel;
    std::weak_ptr<IChannelListener> targetObjectWeakCopy = targetObjectWeak;

    auto msg = [channelCopy, targetObjectWeakCopy]()
    {
        std::shared_ptr<IChannelListener> objectShared = targetObjectWeakCopy.lock();
        if (objectShared)
        {
            objectShared->OnChannelOpen(channelCopy);
        }
    };
    netEventsDispatcher->PostEvent(msg);
}

void ChannelListenerDispatched::OnChannelClosed(const std::shared_ptr<IChannel>& channel, const char8* message)
{
    std::shared_ptr<IChannel> channelCopy = channel;
    std::weak_ptr<IChannelListener> targetObjectWeakCopy = targetObjectWeak;

    auto msg = [channelCopy, targetObjectWeakCopy, message]()
    {
        std::shared_ptr<IChannelListener> objectShared = targetObjectWeakCopy.lock();
        if (objectShared)
        {
            objectShared->OnChannelClosed(channelCopy, message);
        }
    };
    netEventsDispatcher->PostEvent(msg);
}

void ChannelListenerDispatched::OnPacketReceived(const std::shared_ptr<IChannel>& channel, const void* buffer, size_t length)
{
    Vector<uint8> bufferCopy(length);
    Memcpy(bufferCopy.data(), buffer, length);

    std::shared_ptr<IChannel> channelCopy = channel;
    std::weak_ptr<IChannelListener> targetObjectWeakCopy = targetObjectWeak;

    auto msg = [channelCopy, targetObjectWeakCopy, bufferCopy]()
    {
        std::shared_ptr<IChannelListener> objectShared = targetObjectWeakCopy.lock();
        if (objectShared)
        {
            objectShared->OnPacketReceived(channelCopy, bufferCopy.data(), bufferCopy.size());
        }
    };
    netEventsDispatcher->PostEvent(msg);
}

void ChannelListenerDispatched::OnPacketSent(const std::shared_ptr<IChannel>& channel, const void* buffer, size_t length)
{
    std::shared_ptr<IChannel> channelCopy = channel;
    std::weak_ptr<IChannelListener> targetObjectWeakCopy = targetObjectWeak;

    auto msg = [channelCopy, targetObjectWeakCopy, buffer, length]()
    {
        std::shared_ptr<IChannelListener> objectShared = targetObjectWeakCopy.lock();
        if (objectShared)
        {
            objectShared->OnPacketSent(channelCopy, buffer, length);
        }
    };
    netEventsDispatcher->PostEvent(msg);
}

void ChannelListenerDispatched::OnPacketDelivered(const std::shared_ptr<IChannel>& channel, uint32 packetId)
{
    std::shared_ptr<IChannel> channelCopy = channel;
    std::weak_ptr<IChannelListener> targetObjectWeakCopy = targetObjectWeak;

    auto msg = [channelCopy, targetObjectWeakCopy, packetId]()
    {
        std::shared_ptr<IChannelListener> objectShared = targetObjectWeakCopy.lock();
        if (objectShared)
        {
            objectShared->OnPacketDelivered(channelCopy, packetId);
        }
    };
    netEventsDispatcher->PostEvent(msg);
}
}
}
