#pragma once

#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"
#include "TArc/Utils/QtConnections.h"

#include <Reflection/Reflection.h>
#include <Base/BaseTypes.h>

#include <QPointer>
#include <QDialog>

namespace DAVA
{
class IFieldAccessor
{
public:
    virtual String GetFieldValue(const Any& v, const Reflection& field) const = 0;
    virtual Any CreateNewValue(const String& newFieldValue, const Any& propertyValue, M::ValidationResult& result) const = 0;
    virtual Any Parse(const String& strValue, M::ValidationResult& result) const = 0;
    virtual bool IsReadOnly() const
    {
        return false;
    }
    virtual bool OverridePropertyName(QString& name) const
    {
        return false;
    }
};

class DefaultFieldAccessor : public IFieldAccessor
{
public:
    String GetFieldValue(const Any& v, const Reflection& field) const override;
    Any CreateNewValue(const String& newFieldValue, const Any& propertyValue, M::ValidationResult& result) const override;
    Any Parse(const String& strValue, M::ValidationResult& result) const override;
};

class TextComponentValue : public BaseComponentValue, private M::Validator
{
public:
    TextComponentValue();
    TextComponentValue(std::unique_ptr<IFieldAccessor>&& accessor);
    ~TextComponentValue() override = default;

    QString GetPropertyName() const override;

protected:
    Any GetMultipleValue() const override;
    bool IsValidValueToSet(const Any& newValue, const Any& currentValue) const override;
    ControlProxy* CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) override;

    String GetText() const;
    void SetText(const String& text);

    bool IsReadOnly() const override;
    const M::Validator* GetValidator() const;

    M::ValidationResult Validate(const Any& value, const Any& prevValue) const override;

    DAVA_VIRTUAL_REFLECTION(TextComponentValue, BaseComponentValue);

private:
    std::unique_ptr<IFieldAccessor> accessor;
};

class MultiLineTextComponentValue : public TextComponentValue
{
protected:
    ControlProxy* CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) override;

    void OpenMultiLineEdit();

private:
    QtConnections connections;
};
}
