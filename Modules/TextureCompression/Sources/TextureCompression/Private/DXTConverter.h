#pragma once

#include <Render/RenderBase.h>

namespace DAVA
{
class FilePath;
class TextureDescriptor;

class DXTConverter final
{
public:
    static FilePath ConvertToDxt(const TextureDescriptor& descriptor, eGPUFamily gpuFamily, const FilePath& outFolder);
    static FilePath ConvertCubemapToDxt(const TextureDescriptor& descriptor, eGPUFamily gpuFamily, const FilePath& outFolder);
    static FilePath GetDXTOutput(const TextureDescriptor& descriptor, eGPUFamily gpuFamily, const FilePath& outFolder);
};
}
