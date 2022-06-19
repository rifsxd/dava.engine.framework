#include "Classes/Modules/DistanceLinesModule/Private/DistanceLinesPreferences.h"

DAVA_VIRTUAL_REFLECTION_IMPL(DistanceSystemPreferences)
{
    DAVA::ReflectionRegistrator<DistanceSystemPreferences>::Begin()[DAVA::M::DisplayName("Distance lines"), DAVA::M::SettingsSortKey(60)]
    .ConstructorByPointer()
    .Field("linesColor", &DistanceSystemPreferences::linesColor)[DAVA::M::DisplayName("Solid line color")]
    .Field("textColor", &DistanceSystemPreferences::textColor)[DAVA::M::DisplayName("Text color")]
    .End();
}
