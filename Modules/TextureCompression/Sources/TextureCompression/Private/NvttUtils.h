#pragma once

namespace DAVA
{
class Image;
class NvttUtils final
{
public:
    static bool DecompressDxtToRgba(const Image* srcImage, Image* dstImage);
    static bool CompressRgbaToDxt(const Image* srcImage, Image* dstImage);
};
}
