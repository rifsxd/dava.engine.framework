#include "TArc/Controls/ProgressBar.h"

namespace DAVA
{
ProgressBar::ProgressBar(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QProgressBar>(params, ControlDescriptor(params.fields), wrappersProcessor, model, parent)
{
}

ProgressBar::ProgressBar(const Params& params, Reflection model, QWidget* parent)
    : ControlProxyImpl<QProgressBar>(params, ControlDescriptor(params.fields), params.accessor, model, parent)
{
}

void ProgressBar::UpdateControl(const ControlDescriptor& changedFields)
{
    Reflection valueField = model.GetField(GetFieldName(Fields::Value));
    DVASSERT(valueField.IsValid());

    int minV = std::numeric_limits<int>::lowest();
    int maxV = std::numeric_limits<int>::max();

    const M::Range* rangeMeta = nullptr;
    FastName rangeFieldName = GetFieldName(Fields::Range);
    rangeMeta = GetFieldValue<const M::Range*>(Fields::Range, nullptr);

    if (rangeMeta == nullptr)
    {
        rangeMeta = valueField.GetMeta<M::Range>();
    }

    if (rangeMeta != nullptr)
    {
        minV = rangeMeta->minValue.Cast<int>(minV);
        maxV = rangeMeta->maxValue.Cast<int>(maxV);
    }

    if (minV != minimum() || maxV != maximum())
    {
        setRange(minV, maxV);
    }

    if (changedFields.IsChanged(Fields::Value))
    {
        Reflection field = model.GetField(changedFields.GetName(Fields::Value));
        DVASSERT(field.IsValid());

        Any value = field.GetValue();
        if (value.CanCast<int>())
        {
            int v = value.Cast<int>();
            setValue(v);
        }
        else
        {
            setValue(minimum());
        }
    }

    if (changedFields.IsChanged(Fields::Format))
    {
        Reflection field = model.GetField(changedFields.GetName(Fields::Format));
        DVASSERT(field.IsValid());

        Any value = field.GetValue();
        if (value.CanCast<String>())
        {
            String v = value.Cast<String>();
            setFormat(QString::fromStdString(v));
        }
    }
}
} //namespace DAVA
