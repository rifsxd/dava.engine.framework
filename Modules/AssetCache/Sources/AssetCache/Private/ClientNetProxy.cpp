#include "AssetCache/ClientNetProxy.h"
#include "AssetCache/AssetCacheConstants.h"
#include "AssetCache/CachedItemValue.h"
#include "AssetCache/CachePacket.h"

#include <NetworkHelpers/ResolverCallbackDispatched.h>

#include <FileSystem/KeyedArchive.h>
#include <Debug/DVAssert.h>
#include <Logger/Logger.h>
#include <FileSystem/DynamicMemoryFile.h>

namespace DAVA
{
namespace AssetCache
{
ClientNetProxy::ClientNetProxy(Dispatcher<Function<void()>>* dispatcher)
    : dispatcher(dispatcher)
{
    DVASSERT(nullptr != Net::NetCore::Instance());
}

ClientNetProxy::~ClientNetProxy()
{
    Disconnect();
}

void ClientNetProxy::Connect(const String& ip, uint16 port)
{
    Logger::FrameworkDebug("Connecting to %s:%d", ip.c_str(), port);
    DVASSERT(nullptr == netClient);
    DVASSERT(nullptr == addressResolver);
    DVASSERT(nullptr == openedChannel);

    addressResolver.reset(new Net::AddressResolver(Net::NetCore::Instance()->Loop()));
    Net::ResolverCallbackDispatched resolverCallbackDispatched(dispatcher, addressResolver, MakeFunction(this, &ClientNetProxy::OnAddressResolved));
    addressResolver->AsyncResolve(ip.c_str(), port, resolverCallbackDispatched);
}

void ClientNetProxy::Disconnect()
{
    openedChannel = nullptr;

    if (addressResolver)
    {
        addressResolver->Cancel();
        addressResolver.reset();
    }
    if (netClient)
    {
        netClient->Disconnect();
        netClient.reset();
    }
}

void ClientNetProxy::DisconnectBlocked()
{
    openedChannel = nullptr;

    if (addressResolver)
    {
        addressResolver->Cancel();
        addressResolver.reset();
    }
    if (netClient)
    {
        netClient->DisconnectBlocked();
        netClient.reset();
    }
}

void ClientNetProxy::OnAddressResolved(const Net::Endpoint& endpoint, int32 status)
{
    DVASSERT(!netClient);
    DVASSERT(nullptr == openedChannel);

    if (0 == status)
    {
        netClient = Connection::MakeConnection(dispatcher, Net::CLIENT_ROLE, endpoint, this);
    }
    else
    {
        Logger::Error("[ClientNetProxy::OnAddressResolved] address cannot resolved with error %d", status);
    }
}

bool ClientNetProxy::RequestServerStatus()
{
    if (openedChannel)
    {
        //Logger::FrameworkDebug("Requesting cache server status");
        StatusRequestPacket packet;
        return packet.SendTo(openedChannel);
    }

    return false;
}

bool ClientNetProxy::RequestAddNextChunk(const CacheItemKey& key, uint64 dataSize, uint32 numOfChunks, uint32 chunkNumber, const Vector<uint8>& chunkData)
{
    if (openedChannel)
    {
        //Logger::FrameworkDebug("Requesting to add next chunk");
        AddChunkRequestPacket packet(key, dataSize, numOfChunks, chunkNumber, chunkData);
        return packet.SendTo(openedChannel);
    }

    return false;
}

bool ClientNetProxy::RequestGetNextChunk(const CacheItemKey& key, uint32 chunkNumber)
{
    //Logger::FrameworkDebug("Requesting chunk #%u", chunkNumber);
    if (openedChannel)
    {
        GetChunkRequestPacket packet(key, chunkNumber);
        return packet.SendTo(openedChannel);
    }

    return false;
}

bool ClientNetProxy::RequestWarmingUp(const CacheItemKey& key)
{
    //Logger::FrameworkDebug("Requesting warmup");
    if (openedChannel)
    {
        WarmupRequestPacket packet(key);
        return packet.SendTo(openedChannel);
    }

    return false;
}

bool ClientNetProxy::RequestRemoveData(const CacheItemKey& key)
{
    //Logger::FrameworkDebug("Requesting to remove data entry");
    if (openedChannel)
    {
        RemoveRequestPacket packet(key);
        return packet.SendTo(openedChannel);
    }

    return false;
}

bool ClientNetProxy::RequestClearCache()
{
    //Logger::FrameworkDebug("Requesting to clear cache");
    if (openedChannel)
    {
        ClearRequestPacket packet;
        return packet.SendTo(openedChannel);
    }

    return false;
}

void ClientNetProxy::OnChannelOpen(const std::shared_ptr<DAVA::Net::IChannel>& channel)
{
    Logger::FrameworkDebug("Connection established");
    DVASSERT(openedChannel == nullptr);
    openedChannel = channel;
    StateChanged();
}

void ClientNetProxy::OnChannelClosed(const std::shared_ptr<DAVA::Net::IChannel>& channel, const char8*)
{
    Logger::FrameworkDebug("Connection closed");
    DVASSERT(openedChannel == channel);
    openedChannel = nullptr;
    StateChanged();
}

void ClientNetProxy::StateChanged()
{
    for (auto& listener : listeners)
    {
        listener->OnClientProxyStateChanged();
    }
}

void ClientNetProxy::OnPacketReceived(const std::shared_ptr<DAVA::Net::IChannel>& channel, const void* packetData, size_t length)
{
    if (listeners.empty())
    { // do not need to process data in case of nullptr listener
        return;
    }

    IncorrectPacketType incorrectType = IncorrectPacketType::UNDEFINED_DATA;

    DVASSERT(openedChannel == channel);
    if (length > 0)
    {
        std::unique_ptr<CachePacket> packet;
        CachePacket::CreateResult createResult = CachePacket::Create(static_cast<const uint8*>(packetData), static_cast<uint32>(length), packet);

        if (createResult == CachePacket::CREATED)
        {
            DVASSERT(packet);

            switch (packet->type)
            {
            case PACKET_ADD_RESPONSE:
            {
                AddResponsePacket* p = static_cast<AddResponsePacket*>(packet.get());
                //Logger::FrameworkDebug("Response is received: data %s added to cache", p->added ? "is" : "is not");
                for (ClientNetProxyListener* listener : listeners)
                    listener->OnAddedToCache(p->key, p->added);
                return;
            }
            case PACKET_GET_CHUNK_RESPONSE:
            {
                GetChunkResponsePacket* p = static_cast<GetChunkResponsePacket*>(packet.get());
                //Logger::FrameworkDebug("Chunk %u is received", p->chunkNumber);
                for (ClientNetProxyListener* listener : listeners)
                    listener->OnReceivedFromCache(p->key, p->dataSize, p->numOfChunks, p->chunkNumber, p->chunkData);
                return;
            }
            case PACKET_STATUS_RESPONSE:
            {
                //Logger::FrameworkDebug("Response is received: server status is OK");
                for (ClientNetProxyListener* listener : listeners)
                    listener->OnServerStatusReceived();
                return;
            }
            case PACKET_REMOVE_RESPONSE:
            {
                RemoveResponsePacket* p = static_cast<RemoveResponsePacket*>(packet.get());
                //Logger::FrameworkDebug("Response is received: data %s removed from cache", p->removed ? "is" : "is not");
                for (ClientNetProxyListener* listener : listeners)
                    listener->OnRemovedFromCache(p->key, p->removed);
                return;
            }
            case PACKET_CLEAR_RESPONSE:
            {
                ClearResponsePacket* p = static_cast<ClearResponsePacket*>(packet.get());
                //Logger::FrameworkDebug("Response is received: cache %s cleared", p->cleared ? "is" : "is not");
                for (ClientNetProxyListener* listener : listeners)
                    listener->OnCacheCleared(p->cleared);
                return;
            }
            default:
            {
                Logger::Error("%s: Unexpected packet type: %d", __FUNCTION__, packet->type);
                incorrectType = IncorrectPacketType::UNEXPECTED_PACKET;
                break;
            }
            }
        }
        else
        {
            incorrectType = (createResult == CachePacket::ERR_UNSUPPORTED_VERSION) ? IncorrectPacketType::UNSUPPORTED_VERSION : IncorrectPacketType::UNDEFINED_DATA;
        }
    }
    else
    {
        Logger::Error("%s: Empty packet is received", __FUNCTION__);
        incorrectType = IncorrectPacketType::UNDEFINED_DATA;
    }

    for (ClientNetProxyListener* listener : listeners)
        listener->OnIncorrectPacketReceived(incorrectType);
}

void ClientNetProxy::OnPacketSent(const std::shared_ptr<Net::IChannel>& channel, const void* buffer, size_t length)
{
    CachePacket::PacketSent(static_cast<const uint8*>(buffer), length);
}

void ClientNetProxy::AddListener(ClientNetProxyListener* listener)
{
    DVASSERT(listener != nullptr);
    listeners.insert(listener);
}

void ClientNetProxy::RemoveListener(ClientNetProxyListener* listener)
{
    DVASSERT(listener != nullptr);
    listeners.erase(listener);
}

} // end of namespace AssetCache
} // end of namespace DAVA
