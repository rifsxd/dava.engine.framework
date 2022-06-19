#pragma once

#include <FileSystem/FilePath.h>

namespace DAVA
{
#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
/** return ~/Library/Application Support/DAVAEngine/ path */
FilePath GetApplicationSupportPath();
#endif
}
