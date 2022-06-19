#include "TArc/Controls/CheckBox.h"

#include <Base/FastName.h>
#include <Reflection/ReflectedMeta.h>

namespace DAVA
{
CheckBox::CheckBox(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QCheckBox>(params, ControlDescriptor(params.fields), wrappersProcessor, model, parent)
{
    SetupControl();
}

CheckBox::CheckBox(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QCheckBox>(params, ControlDescriptor(params.fields), accessor, model, parent)
{
    SetupControl();
}

void CheckBox::SetupControl()
{
    connections.AddConnection(this, &QCheckBox::stateChanged, MakeFunction(this, &CheckBox::StateChanged));
}

void CheckBox::UpdateControl(const ControlDescriptor& changedFields)
{
    Reflection fieldValue = model.GetField(changedFields.GetName(Fields::Checked));
    DVASSERT(fieldValue.IsValid());

    bool readOnly = IsValueReadOnly(changedFields, Fields::Checked, Fields::IsReadOnly);
    bool enabled = GetFieldValue(Fields::IsEnabled, true);
    setEnabled(!readOnly && enabled);

    const M::ValueDescription* valueDescriptor = fieldValue.GetMeta<M::ValueDescription>();
    if (valueDescriptor != nullptr)
    {
        setText(QString::fromStdString(valueDescriptor->GetDescription(fieldValue.GetValue())));
    }
    else if (changedFields.IsChanged(Fields::TextHint) == true)
    {
        String hint = GetFieldValue(Fields::TextHint, String(""));
        setText(QString::fromStdString(hint));
    }

    if (changedFields.IsChanged(Fields::Checked) == true)
    {
        if (fieldValue.GetValue().CanGet<Qt::CheckState>())
        {
            dataType = eContainedDataType::TYPE_CHECK_STATE;

            Qt::CheckState state = fieldValue.GetValue().Cast<Qt::CheckState>();
            setTristate(state == Qt::PartiallyChecked);
            setCheckState(state);
        }
        else if (fieldValue.GetValue().CanCast<bool>())
        {
            dataType = eContainedDataType::TYPE_BOOL;
            Qt::CheckState state = fieldValue.GetValue().Cast<bool>() ? Qt::Checked : Qt::Unchecked;
            setTristate(false);
            setCheckState(state);
        }
        else
        {
            DVASSERT(false);
        }
    }
}

void CheckBox::StateChanged(int newState)
{
    if (isEnabled() == false)
    {
        return;
    }

    if (newState != Qt::PartiallyChecked)
    {
        Any currentValue = GetFieldValue(Fields::Checked, Any());
        setTristate(false);
        if (dataType == eContainedDataType::TYPE_CHECK_STATE)
        {
            Any newValue = Any(checkState());
            if (currentValue != newValue)
            {
                wrapper.SetFieldValue(GetFieldName(Fields::Checked), newValue);
            }
        }
        else if (dataType == eContainedDataType::TYPE_BOOL)
        {
            Qt::CheckState checkStateValue = checkState();
            DVASSERT(checkStateValue != Qt::PartiallyChecked);
            Any newValue = Any(checkStateValue == Qt::Checked ? true : false);
            if (currentValue != newValue)
            {
                wrapper.SetFieldValue(GetFieldName(Fields::Checked), newValue);
            }
        }
        else
        {
            DVASSERT(false);
        }

        Reflection fieldValue = model.GetField(GetFieldName(Fields::Checked));
        const M::ValueDescription* valueDescriptor = fieldValue.GetMeta<M::ValueDescription>();
        if (valueDescriptor != nullptr)
        {
            setText(QString::fromStdString(valueDescriptor->GetDescription(fieldValue.GetValue())));
        }
    }
}
} // namespace DAVA
