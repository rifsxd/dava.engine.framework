#pragma once

#include <vector>

#include <NetworkHelpers/ServiceCreatorDispatched.h>
#include <NetworkHelpers/ServiceDeleterDispatched.h>

#include <Network/NetCore.h>
#include <Network/Base/Endpoint.h>
#include <Network/PeerDesription.h>

#include <QObject>
#include <QPointer>

class QStandardItemModel;
class QStandardItem;

class DeviceListWidget;
class DeviceLogController;
class MemProfController;

namespace DAVA
{
class ContextAccessor;
class UI;
}

// Struct that holds network services for remote device
// For now only one service - log receiver
struct DeviceServices
{
    std::shared_ptr<DeviceLogController> log;
    std::shared_ptr<MemProfController> memprof;
};

// Register types for use with QVariant
Q_DECLARE_METATYPE(DAVA::Net::Endpoint);
Q_DECLARE_METATYPE(DAVA::Net::PeerDescription);
Q_DECLARE_METATYPE(DeviceServices);

class DeviceListController : public QObject
{
    Q_OBJECT

    enum DeviceDataRole
    {
        // Roles for each item in QStandardItemModel
        ROLE_CONNECTION_ID = Qt::UserRole + 1, // Store NetCore::TrackId to track whether device is connected or no
        ROLE_SOURCE_ADDRESS, // Store endpoint announce has arrived from
        ROLE_PEER_DESCRIPTION, // Store device description recieved from announce
        ROLE_PEER_SERVICES // Store network services to communicate with remote device
    };

public:
    explicit DeviceListController(DAVA::UI* ui, QObject* parent = NULL);
    ~DeviceListController();

    void SetView(DeviceListWidget* view);
    void ShowView();

    // Method invoked when announce packet arrived
    void DiscoverCallback(size_t buflen, const void* buffer, const DAVA::Net::Endpoint& endpoint);

private slots:
    void OnConnectButtonPressed();
    void OnDisconnectButtonPressed();
    void OnShowLogButtonPressed();
    void OnDeviceDiscover(const QString& addr);

private:
    void CreateDiscovererController();
    void DestroyDiscovererController();

    void ConnectDeviceInternal(QModelIndex& index, size_t ifIndex);
    void DisonnectDeviceInternal(QModelIndex& index);

    // Methods to create and delete network services
    DAVA::Net::IChannelListener* CreateLogger(DAVA::Net::ServiceID serviceId, void* context);
    void DeleteLogger(DAVA::Net::IChannelListener*, void* context);

    DAVA::Net::IChannelListener* CreateMemProfiler(DAVA::Net::ServiceID serviceId, void* context);
    void DeleteMemProfiler(DAVA::Net::IChannelListener* obj, void* context);

    // Check whether device already has been discovered
    bool AlreadyInModel(const DAVA::Net::Endpoint& endp, const DAVA::String& appName) const;

    enum DiscoverStartParam
    {
        START_IMMEDIATELY,
        START_LATER
    };
    void DiscoverOnRange(const DAVA::Net::IPAddress& ipAddr, const std::pair<DAVA::uint16, DAVA::uint16>& portsRange);
    void DiscoverOnCurrentPort();
    void DiscoverOnCurrentPortLater();
    void DiscoverNext(DiscoverStartParam = START_IMMEDIATELY);

private:
    QPointer<QStandardItemModel> model;
    QPointer<DeviceListWidget> view;

    DAVA::Net::ServiceCreatorDispatched loggerServiceCreatorAsync;
    DAVA::Net::ServiceDeleterDispatched loggerServiceDeleterAsync;

    DAVA::Net::ServiceCreatorDispatched profilerServiceCreatorAsync;
    DAVA::Net::ServiceDeleterDispatched profilerServiceDeleterAsync;

    DAVA::Net::IPAddress ipAddr;
    std::pair<DAVA::uint16, DAVA::uint16> portsRange;
    uintptr_t discovererControllerId = DAVA::Net::NetCore::INVALID_TRACK_ID;
    DAVA::uint16 currentPort = 0;
    DAVA::uint32 waitingForStartIterations = 0;
    DAVA::uint32 closingPreviousDiscoverIterations = 0;
    DAVA::Net::NetCore::DiscoverStartResult previousStartResult = DAVA::Net::NetCore::DISCOVER_STARTED;

    DAVA::UI* ui = nullptr;

private:
    static QStandardItem* CreateDeviceItem(const DAVA::Net::Endpoint& endp, const DAVA::Net::PeerDescription& peerDescr);
};
