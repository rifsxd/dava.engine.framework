#pragma once

#include <Render/RenderBase.h>
#include <Render/Image/ImageConverter.h>
#include <Render/PixelFormatDescriptor.h>

namespace DAVA
{
class Image;
class ImageConverterImpl : public ImageConverter
{
public:
    bool CanConvert(PixelFormat srcFormat, PixelFormat dstFormat) const;
    bool Convert(const Image* srcImage, Image* dstImage) const;
};

} //DAVA
