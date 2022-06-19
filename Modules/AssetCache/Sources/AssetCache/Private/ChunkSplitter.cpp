#include "AssetCache/ChunkSplitter.h"

namespace DAVA
{
namespace AssetCache
{
namespace ChunkSplitter
{
const uint32 CHUNK_SIZE_IN_BYTES = 5 * 1024 * 1024;

uint32 GetNumberOfChunks(uint64 overallSize)
{
    return static_cast<uint32>((overallSize + CHUNK_SIZE_IN_BYTES - 1) / CHUNK_SIZE_IN_BYTES);
}

Vector<uint8> GetChunk(const Vector<uint8>& dataVector, uint32 chunkNumber)
{
    size_t firstByte = static_cast<size_t>(chunkNumber * CHUNK_SIZE_IN_BYTES);
    if (firstByte < dataVector.size())
    {
        size_t beyondLastByte = std::min(dataVector.size(), static_cast<size_t>(firstByte + CHUNK_SIZE_IN_BYTES));
        return Vector<uint8>(dataVector.begin() + firstByte, dataVector.begin() + beyondLastByte);
    }
    else
    {
        return Vector<uint8>();
    }
}
}
} // namespace AssetCache
} // namespace DAVA
