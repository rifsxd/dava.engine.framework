#include "TArc/Controls/PropertyPanel/Private/SliderComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/ComponentStructureWrapper.h"
#include "TArc/Controls/Widget.h"
#include "TArc/Controls/IntSpinBox.h"
#include "TArc/Controls/DoubleSpinBox.h"
#include "TArc/Controls/Slider.h"

#include <Base/Type.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedTypeDB.h>

#include <QHBoxLayout>

namespace DAVA
{
Any SliderComponentValue::GetMultipleValue() const
{
    return Any();
}

bool SliderComponentValue::IsValidValueToSet(const Any& newValue, const Any& currentValue) const
{
    if (currentValue.IsEmpty())
    {
        return true;
    }

    return newValue != currentValue;
}

ControlProxy* SliderComponentValue::CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor)
{
    Widget* container = new Widget(parent);
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setSpacing(2);
    layout->setMargin(1);
    container->SetLayout(layout);

    const Type* t = nodes[0]->field.ref.GetValueType();
    bool isFloat = t->IsFloatingPoint();

    if (isFloat == true)
    {
        DoubleSpinBox::Params p(GetAccessor(), GetUI(), GetWindowKey());
        p.fields[DoubleSpinBox::Fields::IsReadOnly] = BaseComponentValue::readOnlyFieldName;
        p.fields[DoubleSpinBox::Fields::ShowSpinArrows].BindConstValue(true);
        p.fields[DoubleSpinBox::Fields::Value] = "value";
        container->AddControl(new DoubleSpinBox(p, GetAccessor(), model, container->ToWidgetCast()));
    }
    else
    {
        IntSpinBox::Params p(GetAccessor(), GetUI(), GetWindowKey());
        p.fields[IntSpinBox::Fields::IsReadOnly] = BaseComponentValue::readOnlyFieldName;
        p.fields[IntSpinBox::Fields::ShowSpinArrows].BindConstValue(true);
        p.fields[IntSpinBox::Fields::Value] = "value";
        container->AddControl(new IntSpinBox(p, GetAccessor(), model, container->ToWidgetCast()));
    }

    {
        Slider::Params p(GetAccessor(), GetUI(), GetWindowKey());
        p.fields[Slider::Fields::Enabled] = "sliderEnabled";
        p.fields[Slider::Fields::Orientation].BindConstValue(Qt::Horizontal);
        p.fields[Slider::Fields::Value] = "value";
        p.fields[Slider::Fields::ImmediateValue] = "immediateValue";
        Slider* slider = new Slider(p, model, container->ToWidgetCast());
        container->AddControl(slider);

        container->ToWidgetCast()->setFocusProxy(slider->ToWidgetCast());
    }

    return container;
}

bool SliderComponentValue::IsSliderEnabled() const
{
    return !IsReadOnly();
}

Any SliderComponentValue::GetSliderValue() const
{
    return GetValue();
}

void SliderComponentValue::SetSliderValue(const Any& v)
{
    GetModifyInterface()->ModifyPropertyValue(nodes, v);
}

void SliderComponentValue::SetImmediateSliderValue(const Any& immV)
{
    Any currentValue = GetValue();
    if (IsValidValueToSet(immV, currentValue))
    {
        for (const std::shared_ptr<PropertyNode>& node : nodes)
        {
            node->cachedValue = immV;
            node->field.ref.SetValueWithCast(immV);
        }
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(SliderComponentValue)
{
    ReflectionRegistrator<SliderComponentValue>::Begin(CreateComponentStructureWrapper<SliderComponentValue>())
    .Field("value", &SliderComponentValue::GetSliderValue, &SliderComponentValue::SetSliderValue)[M::ProxyMetaRequire()]
    .Field("sliderEnabled", &SliderComponentValue::IsSliderEnabled, nullptr)
    .Method("immediateValue", &SliderComponentValue::SetImmediateSliderValue)
    .End();
}
} // namespace DAVA
