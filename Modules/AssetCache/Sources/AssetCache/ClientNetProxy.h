#pragma once

#include "AssetCache/Connection.h"
#include "AssetCache/CacheItemKey.h"

#include <Base/BaseTypes.h>
#include <Network/IChannel.h>
#include <Network/Base/AddressResolver.h>

namespace DAVA
{
namespace AssetCache
{
class CachedItemValue;

enum class IncorrectPacketType
{
    UNDEFINED_DATA,
    UNSUPPORTED_VERSION,
    UNEXPECTED_PACKET
};

class ClientNetProxyListener
{
public:
    virtual ~ClientNetProxyListener() = default;

    virtual void OnClientProxyStateChanged(){};
    virtual void OnAddedToCache(const CacheItemKey& key, bool added){};
    virtual void OnReceivedFromCache(const CacheItemKey& key, uint64 dataSize, uint32 numOfChunks, uint32 chunkNumber, const Vector<uint8>& chunkData){};
    virtual void OnRemovedFromCache(const CacheItemKey& key, bool removed){};
    virtual void OnCacheCleared(bool cleared){};
    virtual void OnServerStatusReceived(){};
    virtual void OnIncorrectPacketReceived(IncorrectPacketType){};
};

class ClientNetProxy : public DAVA::Net::IChannelListener
{
public:
    ClientNetProxy(Dispatcher<Function<void()>>*);
    ~ClientNetProxy();

    void AddListener(ClientNetProxyListener*);
    void RemoveListener(ClientNetProxyListener*);

    void Connect(const String& ip, uint16 port);
    void Disconnect();
    void DisconnectBlocked();

    bool ChannelIsOpened() const;

    // requests to sent on server
    bool RequestServerStatus();
    bool RequestAddNextChunk(const CacheItemKey& key, uint64 dataSize, uint32 numOfChunks, uint32 chunkNumber, const Vector<uint8>& chunkData);
    bool RequestGetNextChunk(const CacheItemKey& key, uint32 chunkNumber);
    bool RequestWarmingUp(const CacheItemKey& key);
    bool RequestRemoveData(const CacheItemKey& key);
    bool RequestClearCache();

    Connection* GetConnection() const;

    //Net::IChannelListener
    // Channel is open (underlying transport has connection) and can receive and send data through IChannel interface
    void OnChannelOpen(const std::shared_ptr<Net::IChannel>& channel) override;
    // Channel is closed (underlying transport has disconnected) with reason
    void OnChannelClosed(const std::shared_ptr<Net::IChannel>& channel, const char8* message) override;
    // Some data arrived into channel
    void OnPacketReceived(const std::shared_ptr<Net::IChannel>& channel, const void* buffer, size_t length) override;
    // Buffer has been sent and can be reused or freed
    void OnPacketSent(const std::shared_ptr<Net::IChannel>& channel, const void* buffer, size_t length) override;
    // Data packet with given ID has been delivered to other side
    void OnPacketDelivered(const std::shared_ptr<Net::IChannel>& channel, uint32 packetId) override{};

    void OnAddressResolved(const Net::Endpoint& endpoint, int32 status);

    void StateChanged();

private:
    Dispatcher<Function<void()>>* dispatcher = nullptr;
    std::shared_ptr<Net::AddressResolver> addressResolver;
    std::shared_ptr<Connection> netClient;
    std::shared_ptr<Net::IChannel> openedChannel;

    Set<ClientNetProxyListener*> listeners;
};

inline bool ClientNetProxy::ChannelIsOpened() const
{
    return (openedChannel != nullptr);
}

inline Connection* ClientNetProxy::GetConnection() const
{
    return netClient.get();
}

}; // end of namespace AssetCache
}; // end of namespace DAVA
