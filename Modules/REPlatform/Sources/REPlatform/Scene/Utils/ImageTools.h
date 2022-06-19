#pragma once

#include <TextureCompression/TextureConverter.h>

#include <FileSystem/FilePath.h>
#include <Render/Image/Image.h>
#include <Render/TextureDescriptor.h>

#include <QImage>

namespace DAVA
{
struct Channels
{
    ScopedPtr<Image> red;
    ScopedPtr<Image> green;
    ScopedPtr<Image> blue;
    ScopedPtr<Image> alpha;

    Channels(const ScopedPtr<Image>& red_, const ScopedPtr<Image>& green_, const ScopedPtr<Image>& blue_, const ScopedPtr<Image>& alpha_)
        : red(red_)
        , green(green_)
        , blue(blue_)
        , alpha(alpha_)
    {
    }

    inline bool IsEmpty() const
    {
        return (!red || !green || !blue || !alpha);
    }

    inline bool HasFormat(PixelFormat format) const
    {
        return (red->GetPixelFormat() == format &&
                green->GetPixelFormat() == format &&
                blue->GetPixelFormat() == format &&
                alpha->GetPixelFormat() == format);
    }

    inline bool ChannelesResolutionEqual() const
    {
        return (red->width == green->width && red->width == blue->width && red->width == alpha->width) &&
        (red->height == green->height && red->height == blue->height && red->height == alpha->height);
    }
};

namespace ImageTools
{
enum eComponentsRGBA
{
    COLOR_RED = 0,
    COLOR_GREEN,
    COLOR_BLUE,
    COLOR_ALPHA,
};

uint32 GetTexturePhysicalSize(const TextureDescriptor* descriptor, const eGPUFamily forGPU, uint32 baseMipMaps = 0);
void ConvertImage(const TextureDescriptor* descriptor, const eGPUFamily forGPU, TextureConverter::eConvertQuality quality);

bool SplitImage(const FilePath& pathname);

bool MergeImages(const FilePath& folder);

Channels CreateSplittedImages(Image* originalImage);

Image* CreateMergedImage(const Channels& channes);

void SetChannel(Image* image, eComponentsRGBA channel, uint8 value);

QImage FromDavaImage(const FilePath& pathname);
QImage FromDavaImage(const Image* image);
}
} // namespace DAVA
