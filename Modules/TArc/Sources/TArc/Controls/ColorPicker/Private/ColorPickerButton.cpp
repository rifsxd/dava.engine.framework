#include "TArc/Controls/ColorPicker/ColorPickerButton.h"
#include "TArc/Controls/ColorPicker/ColorPickerDialog.h"
#include "TArc/Qt/QtIcon.h"

#include <Base/FastName.h>
#include <Reflection/ReflectedMeta.h>

#include <QColor>
#include <QPalette>

namespace DAVA
{
ColorPickerButton::ColorPickerButton(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QToolButton>(params, ControlDescriptor(params.fields), wrappersProcessor, model, parent)
{
    SetupControl();
}

ColorPickerButton::ColorPickerButton(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QToolButton>(params, ControlDescriptor(params.fields), accessor, model, parent)
{
    DVASSERT(accessor == params.accessor);
    SetupControl();
}

void ColorPickerButton::SetupControl()
{
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    connections.AddConnection(this, &QToolButton::released, MakeFunction(this, &ColorPickerButton::ButtonReleased));
}

void ColorPickerButton::UpdateControl(const ControlDescriptor& changedFields)
{
    Reflection fieldValue = model.GetField(changedFields.GetName(Fields::Color));
    DVASSERT(fieldValue.IsValid());

    if (changedFields.IsChanged(Fields::Range) == true)
    {
        rangeMeta = GetFieldValue<const M::Range*>(Fields::Range, nullptr);
    }

    if (rangeMeta == nullptr)
    {
        rangeMeta = fieldValue.GetMeta<M::Range>();
    }

    readOnly = IsValueReadOnly(changedFields, Fields::Color, Fields::IsReadOnly);
    if (changedFields.IsChanged(Fields::Color) == true)
    {
        cachedColor = fieldValue.GetValue();
        setIcon(cachedColor.Cast<QIcon>(QIcon()));
    }
}

void ColorPickerButton::mousePressEvent(QMouseEvent* e)
{
    if (readOnly == false)
    {
        QToolButton::mousePressEvent(e);
    }
}

void ColorPickerButton::mouseReleaseEvent(QMouseEvent* e)
{
    if (readOnly == false)
    {
        QToolButton::mouseReleaseEvent(e);
    }
}

void ColorPickerButton::OnColorChanging(Color colorValue, ColorPickerButton::Fields typeField)
{
    if (rangeMeta != nullptr)
    {
        auto clampValue = [](float32 prevV, float32 newV, float32 minV, float32 maxV, float32 stepV)
        {
            float32 halfStep = stepV / 2.0f;
            if (newV > prevV)
            {
                newV += halfStep;
            }
            else
            {
                newV -= halfStep;
            }

            int32 stepCount = static_cast<int32>((newV - prevV) / stepV);
            newV = prevV + stepCount * stepV;

            return Clamp(newV, minV, maxV);
        };

        Color minValue = rangeMeta->minValue.Get<Color>();
        Color maxValue = rangeMeta->maxValue.Get<Color>();
        Color stepValue = rangeMeta->step.Get<Color>();

        Color prevColorValue = cachedColor.Get<Color>();

        colorValue.r = clampValue(prevColorValue.r, colorValue.r, minValue.r, maxValue.r, stepValue.r);
        colorValue.g = clampValue(prevColorValue.g, colorValue.g, minValue.g, maxValue.g, stepValue.g);
        colorValue.b = clampValue(prevColorValue.b, colorValue.b, minValue.b, maxValue.b, stepValue.b);
        colorValue.a = clampValue(prevColorValue.a, colorValue.a, minValue.a, maxValue.a, stepValue.a);
    }

    FastName fieldName = GetFieldName(typeField);
    if (fieldName.IsValid() == false)
    {
        return;
    }

    cachedColor = colorValue;
    setIcon(cachedColor.Cast<QIcon>(QIcon()));
    wrapper.SetFieldValue(fieldName, colorValue);
}

void ColorPickerButton::ButtonReleased()
{
    if (readOnly)
    {
        return;
    }

    ColorPickerDialog cp(controlParams.accessor);
    cp.setWindowTitle("Select color");

    Color prevColorValue = cachedColor.Get<Color>();
    cp.SetDavaColor(prevColorValue);

    auto colorChangingCallFn = [this, &cp]()
    {
        OnColorChanging(cp.GetDavaColor(), Fields::IntermediateColor);
    };

    connections.AddConnection(&cp, &ColorPickerDialog::changing, colorChangingCallFn);
    connections.AddConnection(&cp, &ColorPickerDialog::changed, colorChangingCallFn);

    bool changed = cp.Exec();

    OnColorChanging(prevColorValue, Fields::IntermediateColor);

    if (changed)
    {
        OnColorChanging(cp.GetDavaColor(), Fields::Color);
    }
}
} // namespace DAVA
