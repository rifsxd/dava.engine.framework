#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/Qt/QtIcon.h"
#include "TArc/Utils/QtConnections.h"

#include <Functional/Signal.h>
#include <Reflection/Reflection.h>

#include <QWidget>
#include <QPointer>
#include <QBrush>

class QPaintEvent;
namespace DAVA
{
class FilterWidget : public DAVA::ControlProxyImpl<QWidget>
{
    using TBase = DAVA::ControlProxyImpl<QWidget>;

public:
    enum Fields
    {
        Enabled,
        Inversed,
        Title,
        FieldCount
    };

    DECLARE_CONTROL_PARAMS(Fields);
    FilterWidget(const Params& params, DataWrappersProcessor& processor, Reflection model, QWidget* parent = nullptr);

    Signal<> requestRemoving;
    Signal<> updateRequire;

    void ResetModel(Reflection model);

protected:
    void paintEvent(QPaintEvent* e) override;
    void contextMenuEvent(QContextMenuEvent* e) override;

    void SetupControl();
    void UpdateControl(const ControlDescriptor& descriptor) override;

    QIcon GetEnableButtonIcon() const;
    QIcon GetInverseButtonIcon() const;
    QString GetTitle() const;
    void ToggleEnabling();
    void ToggleInversing();
    void RemoveFilter();

    void UpdateTitlePalette(bool isEnabled);

private:
    QtConnections connections;
    QPointer<QPushButton> titleButton = nullptr;
    QBrush disabledBrush;
    QBrush enabledBrush;

    DAVA_REFLECTION(FilterWidget);
};
} // namespace DAVA
