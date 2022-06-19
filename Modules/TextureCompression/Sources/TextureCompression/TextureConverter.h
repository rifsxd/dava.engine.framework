#pragma once

#include <Base/BaseTypes.h>
#include <FileSystem/FilePath.h>
#include <Render/RenderBase.h>

namespace DAVA
{
class TextureDescriptor;

class TextureConverter final
{
public:
    enum eConvertQuality : uint32
    {
        ECQ_FASTEST = 0,
        ECQ_FAST,
        ECQ_NORMAL,
        ECQ_HIGH,
        ECQ_VERY_HIGH,

        ECQ_COUNT,
        ECQ_DEFAULT = ECQ_VERY_HIGH
    };

    static FilePath ConvertTexture(const TextureDescriptor& descriptor, eGPUFamily gpuFamily, bool updateAfterConversion, eConvertQuality quality, const FilePath& outFolder = FilePath());
    static FilePath GetOutputPath(const TextureDescriptor& descriptor, eGPUFamily gpuFamily);
};
}
