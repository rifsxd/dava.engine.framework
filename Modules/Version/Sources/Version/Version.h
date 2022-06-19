#pragma once

#include <Base/BaseTypes.h>

namespace DAVA
{
class Version final
{
public:
    static String CreateAppVersion(const String& appName);
    static String GetVersion();
};
}
