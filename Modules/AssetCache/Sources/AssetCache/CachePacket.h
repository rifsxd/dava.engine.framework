#pragma once

#include "AssetCache/CacheItemKey.h"
#include "AssetCache/CachedItemValue.h"
#include "AssetCache/AssetCacheConstants.h"

#include <FileSystem/DynamicMemoryFile.h>

#include <memory>

namespace DAVA
{
namespace Net
{
struct IChannel;
}

namespace AssetCache
{

#pragma pack(push, 1) // exact fit - no padding
struct CachePacketHeader
{
    CachePacketHeader(){};
    CachePacketHeader(uint16 header, uint8 _version, ePacketID type)
        : headerID(header)
        , version(_version)
        , packetType(type)
    {
    }

    uint16 headerID = 0;
    uint8 version = 0;
    ePacketID packetType = PACKET_UNKNOWN;
};
#pragma pack(pop) //back to whatever the previous packing mode was

class CachePacket
{
public:
    virtual ~CachePacket(){}; // this is base class for asset cache network packets

    enum CreateResult
    {
        CREATED,
        ERR_UNSUPPORTED_VERSION,
        ERR_INCORRECT_DATA
    };
    static CreateResult Create(const uint8* buffer, uint32 length, std::unique_ptr<CachePacket>& packet);

    bool SendTo(std::shared_ptr<Net::IChannel> channel);
    static void PacketSent(const uint8* buffer, size_t length);

protected:
    enum eBufferCreateMode
    {
        CREATE_SENDING_BUFFER,
        DO_NOT_CREATE_SENDING_BUFFER
    };

    CachePacket(ePacketID type, eBufferCreateMode);

    static std::unique_ptr<CachePacket> CreateByType(ePacketID type);

    void WriteHeader(File* file) const;
    virtual bool DeserializeFromBuffer(File* buffer) = 0;

public:
    ePacketID type = PACKET_UNKNOWN;
    ScopedPtr<DynamicMemoryFile> serializationBuffer;

private:
    static Map<const uint8*, ScopedPtr<DynamicMemoryFile>> sendingPackets;
};

//////////////////////////////////////////////////////////////////////////
class DataChunkPacket : public CachePacket
{
public:
    DataChunkPacket(ePacketID packetId);
    DataChunkPacket(ePacketID packetId, const CacheItemKey& key, uint64 dataSize, uint32 numOfChunks, uint32 chunkNumber, const Vector<uint8>& chunkData);

protected:
    bool DeserializeFromBuffer(File* file) override;

public:
    CacheItemKey key;
    uint64 dataSize = 0;
    uint32 numOfChunks = 0;
    uint32 chunkNumber = 0;
    Vector<uint8> chunkData;
};

//////////////////////////////////////////////////////////////////////////
class AddChunkRequestPacket : public DataChunkPacket
{
public:
    AddChunkRequestPacket();
    AddChunkRequestPacket(const CacheItemKey& key, uint64 dataSize, uint32 numOfChunks, uint32 chunkNumber, const Vector<uint8>& chunkData);
};

//////////////////////////////////////////////////////////////////////////
class AddResponsePacket : public CachePacket
{
public:
    AddResponsePacket();
    AddResponsePacket(const CacheItemKey& key, bool added);

protected:
    bool DeserializeFromBuffer(File* file) override;

public:
    CacheItemKey key;
    bool added = false;
};

//////////////////////////////////////////////////////////////////////////
class GetChunkRequestPacket : public CachePacket
{
public:
    GetChunkRequestPacket();
    GetChunkRequestPacket(const CacheItemKey& key, uint32 chunkNumber);

protected:
    bool DeserializeFromBuffer(File* file) override;

public:
    CacheItemKey key;
    uint32 chunkNumber = 0;
};

//////////////////////////////////////////////////////////////////////////
class GetChunkResponsePacket : public DataChunkPacket
{
public:
    GetChunkResponsePacket();
    GetChunkResponsePacket(const CacheItemKey& key, uint64 dataSize, uint32 numOfChunks, uint32 chunkNumber, const Vector<uint8>& chunkData);
};

//////////////////////////////////////////////////////////////////////////
class WarmupRequestPacket : public CachePacket
{
public:
    WarmupRequestPacket();
    WarmupRequestPacket(const CacheItemKey& key);

protected:
    bool DeserializeFromBuffer(File* file) override;

public:
    CacheItemKey key;
};

//////////////////////////////////////////////////////////////////////////
struct StatusRequestPacket : public CachePacket
{
    StatusRequestPacket();

protected:
    bool DeserializeFromBuffer(File* file) override;
};

//////////////////////////////////////////////////////////////////////////
struct StatusResponsePacket : public CachePacket
{
    StatusResponsePacket();

protected:
    bool DeserializeFromBuffer(File* file) override;
};

//////////////////////////////////////////////////////////////////////////
struct RemoveRequestPacket : public CachePacket
{
    RemoveRequestPacket();
    RemoveRequestPacket(const CacheItemKey& key);

protected:
    bool DeserializeFromBuffer(File* file) override;

public:
    CacheItemKey key;
};

//////////////////////////////////////////////////////////////////////////
class RemoveResponsePacket : public CachePacket
{
public:
    RemoveResponsePacket();
    RemoveResponsePacket(const CacheItemKey& key, bool removed);

protected:
    bool DeserializeFromBuffer(File* file) override;

public:
    CacheItemKey key;
    bool removed = false;
};

//////////////////////////////////////////////////////////////////////////
struct ClearRequestPacket : public CachePacket
{
    ClearRequestPacket();

protected:
    bool DeserializeFromBuffer(File* file) override;
};

//////////////////////////////////////////////////////////////////////////
struct ClearResponsePacket : public CachePacket
{
    ClearResponsePacket();
    ClearResponsePacket(bool cleared);

protected:
    bool DeserializeFromBuffer(File* file) override;

public:
    bool cleared = false;
};

} // end of namespace AssetCache
} // end of namespace DAVA
