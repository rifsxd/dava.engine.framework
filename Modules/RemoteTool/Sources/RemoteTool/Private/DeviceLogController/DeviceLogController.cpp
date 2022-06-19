#include "RemoteTool/Private/DeviceLogController/DeviceLogController.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/WindowSubSystem/UI.h>

#include <QtTools/ConsoleWidget/LogWidget.h>
#include <QtTools/ConsoleWidget/LogModel.h>

#include <Network/NetCore.h>
#include <Utils/UTF8Utils.h>

QMap<QString, LogWidget*> DeviceLogController::views;

DeviceLogController::DeviceLogController(DAVA::UI* ui, const DAVA::Net::PeerDescription& peerDescr, QWidget* _parentWidget, QObject* parent)
    : QObject(parent)
    , parentWidget(_parentWidget)
    , peer(peerDescr)
    , ui(ui)
{
    ShowView();
}

DeviceLogController::~DeviceLogController()
{
}

void DeviceLogController::Init()
{
    channelListenerDispatched.reset(new DAVA::Net::ChannelListenerDispatched(shared_from_this(), DAVA::Net::NetCore::Instance()->GetNetEventsDispatcher()));
}

void DeviceLogController::ShowView()
{
    if (NULL == view)
    {
        const QString title = QString("%1 | %2 (%3 %4)")
                              .arg(peer.GetAppName().c_str())
                              .arg(peer.GetDeviceName().c_str())
                              .arg(peer.GetPlatformString().c_str())
                              .arg(peer.GetVersion().c_str());

        view = GetOrCreateLogView(title, parentWidget, ui);
    }
}

void DeviceLogController::ChannelOpen()
{
    Output("************* Connection open");
}

void DeviceLogController::ChannelClosed(const DAVA::char8* message)
{
    DAVA::String s("************ Connection closed: ");
    s += message;
    Output(s);
}

void DeviceLogController::PacketReceived(const void* packet, size_t length)
{
    DAVA::String msg(static_cast<const DAVA::char8*>(packet), length);
    Output(msg);
}

void DeviceLogController::Output(const DAVA::String& msg)
{
    // Temporal workaround to extract log level from message
    QStringList list = QString(msg.c_str()).split(" ");
    DAVA::Logger::eLogLevel ll = DAVA::Logger::LEVEL_WARNING;
    // Current message format: <date> <time> <level> <text>
    if (list.size() > 3)
    {
        if (list[2] == "framework")
            ll = DAVA::Logger::LEVEL_FRAMEWORK;
        else if (list[2] == "debug")
            ll = DAVA::Logger::LEVEL_DEBUG;
        else if (list[2] == "info")
            ll = DAVA::Logger::LEVEL_INFO;
        else if (list[2] == "warning")
            ll = DAVA::Logger::LEVEL_WARNING;
        else if (list[2] == "error")
            ll = DAVA::Logger::LEVEL_ERROR;
    }

    view->AddMessage(ll, msg.c_str());
}

LogWidget* DeviceLogController::GetOrCreateLogView(const QString& title, QWidget* parentWidget, DAVA::UI* ui)
{
    auto found = views.find(title);
    if (found != views.end())
    {
        return found.value();
    }
    else
    {
        LogWidget* view = new LogWidget(parentWidget);
        view->setWindowFlags(Qt::Window);
        view->setWindowTitle(title);

        DAVA::DockPanelInfo panelInfo;
        panelInfo.title = title;
        panelInfo.area = Qt::RightDockWidgetArea;
        panelInfo.ensureVisible = true;
        DAVA::PanelKey panelKey(title, panelInfo);
        ui->AddView(DAVA::mainWindowKey, panelKey, view);

        views.insert(title, view);
        return view;
    }
}
