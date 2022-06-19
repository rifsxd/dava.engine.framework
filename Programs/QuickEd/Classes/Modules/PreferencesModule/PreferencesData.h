#pragma once

#include <TArc/DataProcessing/SettingsNode.h>

class PreferencesData : public DAVA::SettingsNode
{
public:
    bool IsGuidesEnabled() const;

    static DAVA::FastName guidesEnabledPropertyName;

private:
    void SetGuidesEnabled(bool value);
    bool guidesEnabled = true;

    DAVA_VIRTUAL_REFLECTION(PreferencesData, DAVA::SettingsNode);
};
