#include "Classes/Modules/PackageModule/PackageWidgetSettings.h"

DAVA_VIRTUAL_REFLECTION_IMPL(PackageWidgetSettings)
{
    DAVA::ReflectionRegistrator<PackageWidgetSettings>::Begin()[DAVA::M::DisplayName("Package Widget")]
    .ConstructorByPointer()
    .Field("selectedDevice", &PackageWidgetSettings::selectedDevice)[DAVA::M::HiddenField()]
    .Field("selectedBlank", &PackageWidgetSettings::selectedBlank)[DAVA::M::HiddenField()]
    .Field("flowFlag", &PackageWidgetSettings::flowFlag)[DAVA::M::HiddenField()]
    .Field("useCustomUIViewerPath", &PackageWidgetSettings::useCustomUIViewerPath)[DAVA::M::DisplayName("Override UIViewer Path")]
    .Field("customUIViewerPath", &PackageWidgetSettings::customUIViewerPath)[DAVA::M::DisplayName("UIViewer path")]
    .End();
}