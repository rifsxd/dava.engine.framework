#include "Version/Version.h"
#include "Version/Private/VersionDefine.h"
#include <Utils/StringFormat.h>

namespace DAVA
{
String Version::CreateAppVersion(const String& appName)
{
    return Format("%s | %s [%u bit]", appName.c_str(), APPLICATION_BUILD_VERSION, static_cast<uint32>(sizeof(pointer_size) * 8));
}

String Version::GetVersion()
{
    return APPLICATION_BUILD_VERSION;
}
}
