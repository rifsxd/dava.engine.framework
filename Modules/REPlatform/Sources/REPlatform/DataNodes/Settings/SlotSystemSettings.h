#pragma once

#include <TArc/DataProcessing/SettingsNode.h>
#include <TArc/Qt/QtString.h>

#include <FileSystem/FilePath.h>
#include <Math/Color.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class SlotSystemSettings : public SettingsNode
{
public:
    bool autoGenerateSlotNames = true;
    FilePath lastConfigPath;
    QString lastPresetSaveLoadPath;
    Color slotBoxColor = Color(0.0f, 0.0f, 0.7f, 0.1f);
    Color slotBoxEdgesColor = Color(0.5f, 0.2f, 0.0f, 1.0f);
    float32 pivotPointSize = 0.3f;
    Color slotPivotColor = Color(0.7f, 0.7f, 0.0f, 0.5f);

    DAVA_VIRTUAL_REFLECTION(SlotSystemSettings, SettingsNode);
};
} // namespace DAVA
