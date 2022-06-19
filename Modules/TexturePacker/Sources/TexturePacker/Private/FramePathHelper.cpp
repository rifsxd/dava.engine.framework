#include "TexturePacker/FramePathHelper.h"
#include <Utils/StringFormat.h>

namespace DAVA
{
FilePath FramePathHelper::GetFramePathRelative(const FilePath& nameWithoutExt, int32 frameIndex, const String& extension)
{
    return FormatFramePath(nameWithoutExt.GetAbsolutePathname(), frameIndex, extension);
}

FilePath FramePathHelper::GetFramePathAbsolute(const FilePath& directory, const String& nameWithoutExt,
                                               int32 frameIndex, const String& extension)
{
    FilePath resultPath(directory, FormatFramePath(nameWithoutExt, frameIndex, extension));
    return resultPath;
}

String FramePathHelper::FormatFramePath(const String& fileNameWithoutExt, int32 frameIndex, const String& extension)
{
    return Format("%s_%d%s", fileNameWithoutExt.c_str(), frameIndex, extension.c_str());
}
};
