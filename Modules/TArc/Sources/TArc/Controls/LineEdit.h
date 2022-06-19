#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Core/ContextAccessor.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/Controls/ControlDescriptor.h"
#include "TArc/Controls/Private/ValidatorDelegate.h"

#include <Base/BaseTypes.h>
#include <Base/FastName.h>

#include <QLineEdit>
#include <QFlags>

class QPushButton;
namespace DAVA
{
class LineEdit final : public ControlProxyImpl<QLineEdit>, private ValidatorDelegate
{
    using TBase = ControlProxyImpl<QLineEdit>;

public:
    enum class Fields : uint32
    {
        Text,
        ImmediateText, // method<void(const DAVA::String& text)>
        PlaceHolder,
        IsReadOnly,
        IsEnabled,
        Validator, // const M::Validator*
        Clearable, // bool
        FieldCount
    };

    DECLARE_CONTROL_PARAMS(Fields);

    LineEdit(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    LineEdit(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

private:
    void UpdateControl(const ControlDescriptor& changedFields) override;

    void SetupControl();
    void EditingFinished();
    void TextChanged(const QString& newText);

    M::ValidationResult Validate(const Any& value) const override;
    void ShowHint(const QString& message) override;

private:
    QtConnections connections;
};
} // namespace DAVA
