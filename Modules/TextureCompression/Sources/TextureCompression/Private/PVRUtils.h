#pragma once

namespace DAVA
{
class Image;
class PVRUtils final
{
public:
    static bool DecompressPVRToRgba(const Image* srcImage, Image* dstImage);
};
}
