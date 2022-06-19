#include "AssetCache/AssetCacheClient.h"
#include "AssetCache/ChunkSplitter.h"

#include <FileSystem/FileSystem.h>
#include <Concurrency/LockGuard.h>
#include <Concurrency/Thread.h>
#include <FileSystem/DynamicMemoryFile.h>
#include <Time/SystemTimer.h>
#include <Utils/StringFormat.h>
#include <Logger/Logger.h>
#include <Network/NetCore.h>

namespace DAVA
{
AssetCacheClient::AssetCacheClient()
    : dispatcher([](const Function<void()>& fn) { fn(); })
    , client(&dispatcher)
    , isActive(false)
{
    dispatcher.LinkToCurrentThread();
    client.AddListener(this);
}

AssetCacheClient::~AssetCacheClient()
{
    DumpStats();

    client.RemoveListener(this);

    DVASSERT(isActive == false);
}

AssetCache::Error AssetCacheClient::ConnectSynchronously(const ConnectionParams& connectionParams)
{
    isActive = true;
    timeoutMs = connectionParams.timeoutms;

    client.Connect(connectionParams.ip, AssetCache::ASSET_SERVER_PORT);

    {
        LockGuard<Mutex> guard(connectEstablishLocker);

        uint64 startTime = SystemTimer::GetMs();
        while (client.ChannelIsOpened() == false)
        {
            ProcessNetwork();
            if (!isActive)
            {
                return AssetCache::Error::CANNOT_CONNECT;
            }

            uint64 deltaTime = SystemTimer::GetMs() - startTime;
            if (((timeoutMs > 0) && (deltaTime > timeoutMs)) && (client.ChannelIsOpened() == false))
            {
                Logger::Error("Timeout on connecting to asset cache %s (%lld ms)", connectionParams.ip.c_str(), timeoutMs);
                isActive = false;
                return AssetCache::Error::OPERATION_TIMEOUT;
            }
        }
    }

    return CheckStatusSynchronously();
}

void AssetCacheClient::Disconnect()
{
    isActive = false;

    { // wait for connection establishing loop is finished
        LockGuard<Mutex> guard(connectEstablishLocker);
    }

    client.DisconnectBlocked();
}

AssetCache::Error AssetCacheClient::CheckStatusSynchronously()
{
    {
        LockGuard<Mutex> guard(requestLocker);
        request = Request();
        request.requestID = AssetCache::ePacketID::PACKET_STATUS_REQUEST;
    }

    AssetCache::Error resultCode = AssetCache::Error::CANNOT_SEND_REQUEST;

    bool requestSent = client.RequestServerStatus();
    if (requestSent)
    {
        resultCode = WaitRequest();
    }

    {
        LockGuard<Mutex> guard(requestLocker);
        request.Reset();
    }

    return resultCode;
}

AssetCache::Error AssetCacheClient::AddToCacheSynchronously(const AssetCache::CacheItemKey& key, const AssetCache::CachedItemValue& value)
{
    uint64 dataSizeOverall = 0;
    uint32 chunksOverall = 0;
    {
        LockGuard<Mutex> guard(requestLocker);
        request = Request(AssetCache::PACKET_ADD_CHUNK_REQUEST, key);
        addFilesRequest.Reset();
        value.Serialize(addFilesRequest.serializedData);
        dataSizeOverall = addFilesRequest.serializedData->GetSize();
        chunksOverall = AssetCache::ChunkSplitter::GetNumberOfChunks(dataSizeOverall);
        addFilesRequest.chunksSent = 0;
    }

    AssetCache::Error resultCode = AssetCache::Error::CANNOT_SEND_REQUEST;

    for (uint32 currentChunk = 0; currentChunk < chunksOverall; ++currentChunk)
    {
        Vector<uint8> chunkData;
        {
            LockGuard<Mutex> guard(requestLocker);
            request = Request(AssetCache::PACKET_ADD_CHUNK_REQUEST, key);
            chunkData = AssetCache::ChunkSplitter::GetChunk(addFilesRequest.serializedData->GetDataVector(), currentChunk);
        }

        resultCode = AssetCache::Error::CANNOT_SEND_REQUEST;

        bool requestSent = client.RequestAddNextChunk(key, dataSizeOverall, chunksOverall, currentChunk, chunkData);
        if (requestSent)
        {
            resultCode = WaitRequest();
        }

        {
            LockGuard<Mutex> guard(requestLocker);
            request.Reset();
        }

        if (resultCode != AssetCache::Error::NO_ERRORS)
        {
            break;
        }
    }

    { //process stats
        ++stats.addRequestsCount;
        switch (resultCode)
        {
        case AssetCache::Error::NO_ERRORS:
            ++stats.addRequestsSucceedCount;
            break;
        case AssetCache::Error::OPERATION_TIMEOUT:
            ++stats.addRequestsTimeoutCount;
            break;

        default:
            ++stats.addRequestsFailedCount;
            break;
        }
    }

    return resultCode;
}

AssetCache::Error AssetCacheClient::RequestFromCacheSynchronously(const AssetCache::CacheItemKey& key, AssetCache::CachedItemValue* value)
{
    DVASSERT(value != nullptr);

    {
        LockGuard<Mutex> guard(requestLocker);
        request = Request(AssetCache::PACKET_GET_CHUNK_REQUEST, key);
        getFilesRequest.Reset();
    }

    AssetCache::Error resultCode = AssetCache::Error::CANNOT_SEND_REQUEST;

    bool requestSent = client.RequestGetNextChunk(key, 0);
    if (requestSent)
    {
        resultCode = WaitRequest();
    }

    uint32 chunksOverall = 0;
    {
        LockGuard<Mutex> guard(requestLocker);
        request.Reset();
        chunksOverall = getFilesRequest.chunksOverall;
    }

    if (resultCode == AssetCache::Error::NO_ERRORS)
    {
        DVASSERT(chunksOverall > 0);
        for (uint32 currentChunk = 1; currentChunk < chunksOverall; ++currentChunk)
        {
            {
                LockGuard<Mutex> guard(requestLocker);
                request = Request(AssetCache::PACKET_GET_CHUNK_REQUEST, key);
            }

            resultCode = AssetCache::Error::CANNOT_SEND_REQUEST;

            bool requestSent = client.RequestGetNextChunk(key, currentChunk);
            if (requestSent)
            {
                resultCode = WaitRequest();
            }

            {
                LockGuard<Mutex> guard(requestLocker);
                request.Reset();
            }

            if (resultCode != AssetCache::Error::NO_ERRORS)
            {
                break;
            }
        }

        if (resultCode == AssetCache::Error::NO_ERRORS)
        {
            LockGuard<Mutex> guard(requestLocker);
            if (getFilesRequest.chunksReceived == getFilesRequest.chunksOverall && getFilesRequest.bytesRemaining == 0)
            {
                ScopedPtr<DynamicMemoryFile> f(DynamicMemoryFile::Create(std::move(getFilesRequest.receivedData), File::OPEN | File::READ, "receivedData"));
                value->Deserialize(f);

                const AssetCache::CachedItemValue::Description& description = value->GetDescription();
                Logger::Info("Data got from cache. Generated %s on machine %s (%s)",
                             description.creationDate.c_str(),
                             description.machineName.c_str(),
                             description.comment.c_str());
            }
            else
            {
                Logger::Error("Packet was not completely transferred. Chunks %u/%u, bytes remaining: %u",
                              getFilesRequest.chunksReceived,
                              getFilesRequest.chunksOverall,
                              getFilesRequest.bytesRemaining);
                resultCode = AssetCache::Error::CORRUPTED_DATA;
            }
        }
    }

    { //process stats
        ++stats.getRequestsCount;
        switch (resultCode)
        {
        case AssetCache::Error::NO_ERRORS:
            ++stats.getRequestsSucceedCount;
            break;
        case AssetCache::Error::OPERATION_TIMEOUT:
            ++stats.getRequestsTimeoutCount;
            break;
        case AssetCache::Error::NOT_FOUND_ON_SERVER:
            ++stats.getRequestsNotFoundCount;
            break;

        default:
            ++stats.getRequestsFailedCount;
            break;
        }
    }

    return resultCode;
}

AssetCache::Error AssetCacheClient::RemoveFromCacheSynchronously(const AssetCache::CacheItemKey& key)
{
    {
        LockGuard<Mutex> guard(requestLocker);
        request = Request(AssetCache::PACKET_REMOVE_REQUEST, key);
    }

    AssetCache::Error resultCode = AssetCache::Error::CANNOT_SEND_REQUEST;

    bool requestSent = client.RequestRemoveData(key);
    if (requestSent)
    {
        resultCode = WaitRequest();
    }

    {
        LockGuard<Mutex> guard(requestLocker);
        request.Reset();
    }

    return resultCode;
}

AssetCache::Error AssetCacheClient::ClearCacheSynchronously()
{
    {
        LockGuard<Mutex> guard(requestLocker);
        request = Request(AssetCache::PACKET_CLEAR_REQUEST);
    }

    AssetCache::Error resultCode = AssetCache::Error::CANNOT_SEND_REQUEST;

    bool requestSent = client.RequestClearCache();
    if (requestSent)
    {
        resultCode = WaitRequest();
    }

    {
        LockGuard<Mutex> guard(requestLocker);
        request.Reset();
    }

    return resultCode;
}

AssetCache::Error AssetCacheClient::WaitRequest()
{
    uint64 startTime = SystemTimer::GetMs();

    Request currentRequest;
    {
        LockGuard<Mutex> guard(requestLocker);
        currentRequest = request;
    }

    while (currentRequest.recieved == false)
    {
        ProcessNetwork();

        if (!isActive)
        {
            return AssetCache::Error::OPERATION_TIMEOUT;
        }

        {
            LockGuard<Mutex> guard(requestLocker);
            currentRequest = request;
        }

        uint64 deltaTime = SystemTimer::GetMs() - startTime;
        if (((timeoutMs > 0) && (deltaTime > timeoutMs)) && (currentRequest.recieved == false) && (currentRequest.processingRequest == false))
        {
            Logger::FrameworkDebug("Operation timeout: (%lld ms)", timeoutMs);
            return AssetCache::Error::OPERATION_TIMEOUT;
        }
    }

    if (currentRequest.result == AssetCache::Error::NO_ERRORS)
    {
        while (currentRequest.processingRequest)
        {
            ProcessNetwork();
            LockGuard<Mutex> guard(requestLocker);
            currentRequest = request;
        }
    }

    return currentRequest.result;
}

void AssetCacheClient::OnServerStatusReceived()
{
    LockGuard<Mutex> guard(requestLocker);
    if (request.requestID == AssetCache::PACKET_STATUS_REQUEST)
    {
        request.result = AssetCache::Error::NO_ERRORS;
        request.recieved = true;
        request.processingRequest = false;
    }
    else
    {
        //skip this request, because it was canceled by timeout
    }
}

void AssetCacheClient::OnAddedToCache(const AssetCache::CacheItemKey& key, bool added)
{
    LockGuard<Mutex> guard(requestLocker);

    if ((request.requestID == AssetCache::PACKET_ADD_CHUNK_REQUEST) && request.key == key)
    {
        request.result = (added) ? AssetCache::Error::NO_ERRORS : AssetCache::Error::SERVER_ERROR;
        request.recieved = true;
        request.processingRequest = false;
    }
    else
    {
        //skip this request, because it was canceled by timeout
    }
}

void AssetCacheClient::OnReceivedFromCache(const AssetCache::CacheItemKey& key, uint64 dataSize, uint32 numOfChunks, uint32 chunkNumber, const Vector<uint8>& chunkData)
{
    LockGuard<Mutex> guard(requestLocker);

    if (request.requestID == AssetCache::PACKET_GET_CHUNK_REQUEST && request.key == key)
    {
        if (getFilesRequest.chunksReceived == 0)
        {
            if (dataSize == 0 || numOfChunks == 0)
            {
                request.result = AssetCache::Error::NOT_FOUND_ON_SERVER;
                request.recieved = true;
                request.processingRequest = false;
                return;
            }
            else
            {
                request.result = AssetCache::Error::NO_ERRORS;
                getFilesRequest.chunksOverall = numOfChunks;
                getFilesRequest.bytesRemaining = static_cast<size_t>(dataSize);
                getFilesRequest.receivedData.resize(getFilesRequest.bytesRemaining);
                Logger::FrameworkDebug("Received info: %u bytes, %u chunks", dataSize, numOfChunks);
            }
        }

        if (chunkData.empty())
        {
            request.result = AssetCache::Error::NOT_FOUND_ON_SERVER;
        }
        else if (chunkNumber != getFilesRequest.chunksReceived)
        {
            Logger::Error("Wrong chunk: expected #%u, received #%u", getFilesRequest.chunksReceived, chunkNumber);
            request.result = AssetCache::Error::WRONG_CHUNK;
        }
        else if (getFilesRequest.bytesRemaining < chunkData.size())
        {
            Logger::Error("Chunk #%u size is too big. Remaining bytes: %u, received chunk size: ", chunkNumber, getFilesRequest.bytesRemaining, chunkData.size());
            request.result = AssetCache::Error::WRONG_CHUNK;
        }
        else
        {
            request.result = AssetCache::Error::NO_ERRORS;
            Memcpy(getFilesRequest.receivedData.data() + getFilesRequest.bytesReceived, chunkData.data(), chunkData.size());
            getFilesRequest.bytesReceived += chunkData.size();
            getFilesRequest.bytesRemaining -= chunkData.size();
            ++(getFilesRequest.chunksReceived);
            Logger::FrameworkDebug("Chunk #%u received: %u bytes. Overall received %u, remaining %u", chunkNumber, chunkData.size(), getFilesRequest.bytesReceived, getFilesRequest.bytesRemaining);
        }

        request.recieved = true;
        request.processingRequest = false;
    }
    else
    {
        //skip this request, because it was canceled by timeout
    }
}

void AssetCacheClient::OnRemovedFromCache(const AssetCache::CacheItemKey& key, bool removed)
{
    LockGuard<Mutex> guard(requestLocker);
    if (request.requestID == AssetCache::PACKET_REMOVE_REQUEST && request.key == key)
    {
        request.result = (removed) ? AssetCache::Error::NO_ERRORS : AssetCache::Error::SERVER_ERROR;
        request.recieved = true;
        request.processingRequest = false;
    }
    else
    {
        //skip this request, because it was canceled by timeout
    }
}

void AssetCacheClient::OnCacheCleared(bool cleared)
{
    LockGuard<Mutex> guard(requestLocker);
    if (request.requestID == AssetCache::PACKET_CLEAR_REQUEST)
    {
        request.result = (cleared) ? AssetCache::Error::NO_ERRORS : AssetCache::Error::SERVER_ERROR;
        request.recieved = true;
        request.processingRequest = false;
    }
    else
    {
        //skip this request, because it was canceled by timeout
    }
}

void AssetCacheClient::OnIncorrectPacketReceived(AssetCache::IncorrectPacketType type)
{
    LockGuard<Mutex> guard(requestLocker);
    ++stats.incorrectPacketsCount;
    request.recieved = true;
    request.processingRequest = false;

    switch (type)
    {
    case AssetCache::IncorrectPacketType::UNDEFINED_DATA:
        request.result = AssetCache::Error::CORRUPTED_DATA;
        break;
    case AssetCache::IncorrectPacketType::UNSUPPORTED_VERSION:
        request.result = AssetCache::Error::UNSUPPORTED_VERSION;
        break;
    case AssetCache::IncorrectPacketType::UNEXPECTED_PACKET:
        request.result = AssetCache::Error::UNEXPECTED_PACKET;
        break;
    default:
        DVASSERT(false, Format("Unexpected incorrect packet type: %d", type).c_str());
        request.result = AssetCache::Error::CORRUPTED_DATA;
        break;
    }
}

void AssetCacheClient::OnClientProxyStateChanged()
{
    if (client.ChannelIsOpened() == false)
    {
        isActive = false;

        LockGuard<Mutex> guard(requestLocker);
        request.recieved = true;
        request.processingRequest = false;
        request.result = AssetCache::Error::CANNOT_CONNECT;
    }
}

bool AssetCacheClient::IsConnected() const
{
    return client.ChannelIsOpened();
}

void AssetCacheClient::ProcessNetwork()
{
    Net::NetCore::Instance()->Update();
    if (dispatcher.HasEvents())
    {
        dispatcher.ProcessEvents();
    }
}

void AssetCacheClient::ClearStats()
{
    stats = Stats();
}

void AssetCacheClient::DumpStats() const
{
    if (stats.getRequestsCount + stats.addRequestsCount > 0)
    {
        Logger::Info("======== AC STATS ========");
        if (stats.getRequestsCount > 0)
        {
            Logger::Info("GET:");
            Logger::Info("  all requests: %d", stats.getRequestsCount);

            if (stats.getRequestsSucceedCount > 0)
                Logger::Info("  got from cache: %d", stats.getRequestsSucceedCount);
            if (stats.getRequestsNotFoundCount > 0)
                Logger::Info("  not found: %d", stats.getRequestsNotFoundCount);
            if (stats.getRequestsTimeoutCount > 0)
                Logger::Info("  timeout: %d", stats.getRequestsTimeoutCount);
            if (stats.getRequestsFailedCount > 0)
                Logger::Info("  failed: %d", stats.getRequestsFailedCount);

            DVASSERT(stats.getRequestsCount == (stats.getRequestsFailedCount + stats.getRequestsTimeoutCount + stats.getRequestsSucceedCount + stats.getRequestsNotFoundCount));
        }

        if (stats.addRequestsCount > 0)
        {
            Logger::Info("ADD:");
            Logger::Info("  all requests: %d", stats.addRequestsCount);
            if (stats.addRequestsSucceedCount > 0)
                Logger::Info("  added: %d", stats.addRequestsSucceedCount);
            if (stats.addRequestsTimeoutCount > 0)
                Logger::Info("  timeout: %d", stats.addRequestsTimeoutCount);
            if (stats.addRequestsFailedCount > 0)
                Logger::Info("  failed: %d", stats.addRequestsFailedCount);

            DVASSERT(stats.addRequestsCount == (stats.addRequestsFailedCount + stats.addRequestsTimeoutCount + stats.addRequestsSucceedCount));
        }

        if (stats.incorrectPacketsCount > 0)
        {
            Logger::Info("incorrectPacketsCount: %d", stats.incorrectPacketsCount);
        }
        Logger::Info("==========================");
    }
}

} //namespace DAVA
