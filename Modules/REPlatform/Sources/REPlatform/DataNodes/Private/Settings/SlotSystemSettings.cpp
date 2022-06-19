#include "REPlatform/DataNodes/Settings/SlotSystemSettings.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(SlotSystemSettings)
{
    ReflectionRegistrator<SlotSystemSettings>::Begin()[M::DisplayName("Slot system"), M::SettingsSortKey(80)]
    .ConstructorByPointer()
    .Field("autoGenerateSlotNames", &SlotSystemSettings::autoGenerateSlotNames)[M::DisplayName("Generate slot's name automatically")]
    .Field("lastConfigPath", &SlotSystemSettings::lastConfigPath)[M::HiddenField()]
    .Field("lastPresetSaveLoadPath", &SlotSystemSettings::lastPresetSaveLoadPath)[M::HiddenField()]
    .Field("slotBoxColor", &SlotSystemSettings::slotBoxColor)[M::DisplayName("Box color")]
    .Field("slotBoxEdgesColor", &SlotSystemSettings::slotBoxEdgesColor)[M::DisplayName("Edge color")]
    .Field("pivotPointSize", &SlotSystemSettings::pivotPointSize)[M::DisplayName("Pivot point radius"), M::Range(0.0f, Any(), 0.1f)]
    .Field("slotPivotColor", &SlotSystemSettings::slotPivotColor)[M::DisplayName("Pivot color")]
    .End();
}
} // namespace DAVA
