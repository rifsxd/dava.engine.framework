#pragma once

#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"

namespace DAVA
{
class TooltipComponentValue : public BaseComponentValue
{
public:
    TooltipComponentValue() = default;

protected:
    Any GetMultipleValue() const override;
    bool IsValidValueToSet(const Any& newValue, const Any& currentValue) const override;
    ControlProxy* CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) override;

    bool IsReadOnly() const override;

    String GetTooltip() const;

    DAVA_VIRTUAL_REFLECTION(TooltipComponentValue, BaseComponentValue);
};
} // namespace DAVA
