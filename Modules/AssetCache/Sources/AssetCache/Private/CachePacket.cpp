#include "AssetCache/CachePacket.h"

#include <FileSystem/DynamicMemoryFile.h>
#include <Network/IChannel.h>
#include <Logger/Logger.h>

namespace DAVA
{
namespace AssetCache
{
const uint16 PACKET_HEADER = 0xACCA;
const uint8 PACKET_VERSION = 3;

Map<const uint8*, ScopedPtr<DynamicMemoryFile>> CachePacket::sendingPackets;

namespace CachePacketDetails
{
bool ReadFromBuffer(File* buffer, CacheItemKey& key)
{
    const uint32 keySize = static_cast<uint32>(key.size());
    return (buffer->Read(key.data(), keySize) == keySize);
};

template <class T>
bool ReadFromBuffer(File* buffer, T& value)
{
    return (buffer->Read(&value) == sizeof(value));
};

bool ReadFromBuffer(File* buffer, Vector<uint8>& data, uint32 dataSize)
{
    data.resize(dataSize);
    if (dataSize > 0)
    {
        return (buffer->Read(data.data(), dataSize) == dataSize);
    }
    else
    {
        return true;
    }
};
}

bool CachePacket::SendTo(std::shared_ptr<Net::IChannel> channel)
{
    DVASSERT(channel);

    auto insertRes = sendingPackets.insert(std::make_pair(serializationBuffer->GetData(), serializationBuffer));
    DVASSERT(true == insertRes.second && "packet is already inserted");

    uint32 packetId = 0;
    channel->Send(serializationBuffer->GetData(), static_cast<size_t>(serializationBuffer->GetSize()), 0, &packetId);
    return (packetId != 0);
}

void CachePacket::PacketSent(const uint8* buffer, size_t length)
{
    DVASSERT(sendingPackets.empty() == false);

    auto found = sendingPackets.find(buffer);
    DVASSERT(found != sendingPackets.end() && "packet is not found in sending list");
    DVASSERT(found->second->GetSize() == length);
    sendingPackets.erase(found);
}

CachePacket::CreateResult CachePacket::Create(const uint8* rawdata, uint32 length, std::unique_ptr<CachePacket>& packet)
{
    ScopedPtr<File> buffer(DynamicMemoryFile::Create(const_cast<uint8*>(rawdata), length, File::OPEN | File::READ));

    CachePacketHeader header;
    if (buffer->Read(&header) != sizeof(header))
    {
        Logger::Error("[CachePacket::%s] Cannot read header: %s", __FUNCTION__);
        return ERR_INCORRECT_DATA;
    }

    if (header.headerID != PACKET_HEADER)
    {
        Logger::Error("[CachePacket::%s] Unsupported header id: %d, expected is %d", __FUNCTION__, header.headerID, PACKET_HEADER);
        return ERR_UNSUPPORTED_VERSION;
    }
    if (header.version != PACKET_VERSION)
    {
        Logger::Error("[CachePacket::%s] Unsupported header version: %d, expected is %d", __FUNCTION__, header.version, PACKET_VERSION);
        return ERR_UNSUPPORTED_VERSION;
    }

    packet = CachePacket::CreateByType(header.packetType);
    if (!packet)
    {
        return ERR_INCORRECT_DATA;
    }

    bool loaded = packet->DeserializeFromBuffer(buffer);
    if (!loaded)
    {
        Logger::Error("[CachePacket::%s] Cannot load packet (type: %d)", __FUNCTION__, header.packetType);
        packet.reset();
        return ERR_INCORRECT_DATA;
    }

    return CREATED;
}

std::unique_ptr<CachePacket> CachePacket::CreateByType(ePacketID type)
{
    switch (type)
    {
    case PACKET_ADD_CHUNK_REQUEST:
        return std::unique_ptr<CachePacket>(new AddChunkRequestPacket());
    case PACKET_ADD_RESPONSE:
        return std::unique_ptr<CachePacket>(new AddResponsePacket());
    case PACKET_GET_CHUNK_REQUEST:
        return std::unique_ptr<CachePacket>(new GetChunkRequestPacket());
    case PACKET_GET_CHUNK_RESPONSE:
        return std::unique_ptr<CachePacket>(new GetChunkResponsePacket());
    case PACKET_WARMING_UP_REQUEST:
        return std::unique_ptr<CachePacket>(new WarmupRequestPacket());
    case PACKET_STATUS_REQUEST:
        return std::unique_ptr<CachePacket>(new StatusRequestPacket());
    case PACKET_STATUS_RESPONSE:
        return std::unique_ptr<CachePacket>(new StatusResponsePacket());
    case PACKET_REMOVE_REQUEST:
        return std::unique_ptr<CachePacket>(new RemoveRequestPacket());
    case PACKET_REMOVE_RESPONSE:
        return std::unique_ptr<CachePacket>(new RemoveResponsePacket());
    case PACKET_CLEAR_REQUEST:
        return std::unique_ptr<CachePacket>(new ClearRequestPacket());
    case PACKET_CLEAR_RESPONSE:
        return std::unique_ptr<CachePacket>(new ClearResponsePacket());
    default:
    {
        Logger::Error("[CachePacket::%s] Wrong packet type: %d", __FUNCTION__, type);
        break;
    }
    }

    return nullptr;
}

CachePacket::CachePacket(ePacketID type_, eBufferCreateMode mode)
    : type(type_)
    , serializationBuffer(nullptr)
{
    if (mode == CREATE_SENDING_BUFFER)
    {
        serializationBuffer.reset(DynamicMemoryFile::Create(File::CREATE | File::WRITE));
    }
}

void CachePacket::WriteHeader(File* file) const
{
    CachePacketHeader header(PACKET_HEADER, PACKET_VERSION, type);
    file->Write(&header);
}

//////////////////////////////////////////////////////////////////////////
DataChunkPacket::DataChunkPacket(ePacketID packetId, const CacheItemKey& key, uint64 dataSize, uint32 numOfChunks, uint32 chunkNumber, const Vector<uint8>& chunkData)
    : CachePacket(packetId, CREATE_SENDING_BUFFER)
{
    WriteHeader(serializationBuffer);

    uint32 keySize = static_cast<uint32>(key.size());
    uint32 chunkDataSize = static_cast<uint32>(chunkData.size());

    serializationBuffer->Write(key.data(), keySize);
    serializationBuffer->Write(&dataSize, sizeof(dataSize));
    serializationBuffer->Write(&numOfChunks, sizeof(numOfChunks));
    serializationBuffer->Write(&chunkNumber, sizeof(chunkNumber));
    serializationBuffer->Write(&chunkDataSize, sizeof(chunkDataSize));
    if (chunkDataSize > 0)
    {
        serializationBuffer->Write(chunkData.data(), chunkDataSize);
    }
}

DataChunkPacket::DataChunkPacket(ePacketID packetId)
    : CachePacket(packetId, DO_NOT_CREATE_SENDING_BUFFER)
{
}

bool DataChunkPacket::DeserializeFromBuffer(File* buffer)
{
    using namespace CachePacketDetails;

    uint32 chunkDataSize = 0;
    return (ReadFromBuffer(buffer, key)
            && ReadFromBuffer(buffer, dataSize)
            && ReadFromBuffer(buffer, numOfChunks)
            && ReadFromBuffer(buffer, chunkNumber)
            && ReadFromBuffer(buffer, chunkDataSize)
            && ReadFromBuffer(buffer, chunkData, chunkDataSize));
}

//////////////////////////////////////////////////////////////////////////
AddChunkRequestPacket::AddChunkRequestPacket(const CacheItemKey& key, uint64 dataSize, uint32 numOfChunks, uint32 chunkNumber, const Vector<uint8>& chunkData)
    : DataChunkPacket(PACKET_ADD_CHUNK_REQUEST, key, dataSize, numOfChunks, chunkNumber, chunkData)
{
}

AddChunkRequestPacket::AddChunkRequestPacket()
    : DataChunkPacket(PACKET_ADD_CHUNK_REQUEST)
{
}

//////////////////////////////////////////////////////////////////////////
AddResponsePacket::AddResponsePacket(const CacheItemKey& key_, bool added_)
    : CachePacket(PACKET_ADD_RESPONSE, CREATE_SENDING_BUFFER)
{
    WriteHeader(serializationBuffer);

    serializationBuffer->Write(key_.data(), static_cast<uint32>(key_.size()));
    serializationBuffer->Write(&added_, sizeof(added_));
}

AddResponsePacket::AddResponsePacket()
    : CachePacket(PACKET_ADD_RESPONSE, DO_NOT_CREATE_SENDING_BUFFER)
{
}

bool AddResponsePacket::DeserializeFromBuffer(File* file)
{
    const uint32 keySize = static_cast<uint32>(key.size());
    return ((file->Read(key.data(), keySize) == keySize) && (file->Read(&added) == sizeof(added)));
}

//////////////////////////////////////////////////////////////////////////
GetChunkRequestPacket::GetChunkRequestPacket(const CacheItemKey& key_, uint32 chunkNumber)
    : CachePacket(PACKET_GET_CHUNK_REQUEST, CREATE_SENDING_BUFFER)
{
    WriteHeader(serializationBuffer);

    serializationBuffer->Write(key_.data(), static_cast<uint32>(key_.size()));
    serializationBuffer->Write(&chunkNumber, sizeof(chunkNumber));
}

GetChunkRequestPacket::GetChunkRequestPacket()
    : CachePacket(PACKET_GET_CHUNK_REQUEST, DO_NOT_CREATE_SENDING_BUFFER)
{
}

bool GetChunkRequestPacket::DeserializeFromBuffer(File* buffer)
{
    using namespace CachePacketDetails;
    return ReadFromBuffer(buffer, key) && ReadFromBuffer(buffer, chunkNumber);
}

//////////////////////////////////////////////////////////////////////////
GetChunkResponsePacket::GetChunkResponsePacket(const CacheItemKey& key, uint64 dataSize, uint32 numOfChunks, uint32 chunkNumber, const Vector<uint8>& chunkData)
    : DataChunkPacket(PACKET_GET_CHUNK_RESPONSE, key, dataSize, numOfChunks, chunkNumber, chunkData)
{
}

GetChunkResponsePacket::GetChunkResponsePacket()
    : DataChunkPacket(PACKET_GET_CHUNK_RESPONSE)
{
}

//////////////////////////////////////////////////////////////////////////
WarmupRequestPacket::WarmupRequestPacket(const CacheItemKey& key_)
    : CachePacket(PACKET_WARMING_UP_REQUEST, CREATE_SENDING_BUFFER)
{
    WriteHeader(serializationBuffer);

    serializationBuffer->Write(key_.data(), static_cast<uint32>(key_.size()));
}

WarmupRequestPacket::WarmupRequestPacket()
    : CachePacket(PACKET_WARMING_UP_REQUEST, DO_NOT_CREATE_SENDING_BUFFER)
{
}

bool WarmupRequestPacket::DeserializeFromBuffer(File* file)
{
    const uint32 keySize = static_cast<uint32>(key.size());
    return (file->Read(key.data(), keySize) == keySize);
}

//////////////////////////////////////////////////////////////////////////
StatusRequestPacket::StatusRequestPacket()
    : CachePacket(PACKET_STATUS_REQUEST, CREATE_SENDING_BUFFER)
{
    WriteHeader(serializationBuffer);
}

bool StatusRequestPacket::DeserializeFromBuffer(File* file)
{
    return true;
}

//////////////////////////////////////////////////////////////////////////
StatusResponsePacket::StatusResponsePacket()
    : CachePacket(PACKET_STATUS_RESPONSE, CREATE_SENDING_BUFFER)
{
    WriteHeader(serializationBuffer);
}

bool StatusResponsePacket::DeserializeFromBuffer(File* file)
{
    return true;
}

//////////////////////////////////////////////////////////////////////////
RemoveRequestPacket::RemoveRequestPacket(const CacheItemKey& key_)
    : CachePacket(PACKET_REMOVE_REQUEST, CREATE_SENDING_BUFFER)
{
    WriteHeader(serializationBuffer);
    serializationBuffer->Write(key_.data(), static_cast<uint32>(key_.size()));
}

RemoveRequestPacket::RemoveRequestPacket()
    : CachePacket(PACKET_REMOVE_REQUEST, DO_NOT_CREATE_SENDING_BUFFER)
{
}

bool RemoveRequestPacket::DeserializeFromBuffer(File* file)
{
    const uint32 keySize = static_cast<uint32>(key.size());
    return (file->Read(key.data(), keySize) == keySize);
}

//////////////////////////////////////////////////////////////////////////
RemoveResponsePacket::RemoveResponsePacket(const CacheItemKey& key_, bool removed_)
    : CachePacket(PACKET_REMOVE_RESPONSE, CREATE_SENDING_BUFFER)
{
    WriteHeader(serializationBuffer);

    serializationBuffer->Write(key_.data(), static_cast<uint32>(key_.size()));
    serializationBuffer->Write(&removed_, sizeof(removed_));
}

RemoveResponsePacket::RemoveResponsePacket()
    : CachePacket(PACKET_REMOVE_RESPONSE, DO_NOT_CREATE_SENDING_BUFFER)
{
}

bool RemoveResponsePacket::DeserializeFromBuffer(File* file)
{
    const uint32 keySize = static_cast<uint32>(key.size());
    return ((file->Read(key.data(), keySize) == keySize) && (file->Read(&removed) == sizeof(removed)));
}

//////////////////////////////////////////////////////////////////////////
ClearRequestPacket::ClearRequestPacket()
    : CachePacket(PACKET_CLEAR_REQUEST, CREATE_SENDING_BUFFER)
{
    WriteHeader(serializationBuffer);
}

bool ClearRequestPacket::DeserializeFromBuffer(File* file)
{
    return true;
}

//////////////////////////////////////////////////////////////////////////
ClearResponsePacket::ClearResponsePacket(bool cleared_)
    : CachePacket(PACKET_CLEAR_RESPONSE, CREATE_SENDING_BUFFER)
{
    WriteHeader(serializationBuffer);
    serializationBuffer->Write(&cleared_, sizeof(cleared_));
}

ClearResponsePacket::ClearResponsePacket()
    : CachePacket(PACKET_CLEAR_RESPONSE, DO_NOT_CREATE_SENDING_BUFFER)
{
}

bool ClearResponsePacket::DeserializeFromBuffer(File* file)
{
    return ((file->Read(&cleared) == sizeof(cleared)));
}

} //AssetCache
} //DAVA
