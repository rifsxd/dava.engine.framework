#include "REPlatform/Scene/Utils/ImageTools.h"

#include <Base/GlobalEnum.h>
#include <FileSystem/FileSystem.h>
#include <Render/GPUFamilyDescriptor.h>
#include <Render/Image/ImageConvert.h>
#include <Render/Image/ImageSystem.h>
#include <Render/PixelFormatDescriptor.h>

namespace DAVA
{
namespace ImageTools
{
void SaveImage(Image* image, const FilePath& pathname)
{
    ImageSystem::Save(pathname, image, image->format);
}

Image* LoadImage(const FilePath& pathname)
{
    return ImageSystem::LoadSingleMip(pathname);
}

uint32 GetTexturePhysicalSize(const TextureDescriptor* descriptor, const eGPUFamily forGPU, uint32 baseMipMaps)
{
    uint32 size = 0;

    Vector<FilePath> files;

    if (descriptor->IsCubeMap() && forGPU == GPU_ORIGIN)
    {
        Vector<FilePath> faceNames;
        descriptor->GetFacePathnames(faceNames);

        files.reserve(faceNames.size());
        for (auto& faceName : faceNames)
        {
            if (!faceName.IsEmpty())
                files.push_back(faceName);
        }
    }
    else
    {
        descriptor->CreateLoadPathnamesForGPU(forGPU, files);
    }

    FileSystem* fileSystem = FileSystem::Instance();
    for (const FilePath& imagePathname : files)
    {
        if (fileSystem->Exists(imagePathname) && fileSystem->IsFile(imagePathname))
        {
            ImageInfo info = ImageSystem::GetImageInfo(imagePathname);
            if (!info.IsEmpty())
            {
                uint32 m = Min(baseMipMaps, info.mipmapsCount - 1);
                for (; m < info.mipmapsCount; ++m)
                {
                    uint32 w = Max(info.width >> m, 1u);
                    uint32 h = Max(info.height >> m, 1u);
                    size += ImageUtils::GetSizeInBytes(w, h, info.format);
                }
            }
            else
            {
                Logger::Error("ImageTools::[GetTexturePhysicalSize] Can't detect type of file %s", imagePathname.GetStringValue().c_str());
            }
        }
    }

    return size;
}

void ConvertImage(const TextureDescriptor* descriptor, const eGPUFamily forGPU, TextureConverter::eConvertQuality quality)
{
    if (!descriptor || descriptor->compression[forGPU].format == FORMAT_INVALID)
    {
        return;
    }

    TextureConverter::ConvertTexture(*descriptor, forGPU, true, quality);
}

bool SplitImage(const FilePath& pathname)
{
    ScopedPtr<Image> loadedImage(ImageSystem::LoadSingleMip(pathname));
    if (!loadedImage)
    {
        Logger::Error("Can't load image %s", pathname.GetAbsolutePathname().c_str());
        return false;
    }

    if (loadedImage->GetPixelFormat() != FORMAT_RGBA8888)
    {
        Logger::Error("Incorrect image format %s. Must be RGBA8888", PixelFormatDescriptor::GetPixelFormatString(loadedImage->GetPixelFormat()));
        return false;
    }

    Channels channels = CreateSplittedImages(loadedImage);

    FilePath folder(pathname.GetDirectory());

    SaveImage(channels.red, folder + "r.png");
    SaveImage(channels.green, folder + "g.png");
    SaveImage(channels.blue, folder + "b.png");
    SaveImage(channels.alpha, folder + "a.png");

    return true;
}

bool MergeImages(const FilePath& folder)
{
    DVASSERT(folder.IsDirectoryPathname());

    ScopedPtr<Image> r(LoadImage(folder + "r.png"));
    ScopedPtr<Image> g(LoadImage(folder + "g.png"));
    ScopedPtr<Image> b(LoadImage(folder + "b.png"));
    ScopedPtr<Image> a(LoadImage(folder + "a.png"));
    Channels channels(r, g, b, a);

    if (channels.IsEmpty())
    {
        Logger::Error("Can't load one or more channel images from folder %s", folder.GetAbsolutePathname().c_str());
        return false;
    }

    if (!channels.HasFormat(FORMAT_A8))
    {
        Logger::Error("Can't merge images. Source format must be Grayscale 8bit");
        return false;
    }

    if (!channels.ChannelesResolutionEqual())
    {
        Logger::Error("Can't merge images. Source images must have same size");
        return false;
    }

    ScopedPtr<Image> mergedImage(CreateMergedImage(channels));

    ImageSystem::Save(folder + "merged.png", mergedImage);
    return true;
}

Channels CreateSplittedImages(Image* originalImage)
{
    ScopedPtr<Image> r(Image::Create(originalImage->width, originalImage->height, FORMAT_A8));
    ScopedPtr<Image> g(Image::Create(originalImage->width, originalImage->height, FORMAT_A8));
    ScopedPtr<Image> b(Image::Create(originalImage->width, originalImage->height, FORMAT_A8));
    ScopedPtr<Image> a(Image::Create(originalImage->width, originalImage->height, FORMAT_A8));

    int32 size = originalImage->width * originalImage->height;
    int32 pixelSizeInBytes = PixelFormatDescriptor::GetPixelFormatSizeInBits(FORMAT_RGBA8888) / 8;
    for (int32 i = 0; i < size; ++i)
    {
        int32 offset = i * pixelSizeInBytes;
        r->data[i] = originalImage->data[offset];
        g->data[i] = originalImage->data[offset + 1];
        b->data[i] = originalImage->data[offset + 2];
        a->data[i] = originalImage->data[offset + 3];
    }
    return Channels(r, g, b, a);
}

Image* CreateMergedImage(const Channels& channels)
{
    if (!channels.ChannelesResolutionEqual() || !channels.HasFormat(FORMAT_A8))
    {
        return nullptr;
    }
    Image* mergedImage = Image::Create(channels.red->width, channels.red->height, FORMAT_RGBA8888);
    int32 size = mergedImage->width * mergedImage->height;
    int32 pixelSizeInBytes = PixelFormatDescriptor::GetPixelFormatSizeInBits(FORMAT_RGBA8888) / 8;
    for (int32 i = 0; i < size; ++i)
    {
        int32 offset = i * pixelSizeInBytes;
        mergedImage->data[offset] = channels.red->data[i];
        mergedImage->data[offset + 1] = channels.green->data[i];
        mergedImage->data[offset + 2] = channels.blue->data[i];
        mergedImage->data[offset + 3] = channels.alpha->data[i];
    }
    return mergedImage;
}

void SetChannel(Image* image, eComponentsRGBA channel, uint8 value)
{
    if (image->format != FORMAT_RGBA8888)
    {
        return;
    }
    int32 size = image->width * image->height;
    static const int32 pixelSizeInBytes = 4;
    int32 offset = channel;
    for (int32 i = 0; i < size; ++i, offset += pixelSizeInBytes)
    {
        image->data[offset] = value;
    }
}

QImage FromDavaImage(const FilePath& pathname)
{
    auto image = LoadImage(pathname);
    if (image)
    {
        QImage img = FromDavaImage(image);
        SafeRelease(image);

        return img;
    }

    return QImage();
}

QImage FromDavaImage(const Image* image)
{
    DVASSERT(image != nullptr);

    if (image->format == FORMAT_RGBA8888)
    {
        QImage qtImage(image->width, image->height, QImage::Format_RGBA8888);
        Memcpy(qtImage.bits(), image->data, image->dataSize);
        return qtImage;
    }
    else if (ImageConvert::CanConvertFromTo(image->format, FORMAT_RGBA8888))
    {
        ScopedPtr<Image> newImage(Image::Create(image->width, image->height, FORMAT_RGBA8888));
        bool converted = ImageConvert::ConvertImage(image, newImage);
        if (converted)
        {
            return FromDavaImage(newImage);
        }
        else
        {
            Logger::Error("[%s]: Error converting from %s", __FUNCTION__, GlobalEnumMap<PixelFormat>::Instance()->ToString(image->format));
            return QImage();
        }
    }
    else
    {
        Logger::Error("[%s]: Converting from %s is not implemented", __FUNCTION__, GlobalEnumMap<PixelFormat>::Instance()->ToString(image->format));
        return QImage();
    }
}

} // namespace ImageTools
} // namespace DAVA
