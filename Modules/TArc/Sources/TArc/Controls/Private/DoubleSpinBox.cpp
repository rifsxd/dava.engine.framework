#include "TArc/Controls/DoubleSpinBox.h"
#include "TArc/Utils/ScopedValueGuard.h"

#include "TArc/Utils/StringFormatingUtils.h"

namespace DAVA
{
DoubleSpinBox::DoubleSpinBox(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent /*= nullptr*/)
    : TBase(params, ControlDescriptor(params.fields), wrappersProcessor, model, parent)
{
    setDecimals(6);
}

DoubleSpinBox::DoubleSpinBox(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent /*= nullptr*/)
    : TBase(params, ControlDescriptor(params.fields), accessor, model, parent)
{
    setDecimals(6);
}

void DoubleSpinBox::UpdateControl(const ControlDescriptor& changedFields)
{
    if (changedFields.IsChanged(Fields::Accuracy))
    {
        Reflection r = model.GetField(changedFields.GetName(Fields::Accuracy));
        DVASSERT(r.IsValid());
        Any value = r.GetValue();
        if (value.CanGet<M::FloatNumberAccuracy*>())
        {
            setDecimals(value.Get<M::FloatNumberAccuracy*>()->accuracy);
        }
        setDecimals(r.GetValue().Cast<int>());
    }
    else if (changedFields.IsChanged(Fields::Value))
    {
        Reflection r = model.GetField(changedFields.GetName(Fields::Value));
        DVASSERT(r.IsValid());
        const M::FloatNumberAccuracy* meta = r.GetMeta<M::FloatNumberAccuracy>();
        if (meta != nullptr)
        {
            setDecimals(meta->accuracy);
        }
    }

    TBase::UpdateControl(changedFields);
}

bool DoubleSpinBox::FromText(const QString& input, double& output) const
{
    if (input == ".")
    {
        output = 0.0;
        return true;
    }
    bool isOk = false;
    output = input.toDouble(&isOk);
    return isOk;
}

QString DoubleSpinBox::ToText(const double value) const
{
    QString result;
    FloatToString(value, decimals(), result);

    // for size hint calculation we bound maximum size of text to 6 digit,
    // because default implementation of QDoubleSpinBox calculate sizeHint according to range (minimum value and maximum value - about 40 signs)
    if (sizeHintCalculation == true && result.size() > 6)
    {
        result = "-999.9";
    }

    return result;
}

bool DoubleSpinBox::IsEqualValue(double v1, double v2) const
{
    double diff = Abs(v1 - v2);
    double accuracy = std::pow(10.0, -static_cast<double>(decimals()));
    return diff < accuracy;
}

QValidator::State DoubleSpinBox::TypeSpecificValidate(const QString& input) const
{
    if (input[0] == QChar('-'))
    {
        if (minimum() >= 0)
        {
            return QValidator::Invalid;
        }

        int inputSize = input.size();
        if (inputSize == 1)
        {
            return QValidator::Intermediate;
        }
    }

    return QValidator::Acceptable;
}

QSize DoubleSpinBox::sizeHint() const
{
    QSize s;
    {
        ScopedValueGuard<bool> guard(sizeHintCalculation, true);
        s = TBase::sizeHint();
    }

    return s;
}

QSize DoubleSpinBox::minimumSizeHint() const
{
    QSize s;
    {
        ScopedValueGuard<bool> guard(sizeHintCalculation, true);
        s = TBase::minimumSizeHint();
    }

    return s;
}
} // namespace DAVA
