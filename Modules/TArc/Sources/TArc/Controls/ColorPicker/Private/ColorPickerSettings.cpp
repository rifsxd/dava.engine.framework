#include "TArc/Controls/ColorPicker/ColorPickerSettings.h"
#include "TArc/Utils/ReflectionHelpers.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

#include <QDataStream>
#include <QColor>

namespace DAVA
{
ColorPickerSettings::ColorPickerSettings()
{
    const int32 nColors = Qt::darkYellow - Qt::black + 1;
    Vector<QColor> colors;
    for (int i = 0; i < nColors; i++)
    {
        colors.push_back(QColor(Qt::GlobalColor(i + Qt::black)));
    }

    QByteArray paletteData;
    QDataStream paletteStream(&paletteData, QIODevice::WriteOnly);

    for (int i = 0; i < nColors; i++)
    {
        paletteStream << colors[i].rgba();
    }

    const M::HiddenField* meta = GetReflectedTypeMeta<M::HiddenField>(ReflectedTypeDB::GetByPointer(this));
    if (meta != nullptr)
    {
        maxMultiplier = 1.0f;
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(ColorPickerSettings)
{
    ReflectionRegistrator<ColorPickerSettings>::Begin()[M::DisplayName("Color Picker"), M::SettingsSortKey(0)]
    .ConstructorByPointer()
    .Field("maxMultiplier", &ColorPickerSettings::maxMultiplier)[M::DisplayName("Maximum multiplier")]
    .Field("customPalette", &ColorPickerSettings::customPalette)[M::HiddenField()]
    .Field("dialogGeometry", &ColorPickerSettings::dialogGeometry)[M::HiddenField()]
    .End();
}
} // namespace DAVA