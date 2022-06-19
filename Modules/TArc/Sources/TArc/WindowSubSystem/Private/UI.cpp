#include "TArc/WindowSubSystem/UI.h"
#include "TArc/WindowSubSystem/ActionUtils.h"
#include "TArc/WindowSubSystem/Private/QtEvents.h"
#include "TArc/Qt/QtString.h"
#include "TArc/Controls/ControlProxy.h"

#include <Engine/PlatformApiQt.h>

#include <QUrl>
#include <QList>
#include <QApplication>

namespace DAVA
{
const WindowKey mainWindowKey("MainWindow");

namespace MenuItems
{
const QString menuFile("File");
const QString menuEdit("Edit");
const QString menuView("View");
const QString menuHelp("Help");
const QString menuFind("Find");
}

WindowKey::WindowKey(const String& appID_)
    : appID(appID_)
{
}

const String& WindowKey::GetAppID() const
{
    return appID;
}

bool WindowKey::operator==(const WindowKey& other) const
{
    return appID == other.appID;
}

bool WindowKey::operator!=(const WindowKey& other) const
{
    return !(*this == other);
}

DockPanelInfo::DockPanelInfo()
    : actionPlacementInfo(CreateMenuPoint(QList<QString>() << MenuItems::menuView
                                                           << "Dock"))
{
}

void ShowOverCentralPanel(QWidget* view)
{
    QtOverlayWidgetVisibilityChange event(true);
    PlatformApi::Qt::GetApplication()->sendEvent(view, &event);
}

void HideOverCentralPanel(QWidget* view)
{
    QtOverlayWidgetVisibilityChange event(false);
    PlatformApi::Qt::GetApplication()->sendEvent(view, &event);
}

PanelKey::PanelKey(const QString& viewName_, const DockPanelInfo& info_)
    : PanelKey(DockPanel, viewName_, info_)
{
}

PanelKey::PanelKey(const QString& viewName_, const CentralPanelInfo& info_)
    : PanelKey(CentralPanel, viewName_, info_)
{
}

PanelKey::PanelKey(Type t, const QString& viewName_, const Any& info_)
    : viewName(viewName_)
    , type(t)
    , info(info_)
{
}

PanelKey::PanelKey(const QString& viewName, const OverCentralPanelInfo& info)
    : PanelKey(OverCentralPanel, viewName, info)
{
}

const QString& PanelKey::GetViewName() const
{
    return viewName;
}

PanelKey::Type PanelKey::GetType() const
{
    return type;
}

const Any& PanelKey::GetInfo() const
{
    return info;
}

ActionPlacementInfo::ActionPlacementInfo(const QUrl& url)
{
    AddPlacementPoint(url);
}

void ActionPlacementInfo::AddPlacementPoint(const QUrl& url)
{
    urls.emplace_back(url);
}

const Vector<QUrl>& ActionPlacementInfo::GetUrls() const
{
    return urls;
}

void UI::AddControlView(const WindowKey& windowKey, const PanelKey& panelKey, ControlProxy* widget)
{
    AddView(windowKey, panelKey, widget->ToWidgetCast());
}
} // namespace DAVA
