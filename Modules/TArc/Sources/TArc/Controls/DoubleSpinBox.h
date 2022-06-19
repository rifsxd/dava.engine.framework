#pragma once

#include "TArc/Controls/ControlDescriptor.h"
#include "TArc/Controls/Private/BaseSpinBox.h"
#include "TArc/Qt/QtString.h"

#include <Base/BaseTypes.h>

#include <QDoubleSpinBox>
#include <QValidator>

namespace DAVA
{
class DoubleSpinBox : public BaseSpinBox<QDoubleSpinBox, double>
{
public:
    using TBase = BaseSpinBox<QDoubleSpinBox, double>;

    enum class Fields : uint32
    {
        Value = TBase::BaseFields::Value, // [ReadOnly, Validator, Range]
        IsReadOnly = TBase::BaseFields::IsReadOnly,
        IsEnabled = TBase::BaseFields::IsEnabled,
        IsVisible = TBase::BaseFields::IsVisible,
        Range = TBase::BaseFields::Range, // Value should be castable to " const M::Range* "
        ShowSpinArrows = TBase::BaseFields::ShowSpinArrows,
        Accuracy,
        FieldCount
    };

    DECLARE_CONTROL_PARAMS(Fields);

    DoubleSpinBox(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    DoubleSpinBox(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    void UpdateControl(const ControlDescriptor& changedFields) override;
    bool FromText(const QString& input, double& output) const override;
    QString ToText(const double value) const override;
    bool IsEqualValue(double v1, double v2) const override;

    QValidator::State TypeSpecificValidate(const QString& input) const override;
    mutable bool sizeHintCalculation = false;
};
} // namespace DAVA
