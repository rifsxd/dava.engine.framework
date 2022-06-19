#pragma once

#include <TArc/DataProcessing/SettingsNode.h>

#include <FileSystem/FilePath.h>
#include <Math/Color.h>

class DistanceSystemPreferences : public DAVA::SettingsNode
{
public:
    DAVA::Color linesColor = DAVA::Color(1.0f, 0.0f, 0.0f, 0.9f);
    DAVA::Color textColor = DAVA::Color(1.0f, 0.0f, 0.0f, 0.9f);

    DAVA_VIRTUAL_REFLECTION(DistanceSystemPreferences, DAVA::SettingsNode);
};
