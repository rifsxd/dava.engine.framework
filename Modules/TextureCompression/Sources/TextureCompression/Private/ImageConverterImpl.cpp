#include "TextureCompression/Private/ImageConverterImpl.h"

#include "TextureCompression/Private/PVRUtils.h"
#include "TextureCompression/Private/NvttUtils.h"
#include "TextureCompression/Private/QualcommUtils.h"

#include <Debug/DVAssert.h>
#include <Render/Image/Image.h>

namespace DAVA
{
bool ImageConverterImpl::CanConvert(PixelFormat srcFormat, PixelFormat dstFormat) const
{
    if (srcFormat == FORMAT_RGBA8888)
    { //compress
        return PixelFormatDescriptor::IsDxtFormat(dstFormat) || PixelFormatDescriptor::IsAtcFormat(dstFormat);
    }

    if (dstFormat == FORMAT_RGBA8888)
    { //Decompress
        return PixelFormatDescriptor::IsDxtFormat(srcFormat) || PixelFormatDescriptor::IsAtcFormat(srcFormat) || PixelFormatDescriptor::IsPVRFormat(srcFormat);
    }

    return false;
}

bool ImageConverterImpl::Convert(const Image* srcImage, Image* dstImage) const
{
    DVASSERT(srcImage != nullptr && dstImage != nullptr);
    if (PixelFormatDescriptor::IsAtcFormat(srcImage->format))
    {
        return QualcommUtils::DecompressAtcToRgba(srcImage, dstImage);
    }
    else if (PixelFormatDescriptor::IsAtcFormat(dstImage->format))
    {
        return QualcommUtils::CompressRgbaToAtc(srcImage, dstImage);
    }
    else if (PixelFormatDescriptor::IsDxtFormat(srcImage->format))
    {
        return NvttUtils::DecompressDxtToRgba(srcImage, dstImage);
    }
    else if (PixelFormatDescriptor::IsDxtFormat(dstImage->format))
    {
        return NvttUtils::CompressRgbaToDxt(srcImage, dstImage);
    }
    else if (PixelFormatDescriptor::IsPVRFormat(srcImage->format))
    {
        return PVRUtils::DecompressPVRToRgba(srcImage, dstImage);
    }
    else if (PixelFormatDescriptor::IsPVRFormat(dstImage->format))
    {
        //compression to PVR will be done at PVRConverter class via PVRTexTool
        return false;
    }

    return false;
}

} //DAVA
