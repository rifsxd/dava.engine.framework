#pragma once

#include "TextureCompression/TextureConverter.h"

#include <Base/StaticSingleton.h>
#include <Base/BaseTypes.h>
#include <Render/RenderBase.h>
#include <FileSystem/FilePath.h>

namespace DAVA
{
struct ImageInfo;
class TextureDescriptor;
class PVRConverter : public StaticSingleton<PVRConverter>
{
public:
    PVRConverter();
    virtual ~PVRConverter();

    FilePath ConvertToPvr(const TextureDescriptor& descriptor, eGPUFamily gpuFamily, TextureConverter::eConvertQuality quality, bool addCRC, const FilePath& outFolder);
    FilePath ConvertNormalMapToPvr(const TextureDescriptor& descriptor, eGPUFamily gpuFamily, TextureConverter::eConvertQuality quality, const FilePath& outFolder);

    void SetPVRTexTool(const FilePath& textToolPathname);

    FilePath GetConvertedTexturePath(const TextureDescriptor& descriptor, eGPUFamily gpuFamily, const FilePath& outFolder);

protected:
    FilePath ConvertFloatTexture(const TextureDescriptor& descriptor, eGPUFamily gpuFamily, TextureConverter::eConvertQuality quality, const ImageInfo& sourceInfo, const FilePath& outFolder);
    FilePath ConvertFloatCubeTexture(const TextureDescriptor& descriptor, eGPUFamily gpuFamily, TextureConverter::eConvertQuality quality, const ImageInfo& sourceInfo, const FilePath& outFolder);
    FilePath PrepareCubeMapForPvrConvert(const TextureDescriptor& descriptor);
    void CleanupCubemapAfterConversion(const TextureDescriptor& descriptor);

    void GetToolCommandLine(const TextureDescriptor& descriptor, const FilePath& fileToConvert, eGPUFamily gpuFamily, TextureConverter::eConvertQuality quality, const FilePath& outFolder, Vector<String>& args);

    String GenerateInputName(const TextureDescriptor& descriptor, const FilePath& fileToConvert);

protected:
    Map<PixelFormat, String> pixelFormatToPVRFormat;

    FilePath pvrTexToolPathname;

    Vector<String> pvrToolSuffixes;
    Vector<String> cubemapSuffixes;
};
}
