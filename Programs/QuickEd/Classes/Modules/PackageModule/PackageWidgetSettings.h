#pragma once

#include <TArc/DataProcessing/SettingsNode.h>

class PackageWidgetSettings : public DAVA::SettingsNode
{
public:
    DAVA::uint32 selectedDevice = 0;
    DAVA::uint32 selectedBlank = 0;
    bool flowFlag = false;

    bool useCustomUIViewerPath = false;
    DAVA::String customUIViewerPath;

    DAVA_VIRTUAL_REFLECTION(PackageWidgetSettings, DAVA::SettingsNode);
};