#pragma once

#include <Base/BaseTypes.h>

namespace DAVA
{
namespace AssetCache
{
namespace ChunkSplitter
{
uint32 GetNumberOfChunks(uint64 overallSize);
Vector<uint8> GetChunk(const Vector<uint8>& dataVector, uint32 chunkNumber);
}
} // namespace AssetCache
} // namespace DAVA
