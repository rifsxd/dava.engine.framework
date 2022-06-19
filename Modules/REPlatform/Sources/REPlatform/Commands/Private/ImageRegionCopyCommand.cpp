#include "REPlatform/Commands/ImageRegionCopyCommand.h"

#include <Render/Image/Image.h>
#include <Render/Image/ImageSystem.h>

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
ImageRegionCopyCommand::ImageRegionCopyCommand(Image* _dst, const Vector2& dstPos, Image* src, const Rect& srcRect, FilePath _savePath, Image* _orig)
    : RECommand("Remove entity")
    , dst(_dst)
    , orig(nullptr)
    , copy(nullptr)
    , pos(dstPos)
    , savePath(_savePath)
{
    SafeRetain(dst);

    if (nullptr != src && nullptr != dst)
    {
        if (nullptr != _orig)
        {
            DVASSERT(_orig->width == srcRect.dx);
            DVASSERT(_orig->height == srcRect.dy);

            orig = _orig;
        }
        else
        {
            orig = Image::CopyImageRegion((const Image*)dst, Rect(dstPos.x, dstPos.y, srcRect.dx, srcRect.dy));
        }

        copy = Image::CopyImageRegion((const Image*)src, srcRect);
    }
}

ImageRegionCopyCommand::~ImageRegionCopyCommand()
{
    SafeRelease(dst);
    SafeRelease(copy);
    SafeRelease(orig);
}

void ImageRegionCopyCommand::Undo()
{
    if (nullptr != dst && nullptr != orig)
    {
        dst->InsertImage(orig, pos, Rect(0, 0, (float32)orig->width, (float32)orig->height));
        if (!savePath.IsEmpty())
        {
            ImageSystem::Save(savePath, dst);
        }
    }
}

void ImageRegionCopyCommand::Redo()
{
    if (nullptr != dst && nullptr != copy)
    {
        dst->InsertImage(copy, pos, Rect(0, 0, (float32)copy->width, (float32)copy->height));
        if (!savePath.IsEmpty())
        {
            ImageSystem::Save(savePath, dst);
        }
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(ImageRegionCopyCommand)
{
    ReflectionRegistrator<ImageRegionCopyCommand>::Begin()
    .End();
}
} // namespace DAVA
