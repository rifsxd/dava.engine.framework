#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Core/ContextAccessor.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/Controls/ControlDescriptor.h"

#include <QCheckBox>

namespace DAVA
{
class CheckBox final : public ControlProxyImpl<QCheckBox>
{
public:
    enum class Fields : uint32
    {
        Checked,
        IsReadOnly,
        TextHint,
        IsEnabled,
        FieldCount
    };

    DECLARE_CONTROL_PARAMS(Fields);
    CheckBox(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    CheckBox(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

private:
    void UpdateControl(const ControlDescriptor& changedfields) override;

    void SetupControl();
    void StateChanged(int newState);

    enum class eContainedDataType : uint8
    {
        TYPE_NONE = 0,
        TYPE_BOOL,
        TYPE_CHECK_STATE
    };

    eContainedDataType dataType = eContainedDataType::TYPE_NONE;
    QtConnections connections;
};
} // namespace DAVA
