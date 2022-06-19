#pragma once

#include "TextureCompression/TextureConverter.h"
#include "Math/RectanglePacker/Spritesheet.h"
#include "TexturePacker/DefinitionFile.h"
#include "Math/RectanglePacker/RectanglePacker.h"

#include <Base/BaseTypes.h>
#include <Functional/Function.h>
#include <Render/RenderBase.h>
#include <Render/Texture.h>
#include <Math/Math2D.h>

namespace DAVA
{
class DefinitionFile;
class ImageExt;
class FilePath;

class TexturePacker
{
public:
    static const uint32 DEFAULT_TEXTURE_SIZE = 2048;
    static const Set<PixelFormat> PIXEL_FORMATS_WITH_COMPRESSION;
    static const uint32 DEFAULT_MARGIN = 1;

    struct FilterItem
    {
        int8 minFilter;
        int8 magFilter;
        int8 mipFilter;

        FilterItem(int8 minF, int8 magF, int8 mipF)
        {
            minFilter = minF;
            magFilter = magF;
            mipFilter = mipF;
        }
    };

public:
    TexturePacker();

    // pack textures to single texture
    void PackToTextures(const FilePath& outputPath, const DefinitionFile::Collection& defsList, const Vector<eGPUFamily>& forGPUs);
    // page each PSD file to separate texture
    void PackToTexturesSeparate(const FilePath& outputPath, const DefinitionFile::Collection& defsList, const Vector<eGPUFamily>& forGPUs);
    // pack one sprite and use several textures if more than one needed
    void PackToMultipleTextures(const FilePath& outputPath, const char* basename, const DefinitionFile::Collection& remainingList, const Vector<eGPUFamily>& forGPUs);

    void SetConvertQuality(TextureConverter::eConvertQuality quality);
    void SetTexturePostfix(const String& postfix);

    // Proxy setters
    void SetUseOnlySquareTextures(bool value = true);
    void SetMaxTextureSize(uint32 maxTextureSize);
    void SetAlgorithms(const Vector<PackingAlgorithm>& algorithms);
    void SetTwoSideMargin(bool val = true);
    void SetTexturesMargin(uint32 margin);
    const Set<String>& GetErrors() const;

private:
    struct ImageExportKeys
    {
        eGPUFamily forGPU = GPU_ORIGIN;
        ImageFormat imageFormat = IMAGE_FORMAT_UNKNOWN;
        PixelFormat pixelFormat = FORMAT_INVALID;
        ImageQuality imageQuality = DEFAULT_IMAGE_QUALITY;
        bool toComressForGPU = false;
        bool toConvertOrigin = false;
    };

    Vector<ImageExportKeys> GetExportKeys(const Vector<eGPUFamily>& forGPUs);
    void ExportImage(const ImageExt& image, const Vector<ImageExportKeys>& exportKeys, const FilePath& exportedPathname);

    rhi::TextureAddrMode GetDescriptorWrapMode();
    FilterItem GetDescriptorFilter(bool generateMipMaps = false);

    bool CheckFrameSize(const Size2i& spriteSize, const Size2i& frameSize);

    std::unique_ptr<RectanglePacker::PackTask> CreatePackTask(const DefinitionFile::Collection& defList, const Vector<ImageExportKeys>& imageExportKeys);
    void SaveResultSheets(const FilePath& outputPath, const char* basename, const RectanglePacker::PackResult& packResult, const Vector<ImageExportKeys>& imageExportKeys);

    bool WriteDefinition(const std::unique_ptr<SpritesheetLayout>& sheet, const FilePath& outputPath, const String& textureName, const DefinitionFile& defFile);
    bool WriteMultipleDefinition(const RectanglePacker::SpriteIndexedData& spriteIndexedData, const DefinitionFile& defFile, const FilePath& outputPath, const char* textureBasename);
    void WriteDefinitionString(FILE* fp, const Rect2i& writeRect, const Rect2i& originRect, int textureIndex, const String& frameName);

    void DrawToFinalImage(ImageExt& finalImage, ImageExt& drawedImage, const SpriteBoundsRect& drawRect, const Rect2i& frameRect);

    String MakeTextureName(const char* basename, uint32 textureIndex) const;

    bool NeedSquareTextureForCompression(const Vector<ImageExportKeys>& keys);

    TextureConverter::eConvertQuality quality;

    String texturePostfix;

    Set<String> errors;
    void AddError(const String& errorMsg);
    void AddErrors(const Set<String>& errors_);

    RectanglePacker rectanglePacker;
};

inline void TexturePacker::SetUseOnlySquareTextures(bool value)
{
    rectanglePacker.SetUseOnlySquareTextures(value);
}
inline void TexturePacker::SetMaxTextureSize(uint32 value)
{
    rectanglePacker.SetMaxTextureSize(value);
}
inline void TexturePacker::SetAlgorithms(const Vector<PackingAlgorithm>& value)
{
    rectanglePacker.SetAlgorithms(value);
}
inline void TexturePacker::SetTwoSideMargin(bool value)
{
    rectanglePacker.SetTwoSideMargin(value);
}
inline void TexturePacker::SetTexturesMargin(uint32 value)
{
    rectanglePacker.SetTexturesMargin(value);
}
};
