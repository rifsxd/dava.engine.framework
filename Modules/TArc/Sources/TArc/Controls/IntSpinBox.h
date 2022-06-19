#pragma once

#include "TArc/Controls/ControlDescriptor.h"
#include "TArc/Controls/Private/BaseSpinBox.h"
#include "TArc/Qt/QtString.h"

#include <Base/BaseTypes.h>

#include <QSpinBox>
#include <QValidator>

namespace DAVA
{
class IntSpinBox : public BaseSpinBox<QSpinBox, int>
{
public:
    using TBase = BaseSpinBox<QSpinBox, int>;
    enum class Fields : uint32
    {
        Value = TBase::BaseFields::Value, // [ReadOnly, Validator, Range]
        IsReadOnly = TBase::BaseFields::IsReadOnly,
        IsEnabled = TBase::BaseFields::IsEnabled,
        IsVisible = TBase::BaseFields::IsVisible,
        Range = TBase::BaseFields::Range, // Value should be castable to " const M::Range* "
        ShowSpinArrows = TBase::BaseFields::ShowSpinArrows,
        FieldCount
    };

    DECLARE_CONTROL_PARAMS(Fields);
    IntSpinBox(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    IntSpinBox(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

private:
    bool FromText(const QString& input, int& output) const override;
    QString ToText(const int output) const override;
    bool IsEqualValue(int v1, int v2) const override;

    QValidator::State TypeSpecificValidate(const QString& input) const override;
};
} // namespace DAVA
