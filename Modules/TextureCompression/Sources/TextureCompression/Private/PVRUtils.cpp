#include "TextureCompression/Private/PVRUtils.h"

#include <Base/BaseTypes.h>
#include <Logger/Logger.h>
#include <Render/Image/Image.h>
#include <Render/RenderBase.h>
#include <Render/PixelFormatDescriptor.h>

#include <libpvr/PVRTDecompress.h>

namespace DAVA
{
bool PVRUtils::DecompressPVRToRgba(const Image* encodedImage, Image* decodedImage)
{
    decodedImage->mipmapLevel = encodedImage->mipmapLevel;
    decodedImage->cubeFaceID = encodedImage->cubeFaceID;

    uint32 retCode = 0;
    if (encodedImage->format == PixelFormat::FORMAT_PVR2)
    {
        retCode = PVRTDecompressPVRTC(encodedImage->data, 1, encodedImage->width, encodedImage->height, decodedImage->data);
    }
    else if (encodedImage->format == PixelFormat::FORMAT_PVR4)
    {
        retCode = PVRTDecompressPVRTC(encodedImage->data, 0, encodedImage->width, encodedImage->height, decodedImage->data);
    }
    else if (encodedImage->format == PixelFormat::FORMAT_ETC1)
    {
        retCode = PVRTDecompressETC(encodedImage->data, encodedImage->width, encodedImage->height, decodedImage->data, 0);
    }
    else
    {
        Logger::Error("Can't decode PVR: source Image has unknown format %s", GlobalEnumMap<PixelFormat>::Instance()->ToString(encodedImage->format));
        return false;
    }

    return (retCode == encodedImage->dataSize);
}

} //DAVA
