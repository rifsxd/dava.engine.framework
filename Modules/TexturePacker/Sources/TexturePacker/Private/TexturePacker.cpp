#include "TexturePacker/TexturePacker.h"
#include "Math/RectanglePacker/Spritesheet.h"
#include "TexturePacker/ImageExt.h"
#include "TexturePacker/DefinitionFile.h"
#include "TextureCompression/TextureConverter.h"
#include "TexturePacker/FramePathHelper.h"

#include <CommandLine/CommandLineParser.h>
#include <Render/TextureDescriptor.h>
#include <Render/GPUFamilyDescriptor.h>
#include <Render/PixelFormatDescriptor.h>
#include <Render/Image/ImageSystem.h>
#include <Render/Image/ImageFormatInterface.h>
#include <Render/Image/ImageConvert.h>
#include <FileSystem/FileSystem.h>
#include <Utils/StringFormat.h>
#include <Base/GlobalEnum.h>


#ifdef WIN32
#define snprintf _snprintf
#endif

namespace DAVA
{
static Set<PixelFormat> InitPixelFormatsWithCompression()
{
    Set<PixelFormat> set;
    set.insert(FORMAT_PVR4);
    set.insert(FORMAT_PVR2);
    set.insert(FORMAT_DXT1);
    set.insert(FORMAT_DXT1A);
    set.insert(FORMAT_DXT3);
    set.insert(FORMAT_DXT5);
    set.insert(FORMAT_DXT5NM);
    set.insert(FORMAT_ETC1);
    set.insert(FORMAT_ATC_RGB);
    set.insert(FORMAT_ATC_RGBA_EXPLICIT_ALPHA);
    set.insert(FORMAT_ATC_RGBA_INTERPOLATED_ALPHA);
    set.insert(FORMAT_PVR2_2);
    set.insert(FORMAT_PVR4_2);
    set.insert(FORMAT_EAC_R11_UNSIGNED);
    set.insert(FORMAT_EAC_R11_SIGNED);
    set.insert(FORMAT_EAC_RG11_UNSIGNED);
    set.insert(FORMAT_EAC_RG11_SIGNED);
    set.insert(FORMAT_ETC2_RGB);
    set.insert(FORMAT_ETC2_RGBA);
    set.insert(FORMAT_ETC2_RGB_A1);

    return set;
}

const Set<PixelFormat> TexturePacker::PIXEL_FORMATS_WITH_COMPRESSION = InitPixelFormatsWithCompression();

TexturePacker::TexturePacker()
{
    quality = TextureConverter::ECQ_VERY_HIGH;
    if (CommandLineParser::Instance()->IsFlagSet("--quality"))
    {
        String qualityName = CommandLineParser::Instance()->GetParamForFlag("--quality");
        int32 q = atoi(qualityName.c_str());
        if ((q >= TextureConverter::ECQ_FASTEST) && (q <= TextureConverter::ECQ_VERY_HIGH))
        {
            quality = static_cast<TextureConverter::eConvertQuality>(q);
        }
    }
}

void TexturePacker::PackToTexturesSeparate(const FilePath& outputPath, const DefinitionFile::Collection& defsList, const Vector<eGPUFamily>& forGPUs)
{
    Logger::FrameworkDebug("Packing to separate textures");

    for (const DefinitionFile::Pointer& defFile : defsList)
    {
        PackToMultipleTextures(outputPath, defFile->filename.GetBasename().c_str(), { defFile }, forGPUs);
    }
}

void TexturePacker::PackToTextures(const FilePath& outputPath, const DefinitionFile::Collection& defsList, const Vector<eGPUFamily>& forGPUs)
{
    PackToMultipleTextures(outputPath, "texture", defsList, forGPUs);
}

void TexturePacker::PackToMultipleTextures(const FilePath& outputPath, const char* basename, const DefinitionFile::Collection& defList, const Vector<eGPUFamily>& forGPUs)
{
    Vector<ImageExportKeys> imageExportKeys = GetExportKeys(forGPUs);
    std::unique_ptr<RectanglePacker::PackTask> packTask = CreatePackTask(defList, imageExportKeys);
    std::unique_ptr<const RectanglePacker::PackResult> packResult = rectanglePacker.Pack(*packTask);
    if (packResult->Success())
    {
        DVASSERT(packResult->resultIndexedSprites.size() == defList.size());
        SaveResultSheets(outputPath, basename, *packResult, imageExportKeys);
    }
    else
    {
        AddErrors(packResult->resultErrors);
    }
}

std::unique_ptr<RectanglePacker::PackTask> TexturePacker::CreatePackTask(const DefinitionFile::Collection& defList, const Vector<ImageExportKeys>& imageExportKeys)
{
    auto packTask = std::make_unique<RectanglePacker::PackTask>();
    packTask->needSquareTextureOverriden = NeedSquareTextureForCompression(imageExportKeys);
    for (const DefinitionFile::Pointer& defFile : defList)
    {
        auto spriteDef = std::make_shared<RectanglePacker::SpriteDefinition>();
        spriteDef->frameRects = defFile->frameRects;
        spriteDef->dataPtr = defFile.Get();
        packTask->spriteList.push_back(spriteDef);
    }
    return packTask;
}

void TexturePacker::SaveResultSheets(const FilePath& outputPath, const char* basename, const RectanglePacker::PackResult& packResult, const Vector<ImageExportKeys>& imageExportKeys)
{
    Logger::FrameworkDebug("* Writing %d final texture(s)", static_cast<int32>(packResult.resultSheets.size()));

    Vector<ImageExt> finalImages(packResult.resultSheets.size());
    for (uint32 i = 0; i < finalImages.size(); ++i)
    {
        finalImages[i].Create(packResult.resultSheets[i]->GetRect().dx, packResult.resultSheets[i]->GetRect().dy);
    }

    for (const RectanglePacker::SpriteIndexedData& spriteIndexedData : packResult.resultIndexedSprites)
    {
        const RectanglePacker::SpriteDefinition* spriteDef = spriteIndexedData.spriteDef.get();
        DefinitionFile* defFile = static_cast<DefinitionFile*>(spriteDef->dataPtr);
        FilePath withoutExt(defFile->filename);
        withoutExt.TruncateExtension();
        uint32 frameCount = static_cast<uint32>(spriteIndexedData.frameToSheetIndex.size());
        DVASSERT(frameCount == defFile->frameCount);
        for (uint32 frame = 0; frame < frameCount; ++frame)
        {
            int32 sheetIndex = spriteIndexedData.frameToSheetIndex[frame];
            const SpriteBoundsRect* packedInfo = spriteIndexedData.frameToPackedInfo[frame];
            if (nullptr != packedInfo)
            {
                Logger::FrameworkDebug("[MultiPack] pack to texture: %d", sheetIndex);
                FilePath imagePath = FramePathHelper::GetFramePathRelative(withoutExt, frame, defFile->extension);
                ImageExt image;
                image.Read(imagePath);
                DrawToFinalImage(finalImages[sheetIndex], image, *packedInfo, spriteDef->frameRects[frame]);
            }
            else
            {
                Logger::Error("[MultiPack] Can not pack frame %d from %s", frame, withoutExt.GetStringValue().c_str());
            }
        }
    }

    for (uint32 imageNum = 0; imageNum < finalImages.size(); ++imageNum)
    {
        String textureName = MakeTextureName(basename, imageNum);
        FilePath texturePathWithoutExtension = outputPath + textureName;
        ExportImage(finalImages[imageNum], imageExportKeys, texturePathWithoutExtension);
    }

    for (const RectanglePacker::SpriteIndexedData& spriteIndexedData : packResult.resultIndexedSprites)
    {
        DefinitionFile* defFile = static_cast<DefinitionFile*>(spriteIndexedData.spriteDef->dataPtr);
        String fileName = defFile->filename.GetFilename();
        if (!WriteMultipleDefinition(spriteIndexedData, *defFile, outputPath, basename))
        {
            AddError(Format("* ERROR: Failed to write definition - %s.", fileName.c_str()));
        }
    }
}

String TexturePacker::MakeTextureName(const char* basename, uint32 textureIndex) const
{
    std::stringstream name;
    name << basename << textureIndex << texturePostfix;
    return name.str();
}

bool TexturePacker::WriteMultipleDefinition(const RectanglePacker::SpriteIndexedData& spriteIndexedData, const DefinitionFile& defFile, const FilePath& outputPath, const char* textureBasename)
{
    String fileName = defFile.filename.GetFilename();
    Logger::FrameworkDebug("* Write definition: %s", fileName.c_str());

    FilePath defFilePath = outputPath + fileName;
    FILE* fp = fopen(defFilePath.GetAbsolutePathname().c_str(), "wt");
    if (fp == nullptr)
    {
        AddError(Format("Unable to open file for writing: %s", defFilePath.GetAbsolutePathname().c_str()));
        return false;
    }

    String textureExtension = TextureDescriptor::GetDescriptorExtension();

    uint32 frameCount = static_cast<uint32>(spriteIndexedData.frameToSheetIndex.size());

    Map<int32, int32> sheetIndexToFileIndex;

    // find used texture indexes for this sprite
    for (uint32 frame = 0; frame < frameCount; ++frame)
    {
        int32 sheetIndex = spriteIndexedData.frameToSheetIndex[frame];
        // add value to map to show that this packerIndex was used
        sheetIndexToFileIndex[sheetIndex] = -1;
    }

    // write real used packers count
    fprintf(fp, "%d\n", static_cast<int>(sheetIndexToFileIndex.size()));

    int realIndex = 0;
    // write user texture indexes
    for (auto& sheetIndexToFile : sheetIndexToFileIndex)
    {
        uint32 sheetIndex = sheetIndexToFile.first;
        // here we write filename for i-th texture and write to map real index in file for this texture
        String textureName = MakeTextureName(textureBasename, sheetIndex);
        fprintf(fp, "%s%s\n", textureName.c_str(), textureExtension.c_str());
        sheetIndexToFile.second = realIndex++;
    }

    fprintf(fp, "%d %d\n", defFile.spriteWidth, defFile.spriteHeight);
    fprintf(fp, "%d\n", defFile.frameCount);
    for (uint32 frame = 0; frame < defFile.frameCount; ++frame)
    {
        const SpriteBoundsRect* spriteBounds = spriteIndexedData.frameToPackedInfo[frame];

        int32 packerIndex = sheetIndexToFileIndex[spriteIndexedData.frameToSheetIndex[frame]]; // here get real index in file for our used texture
        if (spriteBounds)
        {
            const Rect2i& origRect = defFile.frameRects[frame];
            const Rect2i& writeRect = spriteBounds->spriteRect;
            String frameName = defFile.frameNames.size() > 0 ? defFile.frameNames[frame] : String();
            WriteDefinitionString(fp, writeRect, origRect, packerIndex, frameName);

            if (!CheckFrameSize(Size2i(defFile.spriteWidth, defFile.spriteHeight), writeRect.GetSize()))
            {
                Logger::Warning("In sprite %s.psd frame %d has size bigger than sprite size. Frame will be cropped.", defFile.filename.GetBasename().c_str(), frame);
            }
        }
        else
        {
            AddError(Format("*** FATAL ERROR: Can't find rect in all of packers for frame - %d. Definition file - %s.",
                            frame,
                            fileName.c_str()));

            fclose(fp);
            FileSystem::Instance()->DeleteFile(outputPath + fileName);
            return false;
        }
    }

    fclose(fp);
    return true;
}

void TexturePacker::SetTexturePostfix(const String& postfix)
{
    texturePostfix = postfix;
}

uint8 GetPixelParameterAt(const Vector<String>& gpuParams, uint8 gpuParamPosition, PixelFormat& pixelFormat)
{
    if (gpuParamPosition < gpuParams.size())
    {
        PixelFormat format = PixelFormatDescriptor::GetPixelFormatByName(FastName(gpuParams[gpuParamPosition].c_str()));
        if (format != FORMAT_INVALID)
        {
            pixelFormat = format;
            return 1;
        }
    }

    return 0;
}

uint8 GetImageParametersAt(const Vector<String>& gpuParams, uint8 gpuParamPosition, ImageFormat& imageFormat, ImageQuality& imageQuality)
{
    uint8 paramsRead = 0;

    if (gpuParamPosition < gpuParams.size())
    {
        ImageFormat format = ImageSystem::GetImageFormatByName(gpuParams[gpuParamPosition]);
        if (format != IMAGE_FORMAT_UNKNOWN)
        {
            imageFormat = format;
            ++paramsRead;
            ++gpuParamPosition;

            if (gpuParamPosition < gpuParams.size())
            {
                if (CompareCaseInsensitive(gpuParams[gpuParamPosition], "lossless") == 0)
                {
                    imageQuality = ImageQuality::LOSSLESS_IMAGE_QUALITY;
                    ++paramsRead;
                }
                else
                {
                    int res = -1;
                    bool parseOk = ParseFromString(gpuParams[gpuParamPosition], res);

                    if (parseOk && res >= 0 && res <= 100)
                    {
                        imageQuality = static_cast<ImageQuality>(res);
                        ++paramsRead;
                    }
                }
            }
        }
    }

    return paramsRead;
}

bool GetGpuParameters(eGPUFamily forGPU, PixelFormat& pixelFormat, ImageFormat& imageFormat,
                      ImageQuality& imageQuality)
{
    const String gpuNameFlag = "--" + GPUFamilyDescriptor::GetGPUName(forGPU);
    // gpu flag has at least one additional parameter: pixel format or image format, or both of them
    // these params may follow in arbitrary order
    Vector<String> gpuParams = CommandLineParser::Instance()->GetParamsForFlag(gpuNameFlag);

    // suppose pixel parameter is at first position and read it
    uint8 gpuParamPosition = 0;
    uint8 pixelParamsRead = GetPixelParameterAt(gpuParams, gpuParamPosition, pixelFormat);

    if (pixelParamsRead == 1)
    {
        ++gpuParamPosition;
    }

    // read image parameters
    uint8 imageParamsRead = GetImageParametersAt(gpuParams, gpuParamPosition, imageFormat, imageQuality);

    // read pixel parameter in case if image parameters were on first position
    if (!pixelParamsRead && imageParamsRead)
    {
        gpuParamPosition += imageParamsRead;
        pixelParamsRead = GetPixelParameterAt(gpuParams, gpuParamPosition, pixelFormat);
    }

    return (pixelParamsRead || imageParamsRead);
}

Vector<TexturePacker::ImageExportKeys> TexturePacker::GetExportKeys(const Vector<eGPUFamily>& forGPUs)
{
    Vector<ImageExportKeys> compressionTargets;

    size_type count = forGPUs.size();
    compressionTargets.resize(count);
    for (size_type i = 0; i < count; ++i)
    {
        eGPUFamily curGPU = forGPUs[i];
        if (curGPU == eGPUFamily::GPU_FAMILY_COUNT)
        {
            size_type targetsCount = static_cast<size_type>(curGPU);
            compressionTargets.resize(targetsCount);
            for (size_type i = 0; i < targetsCount; ++i)
            {
                compressionTargets[i].forGPU = static_cast<eGPUFamily>(i);
            }
            break;
        }
        else
        {
            compressionTargets[i].forGPU = curGPU;
        }
    }

    for (ImageExportKeys& keys : compressionTargets)
    {
        if (keys.forGPU == eGPUFamily::GPU_ORIGIN)
        {
            keys.pixelFormat = PixelFormat::FORMAT_RGBA8888;
            keys.imageFormat = ImageFormat::IMAGE_FORMAT_PNG;
        }
        else if (GPUFamilyDescriptor::IsGPUForDevice(keys.forGPU))
        {
            { // read GPU parameters
                const String gpuNameFlag = "--" + GPUFamilyDescriptor::GetGPUName(keys.forGPU);
                bool gpuParametersRead = false;
                if (CommandLineParser::Instance()->IsFlagSet(gpuNameFlag))
                {
                    gpuParametersRead = GetGpuParameters(keys.forGPU, keys.pixelFormat, keys.imageFormat, keys.imageQuality);
                }

                if (!gpuParametersRead)
                {
                    AddError(Format("Cannot read compression options for GPU '%s'", GlobalEnumMap<eGPUFamily>::Instance()->ToString(keys.forGPU)));
                    continue;
                }
            }

            { // validate pixel and image formats
                if (keys.pixelFormat == PixelFormat::FORMAT_INVALID)
                { //we should always set target pixel format in flags.txt
                    AddError(Format("Compression format was not selected for GPU '%s'", GlobalEnumMap<eGPUFamily>::Instance()->ToString(keys.forGPU)));
                    keys.imageFormat = ImageFormat::IMAGE_FORMAT_UNKNOWN;
                    continue;
                }

                if (keys.imageFormat == ImageFormat::IMAGE_FORMAT_UNKNOWN)
                {
                    keys.imageFormat = GPUFamilyDescriptor::GetCompressedFileFormat(keys.forGPU, keys.pixelFormat);
                }
                else if (keys.imageFormat == ImageFormat::IMAGE_FORMAT_PVR || keys.imageFormat == ImageFormat::IMAGE_FORMAT_DDS)
                {
                    if (GPUFamilyDescriptor::GetCompressedFileFormat(keys.forGPU, keys.pixelFormat) != keys.imageFormat)
                    {
                        keys.imageFormat = ImageFormat::IMAGE_FORMAT_UNKNOWN;
                    }
                }

                if (keys.imageFormat == ImageFormat::IMAGE_FORMAT_UNKNOWN)
                {
                    keys.pixelFormat = PixelFormat::FORMAT_INVALID;
                    AddError(Format("Incorrect settings were selected for GPU '%s'", GlobalEnumMap<eGPUFamily>::Instance()->ToString(keys.forGPU)));
                    continue;
                }
            }

            { // define internal format convertation options. should be refactored after PVR/DXT formats will be moved into ImageConvert
                if (keys.imageFormat == ImageFormat::IMAGE_FORMAT_WEBP
                    || keys.imageFormat == ImageFormat::IMAGE_FORMAT_JPEG
                    || keys.imageFormat == ImageFormat::IMAGE_FORMAT_TGA
                    || keys.imageFormat == ImageFormat::IMAGE_FORMAT_PNG
                    )
                {
                    keys.toConvertOrigin = (keys.pixelFormat == PixelFormat::FORMAT_RGBA8888) ? true : ImageConvert::CanConvertFromTo(PixelFormat::FORMAT_RGBA8888, keys.pixelFormat);
                    if (!keys.toConvertOrigin)
                    {
                        AddError(Format("Can't convert to '%s'", GlobalEnumMap<PixelFormat>::Instance()->ToString(keys.pixelFormat)));

                        keys.pixelFormat = PixelFormat::FORMAT_INVALID;
                        keys.imageFormat = ImageFormat::IMAGE_FORMAT_UNKNOWN;
                        continue;
                    }
                }
                else if (keys.imageFormat == ImageFormat::IMAGE_FORMAT_PVR || keys.imageFormat == ImageFormat::IMAGE_FORMAT_DDS)
                {
                    keys.toComressForGPU = GPUFamilyDescriptor::IsFormatSupported(keys.forGPU, keys.pixelFormat);
                    if (!keys.toComressForGPU)
                    {
                        AddError(Format("DVASSERT: Compression format '%s' is not supported for GPU '%s'",
                                        GlobalEnumMap<PixelFormat>::Instance()->ToString(keys.pixelFormat),
                                        GlobalEnumMap<eGPUFamily>::Instance()->ToString(keys.forGPU)));

                        keys.pixelFormat = PixelFormat::FORMAT_INVALID;
                        keys.imageFormat = ImageFormat::IMAGE_FORMAT_UNKNOWN;
                        continue;
                    }
                }
            }
        }
    }

    return compressionTargets;
}

void TexturePacker::ExportImage(const ImageExt& image, const Vector<ImageExportKeys>& keys, const FilePath& pathnameWithoutExtension)
{
    std::unique_ptr<TextureDescriptor> descriptor(new TextureDescriptor());

    { // prepare general info for sprite drawing
        descriptor->drawSettings.wrapModeS = descriptor->drawSettings.wrapModeT = GetDescriptorWrapMode();
        descriptor->SetGenerateMipmaps(CommandLineParser::Instance()->IsFlagSet(String("--generateMipMaps")));

        TexturePacker::FilterItem ftItem = GetDescriptorFilter(descriptor->GetGenerateMipMaps());
        descriptor->drawSettings.minFilter = ftItem.minFilter;
        descriptor->drawSettings.magFilter = ftItem.magFilter;
        descriptor->drawSettings.mipFilter = ftItem.mipFilter;
        descriptor->pathname = pathnameWithoutExtension + TextureDescriptor::GetDescriptorExtension();
    }

    for (const ImageExportKeys& key : keys)
    {
        if (key.imageFormat == ImageFormat::IMAGE_FORMAT_UNKNOWN || key.pixelFormat == PixelFormat::FORMAT_INVALID)
        {
            Logger::Error("Cannot export texture %s for GPU %s", pathnameWithoutExtension.GetStringValue().c_str(), GlobalEnumMap<eGPUFamily>::Instance()->ToString(key.forGPU));
            continue;
        }

        descriptor->compression[key.forGPU].format = key.pixelFormat;
        descriptor->compression[key.forGPU].imageFormat = key.imageFormat;

        ImageExt imageForGPU(image);
        if (key.imageFormat == ImageFormat::IMAGE_FORMAT_DDS || key.imageFormat == ImageFormat::IMAGE_FORMAT_PVR)
        {
            descriptor->dataSettings.sourceFileFormat = IMAGE_FORMAT_PNG;
        }
        else
        {
            descriptor->dataSettings.sourceFileFormat = key.imageFormat;
            if (key.toConvertOrigin)
            {
                imageForGPU.ConvertToFormat(key.pixelFormat);
            }
        }

        String srcExtension = ImageSystem::GetExtensionsFor(descriptor->dataSettings.sourceFileFormat)[0];
        descriptor->dataSettings.sourceFileExtension = srcExtension;

        imageForGPU.DitherAlpha();
        imageForGPU.Write(descriptor->GetSourceTexturePathname(), key.imageQuality); // save source image

        if (key.toComressForGPU)
        {
            TextureConverter::ConvertTexture(*descriptor, key.forGPU, false, quality);
        }
        else if (key.forGPU != eGPUFamily::GPU_ORIGIN)
        {
            FilePath gpuPath = descriptor->CreateMultiMipPathnameForGPU(key.forGPU);
            FileSystem::Instance()->MoveFile(descriptor->GetSourceTexturePathname(), gpuPath); //create image for gpu (webp/tga ...)
        }
    }

    if (keys.size() == 1)
    {
        descriptor->Export(descriptor->pathname, keys[0].forGPU);
        if (keys[0].toComressForGPU)
        {
            FileSystem::Instance()->DeleteFile(descriptor->GetSourceTexturePathname());
        }
    }
    else
    {
        descriptor->Save(descriptor->pathname);
    }
}

rhi::TextureAddrMode TexturePacker::GetDescriptorWrapMode()
{
    if (CommandLineParser::Instance()->IsFlagSet("--wrapClampToEdge"))
    {
        return rhi::TEXADDR_CLAMP;
    }
    else if (CommandLineParser::Instance()->IsFlagSet("--wrapRepeat"))
    {
        return rhi::TEXADDR_WRAP;
    }
    else if (CommandLineParser::Instance()->IsFlagSet("--wrapMirror"))
    {
        return rhi::TEXADDR_MIRROR;
    }

    // Default Wrap mode
    return rhi::TEXADDR_CLAMP;
}

TexturePacker::FilterItem TexturePacker::GetDescriptorFilter(bool generateMipMaps)
{
    // Default filter
    TexturePacker::FilterItem filterItem(rhi::TEXFILTER_LINEAR, rhi::TEXFILTER_LINEAR, generateMipMaps ? rhi::TEXMIPFILTER_LINEAR : rhi::TEXMIPFILTER_NONE);

    if (CommandLineParser::Instance()->IsFlagSet("--magFilterNearest"))
    {
        filterItem.magFilter = rhi::TEXFILTER_NEAREST;
    }
    if (CommandLineParser::Instance()->IsFlagSet("--magFilterLinear"))
    {
        filterItem.magFilter = rhi::TEXFILTER_LINEAR;
    }
    if (CommandLineParser::Instance()->IsFlagSet("--minFilterNearest"))
    {
        filterItem.minFilter = rhi::TEXFILTER_NEAREST;
    }
    else if (CommandLineParser::Instance()->IsFlagSet("--minFilterLinear"))
    {
        filterItem.minFilter = rhi::TEXFILTER_LINEAR;
    }
    else if (CommandLineParser::Instance()->IsFlagSet("--minFilterNearestMipmapNearest"))
    {
        filterItem.minFilter = rhi::TEXFILTER_NEAREST;
        filterItem.mipFilter = rhi::TEXMIPFILTER_NEAREST;
    }
    else if (CommandLineParser::Instance()->IsFlagSet("--minFilterLinearMipmapNearest"))
    {
        filterItem.minFilter = rhi::TEXFILTER_LINEAR;
        filterItem.mipFilter = rhi::TEXMIPFILTER_NEAREST;
    }
    else if (CommandLineParser::Instance()->IsFlagSet("--minFilterNearestMipmapLinear"))
    {
        filterItem.minFilter = rhi::TEXFILTER_NEAREST;
        filterItem.mipFilter = rhi::TEXMIPFILTER_LINEAR;
    }
    else if (CommandLineParser::Instance()->IsFlagSet("--minFilterLinearMipmapLinear"))
    {
        filterItem.minFilter = rhi::TEXFILTER_LINEAR;
        filterItem.mipFilter = rhi::TEXMIPFILTER_LINEAR;
    }

    return filterItem;
}

bool TexturePacker::NeedSquareTextureForCompression(const Vector<ImageExportKeys>& keys)
{
    for (const ImageExportKeys& key : keys)
    {
        bool needSquare = (key.toComressForGPU && PIXEL_FORMATS_WITH_COMPRESSION.find(key.pixelFormat) != PIXEL_FORMATS_WITH_COMPRESSION.end());
        if (needSquare)
        {
            return true;
        }
    }

    return false;
}

bool TexturePacker::CheckFrameSize(const Size2i& spriteSize, const Size2i& frameSize)
{
    bool isSizeCorrect = ((frameSize.dx <= spriteSize.dx) && (frameSize.dy <= spriteSize.dy));

    return isSizeCorrect;
}

void TexturePacker::DrawToFinalImage(ImageExt& finalImage, ImageExt& drawedImage, const SpriteBoundsRect& packedCell, const Rect2i& alphaOffsetRect)
{
    finalImage.DrawImage(packedCell, alphaOffsetRect, &drawedImage);

    if (CommandLineParser::Instance()->IsFlagSet("--debug"))
    {
        finalImage.DrawRect(packedCell.marginsRect, 0xFF0000FF);
    }
}

void TexturePacker::WriteDefinitionString(FILE* fp, const Rect2i& writeRect, const Rect2i& originRect, int textureIndex, const String& frameName)
{
    if (CommandLineParser::Instance()->IsFlagSet("--disableCropAlpha"))
    {
        fprintf(fp, "%d %d %d %d %d %d %d %s\n", writeRect.x, writeRect.y, writeRect.dx, writeRect.dy, 0, 0, textureIndex, frameName.c_str());
    }
    else
    {
        fprintf(fp, "%d %d %d %d %d %d %d %s\n", writeRect.x, writeRect.y, writeRect.dx, writeRect.dy, originRect.x, originRect.y, textureIndex, frameName.c_str());
    }
}

const Set<String>& TexturePacker::GetErrors() const
{
    return errors;
}

void TexturePacker::SetConvertQuality(TextureConverter::eConvertQuality _quality)
{
    quality = _quality;
}

void TexturePacker::AddError(const String& errorMsg)
{
    Logger::Error(errorMsg.c_str());
    errors.insert(errorMsg);
}

void TexturePacker::AddErrors(const Set<String>& errors_)
{
    for (auto& error : errors_)
    {
        AddError(error);
    }
}
};
