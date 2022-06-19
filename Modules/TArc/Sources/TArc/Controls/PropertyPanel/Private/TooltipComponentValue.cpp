#include "TArc/Controls/PropertyPanel/Private/TooltipComponentValue.h"
#include "TArc/Controls/CommonStrings.h"
#include "TArc/Controls/Label.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
Any TooltipComponentValue::GetMultipleValue() const
{
    return Any(MultipleValuesString);
}

bool TooltipComponentValue::IsValidValueToSet(const Any& newValue, const Any& currentValue) const
{
    return false;
}

ControlProxy* TooltipComponentValue::CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor)
{
    Label::Params params(GetAccessor(), GetUI(), GetWindowKey());
    params.fields[Label::Fields::Text] = "tooltip";
    return new Label(params, wrappersProcessor, model, parent);
}

bool TooltipComponentValue::IsReadOnly() const
{
    return true;
}

String TooltipComponentValue::GetTooltip() const
{
    String tooltip;
    for (const std::shared_ptr<PropertyNode>& node : nodes)
    {
        const M::Tooltip* tooltipMeta = node->field.ref.GetMeta<M::Tooltip>();
        if (tooltipMeta == nullptr)
        {
            return MultipleValuesString;
        }

        Reflection r = node->field.ref.GetField(FastName(tooltipMeta->tooltipFieldName));
        if (r.IsValid() == false)
        {
            continue;
        }

        String currentToolTip = r.GetValue().Cast<String>("");
        if (tooltip.empty() == true)
        {
            tooltip = currentToolTip;
        }
        else if (tooltip != currentToolTip)
        {
            return MultipleValuesString;
        }
    }

    return tooltip;
}

DAVA_VIRTUAL_REFLECTION_IMPL(TooltipComponentValue)
{
    ReflectionRegistrator<TooltipComponentValue>::Begin()
    .Field("tooltip", &TooltipComponentValue::GetTooltip, nullptr)
    .End();
}
} // namespace DAVA
