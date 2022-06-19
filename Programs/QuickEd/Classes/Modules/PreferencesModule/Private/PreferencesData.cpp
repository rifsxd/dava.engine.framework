#include "Modules/PreferencesModule/PreferencesData.h"

DAVA_VIRTUAL_REFLECTION_IMPL(PreferencesData)
{
    DAVA::ReflectionRegistrator<PreferencesData>::Begin()[DAVA::M::HiddenField()]
    .ConstructorByPointer()
    .Field(guidesEnabledPropertyName.c_str(), &PreferencesData::IsGuidesEnabled, &PreferencesData::SetGuidesEnabled)
    .End();
}

bool PreferencesData::IsGuidesEnabled() const
{
    return guidesEnabled;
}

void PreferencesData::SetGuidesEnabled(bool value)
{
    guidesEnabled = value;
}

DAVA::FastName PreferencesData::guidesEnabledPropertyName{ "guides enabled" };
