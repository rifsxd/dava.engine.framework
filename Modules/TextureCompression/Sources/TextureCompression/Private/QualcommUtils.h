#pragma once

namespace DAVA
{
class Image;
class QualcommUtils final
{
public:
    static bool DecompressAtcToRgba(const Image* image, Image* dstImage);
    static bool CompressRgbaToAtc(const Image* image, Image* dstImage);
};
}
