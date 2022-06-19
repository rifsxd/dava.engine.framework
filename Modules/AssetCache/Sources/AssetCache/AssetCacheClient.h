#pragma once

#include "AssetCache/AssetCache.h"

#include <Base/Introspection.h>
#include <FileSystem/DynamicMemoryFile.h>

#include <atomic>

namespace DAVA
{
class AssetCacheClient final : public AssetCache::ClientNetProxyListener
{
public:
    struct ConnectionParams
    {
        ConnectionParams() = default;
        String ip = AssetCache::GetLocalHost();
        uint16 port = AssetCache::ASSET_SERVER_PORT;
        uint64 timeoutms = 60u * 1000u;
    };

    AssetCacheClient();
    ~AssetCacheClient() override;

    AssetCache::Error ConnectSynchronously(const ConnectionParams& connectionParams);
    void Disconnect();

    AssetCache::Error AddToCacheSynchronously(const AssetCache::CacheItemKey& key, const AssetCache::CachedItemValue& value);
    AssetCache::Error RequestFromCacheSynchronously(const AssetCache::CacheItemKey& key, AssetCache::CachedItemValue* value);
    AssetCache::Error RemoveFromCacheSynchronously(const AssetCache::CacheItemKey& key);
    AssetCache::Error ClearCacheSynchronously();

    uint64 GetTimeoutMs() const;
    bool IsConnected() const;

    void ClearStats();
    void DumpStats() const;

private:
    AssetCache::Error WaitRequest();
    AssetCache::Error CheckStatusSynchronously();
    void ProcessNetwork();

    //ClientNetProxyListener
    void OnAddedToCache(const AssetCache::CacheItemKey& key, bool added) override;
    void OnReceivedFromCache(const AssetCache::CacheItemKey& key, uint64 dataSize, uint32 numOfChunks, uint32 chunkNumber, const Vector<uint8>& chunkData) override;
    void OnRemovedFromCache(const AssetCache::CacheItemKey& key, bool removed) override;
    void OnCacheCleared(bool cleared) override;
    void OnServerStatusReceived() override;
    void OnIncorrectPacketReceived(AssetCache::IncorrectPacketType) override;
    void OnClientProxyStateChanged() override;

private:
    struct Request
    {
        Request() = default;
        Request(AssetCache::ePacketID requestID_)
            : requestID(requestID_)
        {
        }
        Request(AssetCache::ePacketID requestID_, const AssetCache::CacheItemKey& key_, AssetCache::CachedItemValue* value_ = nullptr)
            : key(key_)
            , value(value_)
            , requestID(requestID_)
        {
        }

        void Reset()
        {
            value = nullptr;

            requestID = AssetCache::PACKET_UNKNOWN;
            result = AssetCache::Error::CODE_NOT_INITIALIZED;

            recieved = false;
            processingRequest = false;
        }

        AssetCache::CacheItemKey key;
        AssetCache::CachedItemValue* value = nullptr;

        AssetCache::ePacketID requestID = AssetCache::PACKET_UNKNOWN;
        AssetCache::Error result = AssetCache::Error::NO_ERRORS;

        bool recieved = false;
        bool processingRequest = false;
    };

    Dispatcher<Function<void()>> dispatcher;

    struct GetFilesRequest
    {
        Vector<uint8> receivedData;
        size_t bytesReceived = 0;
        size_t bytesRemaining = 0;
        uint32 chunksReceived = 0;
        uint32 chunksOverall = 0;

        void Reset()
        {
            receivedData.clear();
            bytesReceived = 0;
            bytesRemaining = 0;
            chunksReceived = 0;
        }
    };

    struct AddFilesRequest
    {
        AddFilesRequest()
        {
            serializedData = DynamicMemoryFile::Create(File::CREATE | File::WRITE | File::READ);
            Reset();
        }

        void Reset()
        {
            serializedData->Truncate(0);
            chunksSent = 0;
            chunksOverall = 0;
        }

        ScopedPtr<DynamicMemoryFile> serializedData;
        uint32 chunksSent = 0;
        uint32 chunksOverall = 0;
    };

    struct Stats
    {
        uint32 getRequestsCount = 0;
        uint32 getRequestsFailedCount = 0;
        uint32 getRequestsTimeoutCount = 0;
        uint32 getRequestsSucceedCount = 0;
        uint32 getRequestsNotFoundCount = 0;

        uint32 addRequestsCount = 0;
        uint32 addRequestsFailedCount = 0;
        uint32 addRequestsTimeoutCount = 0;
        uint32 addRequestsSucceedCount = 0;

        uint32 incorrectPacketsCount = 0;
    };

    AssetCache::ClientNetProxy client;

    uint64 timeoutMs = 60u * 1000u;

    Mutex requestLocker;
    Mutex connectEstablishLocker;
    Request request;
    GetFilesRequest getFilesRequest;
    AddFilesRequest addFilesRequest;

    Stats stats;
    std::atomic<bool> isActive;
};

inline uint64 AssetCacheClient::GetTimeoutMs() const
{
    return timeoutMs;
}

} //END of DAVA
