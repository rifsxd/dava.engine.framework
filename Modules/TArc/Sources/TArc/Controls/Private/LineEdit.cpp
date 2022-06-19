#include "TArc/Controls/LineEdit.h"
#include "TArc/Controls/Private/TextValidator.h"
#include "TArc/Utils/Utils.h"

#include <Base/FastName.h>
#include <Reflection/ReflectedMeta.h>

#include <QBoxLayout>
#include <QPushButton>

namespace DAVA
{
LineEdit::LineEdit(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QLineEdit>(params, ControlDescriptor(params.fields), wrappersProcessor, model, parent)
{
    SetupControl();
}

LineEdit::LineEdit(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QLineEdit>(params, ControlDescriptor(params.fields), accessor, model, parent)
{
    SetupControl();
}

void LineEdit::SetupControl()
{
    connections.AddConnection(this, &QLineEdit::editingFinished, MakeFunction(this, &LineEdit::EditingFinished));
    connections.AddConnection(this, &QLineEdit::textChanged, MakeFunction(this, &LineEdit::TextChanged));
    TextValidator* validator = new TextValidator(this, this);
    setValidator(validator);

    QHBoxLayout* l = new QHBoxLayout();
    l->setContentsMargins(QMargins());
    l->setSpacing(1);
    l->addStretch();
    setLayout(l);
}

void LineEdit::EditingFinished()
{
    RETURN_IF_MODEL_LOST(void());

    if (!isReadOnly())
    {
        String newText = text().toStdString();
        if (GetFieldValue<String>(Fields::Text, "") != newText)
        {
            wrapper.SetFieldValue(GetFieldName(Fields::Text), newText);
        }
    }
}

void LineEdit::UpdateControl(const ControlDescriptor& descriptor)
{
    RETURN_IF_MODEL_LOST(void());
    bool readOnlyChanged = descriptor.IsChanged(Fields::IsReadOnly);
    bool textChanged = descriptor.IsChanged(Fields::Text);
    if (readOnlyChanged || textChanged)
    {
        setReadOnly(IsValueReadOnly(descriptor, Fields::Text, Fields::IsReadOnly));

        if (textChanged)
        {
            QString newText = QString::fromStdString(GetFieldValue(Fields::Text, DAVA::String()));
            if (newText != text())
            {
                setText(newText);
            }
        }
    }

    if (descriptor.IsChanged(Fields::IsEnabled))
    {
        setEnabled(GetFieldValue<bool>(Fields::IsEnabled, true));
    }

    if (descriptor.IsChanged(Fields::PlaceHolder))
    {
        setPlaceholderText(QString::fromStdString(GetFieldValue<String>(Fields::PlaceHolder, "")));
    }

    if (descriptor.IsChanged(Fields::Clearable))
    {
        bool needClearButton = GetFieldValue<bool>(Fields::Clearable, false);
        setClearButtonEnabled(needClearButton);
    }
}

M::ValidationResult LineEdit::Validate(const Any& value) const
{
    RETURN_IF_MODEL_LOST(M::ValidationResult());
    Reflection field = model.GetField(GetFieldName(Fields::Text));
    DVASSERT(field.IsValid());

    M::ValidationResult r;
    r.state = M::ValidationResult::eState::Valid;

    const M::Validator* validator = GetFieldValue<const M::Validator*>(Fields::Validator, nullptr);
    if (validator != nullptr)
    {
        r = validator->Validate(value, field.GetValue());
    }

    if (r.state == Metas::ValidationResult::eState::Invalid)
    {
        return r;
    }

    validator = field.GetMeta<M::Validator>();
    if (validator != nullptr)
    {
        r = validator->Validate(value, field.GetValue());
    }

    return r;
}

void LineEdit::ShowHint(const QString& message)
{
    NotificationParams notifParams;
    notifParams.title = "Invalid value";
    notifParams.message.message = message.toStdString();
    notifParams.message.type = Result::RESULT_ERROR;
    controlParams.ui->ShowNotification(controlParams.wndKey, notifParams);
}

void LineEdit::TextChanged(const QString& newText)
{
    RETURN_IF_MODEL_LOST(void());
    if (!isReadOnly())
    {
        if (hasFocus() == true)
        {
            FastName immediateFieldName = GetFieldName(Fields::ImmediateText);
            if (immediateFieldName.IsValid())
            {
                AnyFn method = model.GetMethod(immediateFieldName.c_str());
                if (method.IsValid())
                {
                    method.Invoke(newText.toStdString());
                }
            }
        }
        else
        {
            EditingFinished();
        }
    }
}
} // namespace DAVA
