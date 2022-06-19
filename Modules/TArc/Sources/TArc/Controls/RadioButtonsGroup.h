#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Controls/ControlDescriptor.h"
#include "TArc/Core/ContextAccessor.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/Controls/CommonStrings.h"

#include <QWidget>
#include <QButtonGroup>

namespace DAVA
{
class RadioButtonsGroup final : public ControlProxyImpl<QWidget>
{
public:
    enum class Fields : uint32
    {
        Enabled,
        Value,
        Enumerator,
        Orientation,
        FieldCount
    };

    DECLARE_CONTROL_PARAMS(Fields);
    RadioButtonsGroup(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    RadioButtonsGroup(const Params& params, Reflection model, QWidget* parent = nullptr);

private:
    void SetupControl();
    void UpdateControl(const ControlDescriptor& changedfields) override;
    void CreateItems(const Reflection& fieldValue, const Reflection& fieldEnumerator, Qt::Orientation orientation);
    void CreateItem(const String& value, const QVariant& data, Qt::Orientation orientation, int32 index);
    int SelectCurrentItem(const Reflection& fieldValue, const Reflection& fieldEnumerator);
    void OnButtonToggled(QAbstractButton* button);

    bool updateControlProceed = false;
    QtConnections connections;
    QButtonGroup buttonGroup;
};
} // namespace DAVA
