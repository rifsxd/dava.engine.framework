#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Controls/CommonStrings.h"
#include "TArc/Utils/QtConnections.h"

#include <QScrollBar>

namespace DAVA
{
class ScrollBar : public ControlProxyImpl<QScrollBar>
{
public:
    enum Fields : uint32
    {
        Value,
        Enabled,
        Visible,
        Minimum,
        Maximum,
        PageStep,
        Orientation,
        FieldCount
    };

    DECLARE_CONTROL_PARAMS(Fields);
    ScrollBar(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    ScrollBar(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

private:
    void SetupControl();
    void UpdateControl(const ControlDescriptor& changedFields) override;

    void OnValueChanged(int val);

    QtConnections connections;
};
} // namespace DAVA
