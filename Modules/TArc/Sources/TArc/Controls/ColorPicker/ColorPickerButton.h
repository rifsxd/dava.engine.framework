#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Controls/ControlDescriptor.h"
#include "TArc/Core/ContextAccessor.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/WindowSubSystem/UI.h"

#include <Math/Color.h>

#include <QToolButton>

namespace DAVA
{
class ColorPickerButton : public ControlProxyImpl<QToolButton>
{
public:
    enum class Fields : uint32
    {
        Color,
        IntermediateColor,
        IsReadOnly,
        Range,
        FieldCount
    };

    DECLARE_CONTROL_PARAMS(Fields);

    ColorPickerButton(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    ColorPickerButton(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

private:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

    void OnColorChanging(Color colorValue, ColorPickerButton::Fields typeField);

    void UpdateControl(const ControlDescriptor& changedfields) override;
    void SetupControl();

    void ButtonReleased();

    const M::Range* rangeMeta = nullptr;
    Any cachedColor;
    bool readOnly = false;
    QtConnections connections;
};
} // namespace DAVA