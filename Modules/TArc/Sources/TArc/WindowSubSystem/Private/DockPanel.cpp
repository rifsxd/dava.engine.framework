#include "TArc/WindowSubSystem/Private/DockPanel.h"

#include <Functional/Function.h>
#include <QDebug>

namespace DAVA
{
DockPanel::DockPanel(const Params& params_, const QString& title, QWidget* parent)
    : QDockWidget(title, parent)
    , params(params_)
    , binder(params.accessor)
{
    Setup();
}

DockPanel::DockPanel(const Params& params_, QWidget* parent)
    : QDockWidget(parent)
    , params(params_)
    , binder(params.accessor)
{
    Setup();
}

void DockPanel::Setup()
{
    const FieldDescriptor& titleDescriptor = params.descriptors[DockPanelInfo::Fields::Title];
    if (titleDescriptor.IsEmpty() == false)
    {
        binder.BindField(titleDescriptor, MakeFunction(this, &DockPanel::OnTitleChanged));
    }

    const FieldDescriptor& activeStateDescr = params.descriptors[DockPanelInfo::Fields::IsActive];
    if (activeStateDescr.IsEmpty() == false)
    {
        binder.BindField(activeStateDescr, MakeFunction(this, &DockPanel::OnActiveStateChanged));
    }

    QObject::connect(this, &QDockWidget::visibilityChanged, [this](bool isVisible) {
        isActive = isVisible;
        SetValue(DockPanelInfo::Fields::IsActive, isVisible);
    });

    QObject::connect(this, &QWidget::windowTitleChanged, [this](const QString& title) {
        SetValue(DockPanelInfo::Fields::Title, title);
    });
}

void DockPanel::OnTitleChanged(const Any& v)
{
    if (v.CanCast<QString>())
    {
        QString newTitle = v.Cast<QString>();
        setWindowTitle(newTitle);
    }
    else
    {
        setWindowTitle("");
    }
}

void DockPanel::OnActiveStateChanged(const Any& v)
{
    if (v.CanCast<bool>())
    {
        bool activateWindow = v.Cast<bool>();
        if (activateWindow == true && isActive == false)
        {
            show();
            raise();
        }
    }
}

void DockPanel::SetValue(DockPanelInfo::Fields field, const Any& v)
{
    const FieldDescriptor& descr = params.descriptors[field];
    if (descr.IsEmpty())
    {
        return;
    }

    binder.SetValue(descr, v);
}
} // namespace DAVA
