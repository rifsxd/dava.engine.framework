#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Math/Rect.h>
#include <FileSystem/FilePath.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class Image;
class ImageRegionCopyCommand : public RECommand
{
public:
    ImageRegionCopyCommand(Image* dst, const Vector2& dstPos, Image* src, const Rect& srcRect, FilePath savePath = FilePath(), Image* orig = NULL);
    ~ImageRegionCopyCommand();

    void Undo() override;
    void Redo() override;

    Image* dst;
    Image* orig;
    Image* copy;
    Vector2 pos;
    FilePath savePath;

private:
    DAVA_VIRTUAL_REFLECTION(ImageRegionCopyCommand, RECommand);
};
} // namespace DAVA
