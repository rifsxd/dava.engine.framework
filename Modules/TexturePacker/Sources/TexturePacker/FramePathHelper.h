#pragma once

#include <Base/BaseTypes.h>
#include <FileSystem/FilePath.h>

namespace DAVA
{
class FramePathHelper
{
public:
    // Convert the frame name and index to relative PNG frame path.
    static FilePath GetFramePathRelative(const FilePath& nameWithoutExt, int32 frameIndex, const String& extension);

    // The same but with absolute path.
    static FilePath GetFramePathAbsolute(const FilePath& directory, const String& nameWithoutExt, int32 frameIndex, const String& extension);

protected:
    // Format the frame path.
    static String FormatFramePath(const String& fileNameWithoutExt, int32 frameIndex, const String& extension);
};
};
