#pragma once

#include <NetworkHelpers/ChannelListenerDispatched.h>

#include <Network/PeerDesription.h>
#include <Network/NetService.h>
#include <Network/IChannel.h>

#include <QObject>
#include <QPointer>
#include <QString>

class LogWidget;

namespace DAVA
{
class ContextAccessor;
class UI;
}

class DeviceLogController : public QObject,
                            public DAVA::Net::NetService,
                            public std::enable_shared_from_this<DeviceLogController>
{
    Q_OBJECT

public:
    explicit DeviceLogController(DAVA::UI* ui, const DAVA::Net::PeerDescription& peerDescr, QWidget* parentWidget, QObject* parent = NULL);
    ~DeviceLogController();
    void Init();

    void ShowView();

    virtual void ChannelOpen();
    virtual void ChannelClosed(const DAVA::char8* message);
    virtual void PacketReceived(const void* packet, size_t length);

    DAVA::Net::IChannelListener* GetAsyncChannelListener()
    {
        return channelListenerDispatched.get();
    }

private:
    void Output(const DAVA::String& msg);
    static LogWidget* GetOrCreateLogView(const QString& title, QWidget* parentWidget, DAVA::UI* ui);

private:
    static QMap<QString, LogWidget*> views;
    QPointer<LogWidget> view;
    QPointer<QWidget> parentWidget;
    DAVA::Net::PeerDescription peer;
    std::unique_ptr<DAVA::Net::ChannelListenerDispatched> channelListenerDispatched;
    DAVA::UI* ui = nullptr;
};
