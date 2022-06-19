#include "TArc/Controls/ScrollBar.h"

namespace DAVA
{
ScrollBar::ScrollBar(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QScrollBar>(params, ControlDescriptor(params.fields), wrappersProcessor, model, parent)
{
    SetupControl();
}

ScrollBar::ScrollBar(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QScrollBar>(params, ControlDescriptor(params.fields), accessor, model, parent)
{
    SetupControl();
}

void ScrollBar::SetupControl()
{
    connections.AddConnection(this, &QScrollBar::valueChanged, MakeFunction(this, &ScrollBar::OnValueChanged));
}

void ScrollBar::UpdateControl(const ControlDescriptor& changedFields)
{
    bool minChanged = changedFields.IsChanged(Fields::Minimum);
    bool maxChanged = changedFields.IsChanged(Fields::Maximum);

    //setMinimum will call setValue inside QScrollBar so disable valueChanged signal on sync
    connections.RemoveConnection(this, &QScrollBar::valueChanged);

    if (changedFields.IsChanged(Fields::Enabled))
    {
        bool enabled = GetFieldValue<bool>(Fields::Enabled, true);
        setEnabled(enabled);
    }

    if (minChanged)
    {
        int min = GetFieldValue<int>(Fields::Minimum, 0);
        setMinimum(min);
    }

    if (maxChanged)
    {
        int max = GetFieldValue<int>(Fields::Maximum, 0);
        setMaximum(max);
    }

    if (changedFields.IsChanged(Fields::PageStep))
    {
        int pageStep = GetFieldValue<int>(Fields::PageStep, 0);
        setPageStep(pageStep);
    }

    if (minChanged || maxChanged || changedFields.IsChanged(Fields::Value))
    {
        int value = GetFieldValue<int>(Fields::Value, 0);
        setValue(value);
    }

    if (changedFields.IsChanged(Fields::Orientation))
    {
        Qt::Orientation orientation = GetFieldValue<Qt::Orientation>(Fields::Orientation, Qt::Horizontal);
        setSizePolicy(orientation == Qt::Horizontal ? QSizePolicy::Expanding : QSizePolicy::Maximum,
                      orientation == Qt::Horizontal ? QSizePolicy::Maximum : QSizePolicy::Expanding);
        setOrientation(orientation);

        setMinimumSize(QSize(orientation == Qt::Horizontal ? 0 : sizeHint().width(),
                             orientation == Qt::Horizontal ? sizeHint().height() : 0));
    }

    if (changedFields.IsChanged(Fields::Visible))
    {
        setVisible(GetFieldValue<bool>(Fields::Visible, true));
    }
    connections.AddConnection(this, &QScrollBar::valueChanged, MakeFunction(this, &ScrollBar::OnValueChanged));
}

void ScrollBar::OnValueChanged(int value)
{
    if (isEnabled() == false)
    {
        return;
    }

    int currentValue = GetFieldValue<int>(Fields::Value, 0);
    if (value != currentValue)
    {
        wrapper.SetFieldValue(GetFieldName(Fields::Value), value);
    }
}
} // namespace DAVA
