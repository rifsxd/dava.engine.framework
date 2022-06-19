#pragma once
#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"

#include <Base/BaseTypes.h>

namespace DAVA
{
class SliderComponentValue : public BaseComponentValue
{
public:
    SliderComponentValue() = default;

protected:
    Any GetMultipleValue() const override;
    bool IsValidValueToSet(const Any& newValue, const Any& currentValue) const override;
    ControlProxy* CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) override;

    bool IsSliderEnabled() const;
    Any GetSliderValue() const;
    void SetSliderValue(const Any& v);

    void SetImmediateSliderValue(const Any& immV);
    DAVA_VIRTUAL_REFLECTION(SliderComponentValue, BaseComponentValue);
};
} // namespace DAVA
