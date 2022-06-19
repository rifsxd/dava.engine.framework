#pragma once

#include "TArc/DataProcessing/SettingsNode.h"
#include "TArc/Qt/QtByteArray.h"
#include "TArc/Qt/QtRect.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
class ColorPickerSettings : public SettingsNode
{
public:
    ColorPickerSettings();

    float32 maxMultiplier = 2.0;
    QByteArray customPalette;
    QRect dialogGeometry;

    DAVA_VIRTUAL_REFLECTION(ColorPickerSettings, SettingsNode);
};
} // namespace DAVA