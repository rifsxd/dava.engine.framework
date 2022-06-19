#include "TArc/Controls/FilePathEdit.h"
#include "TArc/Controls/Private/TextValidator.h"

#include <Base/FastName.h>
#include <Utils/StringFormat.h>
#include <Reflection/ReflectedMeta.h>

#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>

namespace DAVA
{
FilePathEdit::FilePathEdit(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QWidget>(params, ControlDescriptor(params.fields), wrappersProcessor, model, parent)
{
    SetupControl();
}

FilePathEdit::FilePathEdit(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QWidget>(params, ControlDescriptor(params.fields), accessor, model, parent)
{
    SetupControl();
}

void FilePathEdit::SetupControl()
{
    edit = new QLineEdit(this);
    edit->setObjectName("filePathEdit");
    setFocusProxy(edit);
    setFocusPolicy(edit->focusPolicy());

    button = new QToolButton(this);
    button->setAutoRaise(true);
    button->setIcon(QIcon(":/TArc/Resources/openfile.png"));
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    button->setObjectName("openFileDialogButton");

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(edit);
    layout->addWidget(button);

    connections.AddConnection(edit, &QLineEdit::editingFinished, MakeFunction(this, &FilePathEdit::EditingFinished));
    connections.AddConnection(button, &QToolButton::clicked, MakeFunction(this, &FilePathEdit::ButtonClicked));
}

void FilePathEdit::EditingFinished()
{
    RETURN_IF_MODEL_LOST(void());
    if (!edit->isReadOnly())
    {
        Any currenRawValue;
        const FastName& fieldName = GetFieldName(Fields::Value);
        if (fieldName.IsValid() == true)
        {
            Reflection field = model.GetField(fieldName);
            if (field.IsValid())
            {
                currenRawValue = field.GetValue();
            }
        }
        bool isValuesEqual = false;
        if (currenRawValue.CanCast<FilePath>())
        {
            isValuesEqual = FilePath(edit->text().toStdString()) == currenRawValue.Cast<FilePath>();
        }
        else if (currenRawValue.CanCast<String>())
        {
            isValuesEqual = edit->text().toStdString() == currenRawValue.Cast<String>();
        }
        else
        {
            DVASSERT(false);
        }

        if (isValuesEqual == false)
        {
            std::pair<FilePath, M::ValidationResult> pair = ConvertStringToPath(edit->text().toStdString());
            if (pair.second.state == M::ValidationResult::eState::Valid)
            {
                wrapper.SetFieldValue(GetFieldName(Fields::Value), pair.first);
                if (pair.first.GetStringValue() != pair.first.GetAbsolutePathname())
                {
                    edit->setText(QString::fromStdString(pair.first.GetAbsolutePathname()));
                }
            }
            else
            {
                Reflection r = model.GetField(GetFieldName(Fields::Value));
                DVASSERT(r.IsValid());
                UpdateControlValue(r.GetValue());
            }
        }
    }
}

void FilePathEdit::ButtonClicked()
{
    RETURN_IF_MODEL_LOST(void());
    if (!edit->isReadOnly())
    {
        QString path;
        if (IsFile() == true)
        {
            FileDialogParams params = GetFileDialogParams();
            path = controlParams.ui->GetOpenFileName(controlParams.wndKey, params);
        }
        else
        {
            DirectoryDialogParams params;
            params.dir = edit->text();
            params.title = QString::fromStdString("Open directory");
            path = controlParams.ui->GetExistingDirectory(controlParams.wndKey, params);
        }

        if (path.isEmpty() == false)
        {
            std::pair<FilePath, M::ValidationResult> pair = ConvertStringToPath(path.toStdString());
            if (pair.second.state == M::ValidationResult::eState::Valid)
            {
                edit->setText(QString::fromStdString(pair.first.GetAbsolutePathname()));
                EditingFinished();
            }
        }
    }
}

void FilePathEdit::UpdateControl(const ControlDescriptor& descriptor)
{
    RETURN_IF_MODEL_LOST(void());
    bool readOnlyChanged = descriptor.IsChanged(Fields::IsReadOnly);
    bool textChanged = descriptor.IsChanged(Fields::Value);
    if (readOnlyChanged || textChanged)
    {
        edit->setReadOnly(IsValueReadOnly(descriptor, Fields::Value, Fields::IsReadOnly));

        if (textChanged)
        {
            Reflection fieldValue = model.GetField(descriptor.GetName(Fields::Value));
            DVASSERT(fieldValue.IsValid());
            Any value = fieldValue.GetValue();
            UpdateControlValue(value);
        }
    }

    if (descriptor.IsChanged(Fields::IsEnabled))
    {
        edit->setEnabled(GetFieldValue<bool>(Fields::IsEnabled, true));
    }

    if (descriptor.IsChanged(Fields::PlaceHolder))
    {
        edit->setPlaceholderText(QString::fromStdString(GetFieldValue<String>(Fields::PlaceHolder, "")));
    }

    button->setEnabled(edit->isReadOnly() == false && edit->isEnabled() == true);
}

std::pair<FilePath, M::ValidationResult> FilePathEdit::ConvertStringToPath(const String& pathStr)
{
    FilePath path(pathStr);
    if (IsFile() == false && path.IsEmpty() == false)
    {
        path.MakeDirectoryPathname();
    }

    M::ValidationResult result = Validate(path);
    ProcessValidationResult(result, path);

    return std::make_pair(path, result);
}

M::ValidationResult FilePathEdit::Validate(const Any& value) const
{
    RETURN_IF_MODEL_LOST(M::ValidationResult());
    Reflection field = model.GetField(GetFieldName(Fields::Value));
    DVASSERT(field.IsValid());

    const M::Validator* validator = field.GetMeta<M::Validator>();
    if (validator != nullptr)
    {
        return validator->Validate(value, field.GetValue());
    }

    M::ValidationResult r;
    r.state = M::ValidationResult::eState::Valid;
    return r;
}

bool FilePathEdit::IsFile() const
{
    Reflection r = model.GetField(GetFieldName(Fields::Value));
    DVASSERT(r.IsValid());
    return r.GetMeta<M::Directory>() == nullptr;
}

FileDialogParams FilePathEdit::GetFileDialogParams() const
{
    FileDialogParams params;
    Reflection r = model.GetField(GetFieldName(Fields::Value));
    DVASSERT(r.IsValid());
    const M::File* file = r.GetMeta<M::File>();
    if (file != nullptr)
    {
        params.dir = QString::fromStdString(file->GetDefaultPath());
        if (!edit->text().isEmpty())
        {
            params.dir = edit->text();
        }
        params.filters = QString::fromStdString(file->filters);
        params.title = QString::fromStdString(file->dlgTitle);
    }
    else
    {
        params.dir = edit->text();
        params.filters = "All(*.*)";
        params.title = "Open File";
    }

    return params;
}

void FilePathEdit::ProcessValidationResult(M::ValidationResult& validationResult, FilePath& path)
{
    if (validationResult.state == M::ValidationResult::eState::Valid)
    {
        return;
    }

    if (validationResult.message.empty() == false)
    {
        ShowHint(QString::fromStdString(validationResult.message));
    }

    if (validationResult.fixedValue.IsEmpty() == false)
    {
        path = validationResult.fixedValue.Cast<FilePath>();
        validationResult.state = M::ValidationResult::eState::Valid;
    }
}

void FilePathEdit::ShowHint(const QString& message)
{
    NotificationParams notif;
    notif.title = "Incorrect value";
    notif.message.type = Result::RESULT_ERROR;
    notif.message.message = message.toStdString();
    controlParams.ui->ShowNotification(controlParams.wndKey, notif);
}

void FilePathEdit::UpdateControlValue(const Any& value)
{
    if (value.CanGet<FilePath>())
    {
        edit->setText(QString::fromStdString(value.Get<FilePath>().GetAbsolutePathname()));
    }
    else if (value.CanCast<String>())
    {
        edit->setText(QString::fromStdString(value.Cast<String>()));
    }
}
} // namespace DAVA
