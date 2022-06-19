#include "RemoteTool/Private/DeviceListController.h"
#include "RemoteTool/Private/DeviceList/DeviceListWidget.h"
#include "RemoteTool/Private/DeviceLogController/DeviceLogController.h"
#include "RemoteTool/Private/MemoryTool/MemProfController.h"

#include <LoggerService/ServiceInfo.h>
#include <MemoryProfilerService/ServiceInfo.h>

#include <Logger/Logger.h>
#include <Base/TemplateHelpers.h>
#include <Network/NetworkCommon.h>
#include <Network/PeerDesription.h>
#include <Network/ServicesProvider.h>
#include <Utils/StringFormat.h>

#include <QDebug>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTreeView>
#include <QUuid>
#include <QMessageBox>
#include <QTimer>

DeviceListController::DeviceListController(DAVA::UI* ui, QObject* parent)
    : QObject(parent)
    , model(NULL)
    , loggerServiceCreatorAsync(DAVA::MakeFunction(this, &DeviceListController::CreateLogger), DAVA::Net::NetCore::Instance()->GetNetEventsDispatcher())
    , loggerServiceDeleterAsync(DAVA::MakeFunction(this, &DeviceListController::DeleteLogger), DAVA::Net::NetCore::Instance()->GetNetEventsDispatcher())
    , profilerServiceCreatorAsync(DAVA::MakeFunction(this, &DeviceListController::CreateMemProfiler), DAVA::Net::NetCore::Instance()->GetNetEventsDispatcher())
    , profilerServiceDeleterAsync(DAVA::MakeFunction(this, &DeviceListController::DeleteMemProfiler), DAVA::Net::NetCore::Instance()->GetNetEventsDispatcher())
    , ui(ui)
{
    using namespace DAVA;
    using namespace DAVA::Net;

    model = new QStandardItemModel(this);

    // Register network service for receiving logs from device
    NetCore::Instance()->RegisterService(LOG_SERVICE_ID, MakeFunction(&loggerServiceCreatorAsync, &ServiceCreatorDispatched::ServiceCreatorCall), MakeFunction(&loggerServiceDeleterAsync, &ServiceDeleterDispatched::ServiceDeleterCall), "Logger");
    NetCore::Instance()->RegisterService(MEMORY_PROFILER_SERVICE_ID, MakeFunction(&profilerServiceCreatorAsync, &ServiceCreatorDispatched::ServiceCreatorCall), MakeFunction(&profilerServiceDeleterAsync, &ServiceDeleterDispatched::ServiceDeleterCall), "Memory profiler");

    // Create controller for discovering remote devices
    CreateDiscovererController();
}

DeviceListController::~DeviceListController()
{
    // Block until all controllers are destroyed
    DAVA::Net::NetCore::Instance()->DestroyAllControllersBlocked();
    // We need to unregister services as we register them on window creation and duplicate services are not allowed
    DAVA::Net::NetCore::Instance()->UnregisterAllServices();
}

void DeviceListController::SetView(DeviceListWidget* _view)
{
    view = _view;
    view->ItemView()->setModel(model);

    connect(view, &DeviceListWidget::connectClicked, this, &DeviceListController::OnConnectButtonPressed);
    connect(view, &DeviceListWidget::disconnectClicked, this, &DeviceListController::OnDisconnectButtonPressed);
    connect(view, &DeviceListWidget::showLogClicked, this, &DeviceListController::OnShowLogButtonPressed);
    connect(view, &DeviceListWidget::deviceDiscoverClicked, this, &DeviceListController::OnDeviceDiscover);
}

void DeviceListController::ShowView()
{
    if (!view.isNull())
    {
        // Here code to show view if hidden or restore view if minimized or hidden by main window
        view->show();
        view->activateWindow();
        view->raise();
    }
}

void DeviceListController::OnDeviceDiscover(const QString& addr)
{
    using namespace DAVA;
    using namespace DAVA::Net;

    if (!NetCore::IsNetworkEnabled())
    {
        Logger::Warning("[DeviceListController] Network is disabled in this build");
        return;
    }

    // addr should be in form ipaddress:port, port is optional and by default is 9999

    std::pair<uint16, uint16> portsRange;

    bool valid = true;
    int delimeter = addr.indexOf(QChar(':'));
    if (delimeter > 0)
    {
        portsRange.first = addr.mid(delimeter + 1).toUShort(&valid);
        portsRange.second = portsRange.first;
    }
    else
    {
        portsRange = ServicesProvider::GetTcpPortsRange();
    }

    if (valid)
    {
        String ipaddrPart = addr.mid(0, delimeter).toStdString().c_str();
        IPAddress ipaddr = IPAddress::FromString(ipaddrPart);
        valid = !ipaddr.IsUnspecified();
        if (valid)
        {
            DiscoverOnRange(ipaddr, portsRange);
            return;
        }
    }

    if (!valid)
    {
        QMessageBox::critical(view, "Invalid input", "Expected input: xxx.xxx.xxx.xxx[:port]\n"
                                                     "   xxx.xxx.xxx.xxx - destination IP address\n"
                                                     "   port - optional port, default is 9998");
    }
}

void DeviceListController::CreateDiscovererController()
{
    using namespace DAVA::Net;
    Endpoint endpoint(NetCore::defaultAnnounceMulticastGroup, NetCore::DEFAULT_UDP_ANNOUNCE_PORT);
    discovererControllerId = NetCore::Instance()->CreateDiscoverer(endpoint, DAVA::MakeFunction(this, &DeviceListController::DiscoverCallback));
}

void DeviceListController::DestroyDiscovererController()
{
    if (discovererControllerId != DAVA::Net::NetCore::INVALID_TRACK_ID)
    {
        DAVA::Net::NetCore::Instance()->DestroyControllerBlocked(discovererControllerId);
        discovererControllerId = DAVA::Net::NetCore::INVALID_TRACK_ID;
    }
}

DAVA::Net::IChannelListener* DeviceListController::CreateLogger(DAVA::Net::ServiceID serviceId, void* context)
{
    // Service creator method is called each time when connection has been established
    // As network service was created when 'Connect' button has been pressed so here simply return
    // pointer to created service

    // Context holds index of discovered device
    int row = static_cast<int>(reinterpret_cast<intptr_t>(context));
    if (model != NULL && 0 <= row && row < model->rowCount())
    {
        QModelIndex index = model->index(row, 0);
        DeviceServices services = index.data(ROLE_PEER_SERVICES).value<DeviceServices>();
        return services.log->GetAsyncChannelListener();
    }
    return NULL;
}

void DeviceListController::DeleteLogger(DAVA::Net::IChannelListener*, void* context)
{
    // Service deleter method is called before connector is destroyed

    // Context holds index of discovered device
    int row = static_cast<int>(reinterpret_cast<intptr_t>(context));
    if (model != NULL && 0 <= row && row < model->rowCount())
    {
        QModelIndex index = model->index(row, 0);

        QStandardItem* item = model->itemFromIndex(index);
        if (item != NULL)
        {
            DeviceServices services = index.data(ROLE_PEER_SERVICES).value<DeviceServices>();
            services.log.reset();

            QVariant v;
            v.setValue(services);
            item->setData(v, ROLE_PEER_SERVICES);
        }
    }
}

DAVA::Net::IChannelListener* DeviceListController::CreateMemProfiler(DAVA::Net::ServiceID serviceId, void* context)
{
    int row = static_cast<int>(reinterpret_cast<intptr_t>(context));
    if (model != NULL && 0 <= row && row < model->rowCount())
    {
        QModelIndex index = model->index(row, 0);
        DeviceServices services = index.data(ROLE_PEER_SERVICES).value<DeviceServices>();
        return services.memprof->GetAsyncChannelListener();
    }
    return NULL;
}

void DeviceListController::DeleteMemProfiler(DAVA::Net::IChannelListener* obj, void* context)
{
    int row = static_cast<int>(reinterpret_cast<intptr_t>(context));
    if (model != NULL && 0 <= row && row < model->rowCount())
    {
        QModelIndex index = model->index(row, 0);

        QStandardItem* item = model->itemFromIndex(index);
        if (item != NULL)
        {
            DeviceServices services = index.data(ROLE_PEER_SERVICES).value<DeviceServices>();
            services.memprof.reset();

            QVariant v;
            v.setValue(services);
            item->setData(v, ROLE_PEER_SERVICES);
        }
    }
}

void DeviceListController::ConnectDeviceInternal(QModelIndex& index, size_t ifIndex)
{
    using namespace DAVA;
    using namespace DAVA::Net;

    // Check whether we have connection with device
    NetCore::TrackId trackId = static_cast<NetCore::TrackId>(index.data(ROLE_CONNECTION_ID).toULongLong());
    if (trackId != NetCore::INVALID_TRACK_ID)
    {
        DeviceServices services = index.data(ROLE_PEER_SERVICES).value<DeviceServices>();
        services.log->ShowView();
        return;
    }

    Endpoint endp = index.data(ROLE_SOURCE_ADDRESS).value<Endpoint>();
    PeerDescription peer = index.data(ROLE_PEER_DESCRIPTION).value<PeerDescription>();
    {
        IPAddress addr = endp.Address(); // Use IP address from multicast packets
        NetConfig config = peer.NetworkConfig().Mirror(addr);
        const Vector<uint32>& servIds = config.Services();

        // Check whether remote device is under memory profiler and increase read timeout
        // Else leave it zero to allow underlying network system to choose timeout itself
        bool deviceUnderMemoryProfiler = std::find(servIds.begin(), servIds.end(), MEMORY_PROFILER_SERVICE_ID) != servIds.end();
        uint32 readTimeout = deviceUnderMemoryProfiler ? 120 * 1000 : Net::DEFAULT_READ_TIMEOUT;

        QStandardItem* item = model->itemFromIndex(index);
        if (NULL == item)
            return;

        // Append prefix 'ACTIVE!' to distinguish active objects
        QString s = item->text();
        item->setText("ACTIVE! " + s);

        {
            DeviceServices services = index.data(ROLE_PEER_SERVICES).value<DeviceServices>();
            // Check whether remote device has corresponding services
            auto iterService = std::find(servIds.begin(), servIds.end(), LOG_SERVICE_ID);
            if (iterService != servIds.end())
            {
                services.log.reset(new DeviceLogController(ui, peer, view, this));
                services.log->Init();
            }
            iterService = std::find(servIds.begin(), servIds.end(), MEMORY_PROFILER_SERVICE_ID);
            if (iterService != servIds.end())
            {
                services.memprof.reset(new MemProfController(peer, view, this));
            }

            QVariant v;
            v.setValue(services);
            item->setData(v, ROLE_PEER_SERVICES);
        }

        trackId = NetCore::Instance()->CreateController(config, reinterpret_cast<void*>(static_cast<intptr_t>(index.row())), readTimeout);
        if (trackId != NetCore::INVALID_TRACK_ID)
        {
            // Update item's ROLE_CONNECTION_ID and ROLE_PEER_SERVICES
            item->setData(QVariant(static_cast<qulonglong>(trackId)), ROLE_CONNECTION_ID);
        }
    }
}

void DeviceListController::DisonnectDeviceInternal(QModelIndex& index)
{
    using namespace DAVA::Net;

    // Check whether we have connection with device
    NetCore::TrackId trackId = static_cast<NetCore::TrackId>(index.data(ROLE_CONNECTION_ID).toULongLong());
    if (NetCore::INVALID_TRACK_ID == trackId)
        return;

    // Cleare item's ROLE_CONNECTION_ID to
    QStandardItem* item = model->itemFromIndex(index);
    if (NULL == item)
        return;

    item->setData(QVariant(static_cast<qulonglong>(NetCore::INVALID_TRACK_ID)), ROLE_CONNECTION_ID);
    // And destroy controller related to remote device
    NetCore::Instance()->DestroyControllerBlocked(trackId);

    QString s = item->text();
    int pos = s.indexOf('!');
    if (pos > 0)
    {
        item->setText(s.mid(pos + 2));
    }
}

void DeviceListController::OnConnectButtonPressed()
{
    if (!DAVA::Net::NetCore::IsNetworkEnabled())
    {
        DAVA::Logger::Warning("[DeviceListController] Network is disabled in this build");
        return;
    }

    // 'Connect' button has been pressed
    QModelIndexList selection = view->ItemView()->selectionModel()->selectedRows();
    for (int i = 0; i < selection.size(); i++)
    {
        QModelIndex& index = selection[i];
        if (index.parent().isValid())
            continue;
        // Do actual connect to device which description is stored in item identified by index
        // and using device's first network interface
        ConnectDeviceInternal(index, 0);
    }
}

void DeviceListController::OnDisconnectButtonPressed()
{
    if (!DAVA::Net::NetCore::IsNetworkEnabled())
    {
        DAVA::Logger::Warning("[DeviceListController] Network is disabled in this build");
        return;
    }

    // 'Disconnect' button has been pressed
    QModelIndexList selection = view->ItemView()->selectionModel()->selectedRows();
    for (int i = 0; i < selection.size(); i++)
    {
        QModelIndex& index = selection[i];
        if (index.parent().isValid())
            continue;
        DisonnectDeviceInternal(index);
    }
}

void DeviceListController::OnShowLogButtonPressed()
{
    using namespace DAVA;
    using namespace DAVA::Net;

    if (!NetCore::IsNetworkEnabled())
    {
        Logger::Warning("[DeviceListController] Network is disabled in this build");
        return;
    }

    // 'Show log' button has been pressed
    QModelIndexList selection = view->ItemView()->selectionModel()->selectedRows();
    for (int i = 0; i < selection.size(); i++)
    {
        QModelIndex& index = selection[i];
        if (index.parent().isValid())
            continue;

        // If connection has been established then show log output window
        NetCore::TrackId trackId = static_cast<NetCore::TrackId>(index.data(ROLE_CONNECTION_ID).toULongLong());
        if (trackId != NetCore::INVALID_TRACK_ID)
        {
            DeviceServices services = index.data(ROLE_PEER_SERVICES).value<DeviceServices>();
            if (services.memprof)
            {
                services.memprof->ShowView();
            }
        }
    }
}

QStandardItem* DeviceListController::CreateDeviceItem(const DAVA::Net::Endpoint& endp, const DAVA::Net::PeerDescription& peerDescr)
{
    using namespace DAVA;
    using namespace DAVA::Net;

    // Item text in the form of <name> - <platform> - <ip address>
    // E.g., 9f5656fd - Android - 192.168.0.24
    const QString caption = QString("%1 - %2 - %3 - %4")
                            .arg(peerDescr.GetAppName().c_str())
                            .arg(peerDescr.GetDeviceName().c_str())
                            .arg(peerDescr.GetPlatformString().c_str())
                            .arg(endp.Address().ToString().c_str());
    QStandardItem* item = new QStandardItem();
    item->setText(caption);

    // Set item's properties:
    //  - empty connection id
    //  - zero active flag
    //  - endpoint from which device description has been obtained
    //  - obtained device description
    //  - empty network services
    item->setData(QVariant(static_cast<qulonglong>(NetCore::INVALID_TRACK_ID)), ROLE_CONNECTION_ID);
    {
        QVariant v;
        v.setValue(endp);
        item->setData(v, ROLE_SOURCE_ADDRESS);
    }
    {
        QVariant v;
        v.setValue(peerDescr);
        item->setData(v, ROLE_PEER_DESCRIPTION);
    }
    {
        QVariant v;
        v.setValue(DeviceServices());
        item->setData(v, ROLE_PEER_SERVICES);
    }

    {
        // Add subitem with text: <manufacturer> <model> <platform> <version>
        // E.g. Samsung SM-G900F Android 4.4.2
        const QString text = QString("%1 %2 %3 %4")
                             .arg(peerDescr.GetManufacturer().c_str())
                             .arg(peerDescr.GetModel().c_str())
                             .arg(peerDescr.GetPlatformString().c_str())
                             .arg(peerDescr.GetVersion().c_str());
        QStandardItem* subitem = new QStandardItem();
        subitem->setText(text);
        item->appendRow(subitem);
    }
    {
        // Add list of available network interfaces
        QStandardItem* top = new QStandardItem();
        top->setText("Available interfaces");
        item->appendRow(top);

        DVASSERT(false == peerDescr.NetworkInterfaces().empty());
        const Vector<IfAddress>& v = peerDescr.NetworkInterfaces();
        for (size_t i = 0, n = v.size(); i < n; ++i)
        {
            char8 sphys[30];
            const IfAddress::PhysAddress& phys = v[i].PhysicalAddress();
            Snprintf(sphys, COUNT_OF(sphys), "%02X:%02X:%02X:%02X:%02X:%02X"
                     ,
                     phys.data[0]
                     ,
                     phys.data[1]
                     ,
                     phys.data[2]
                     ,
                     phys.data[3]
                     ,
                     phys.data[4]
                     ,
                     phys.data[5]);
            const QString text = QString("IP=%1, MAC=%2")
                                 .arg(v[i].Address().ToString().c_str())
                                 .arg(sphys);
            QStandardItem* subitem = new QStandardItem();
            subitem->setText(text);
            top->appendRow(subitem);
        }
    }
    {
        // Add list of available transports
        QStandardItem* top = new QStandardItem();
        top->setText("Available transports");
        item->appendRow(top);

        const Vector<NetConfig::TransportConfig>& tr = peerDescr.NetworkConfig().Transports();
        for (size_t i = 0, n = tr.size(); i < n; ++i)
        {
            const char* str = "!!!";
            if (tr[i].type == TRANSPORT_TCP)
                str = "TCP";
            const QString text = QString("%1 - %2")
                                 .arg(str)
                                 .arg(tr[i].endpoint.ToString().c_str());
            QStandardItem* subitem = new QStandardItem();
            subitem->setText(text);
            top->appendRow(subitem);
        }
    }
    {
        // Add list of available services, here service name becomes useful
        QStandardItem* top = new QStandardItem();
        top->setText("Available services");
        item->appendRow(top);

        const Vector<uint32>& serv = peerDescr.NetworkConfig().Services();
        for (size_t i = 0, n = serv.size(); i < n; ++i)
        {
            const char8* name = NetCore::Instance()->ServiceName(serv[i]);
            const QString text = name != NULL ? QString(name)
                                                :
                                                QString("Unknown service %1").arg(serv[i]);
            QStandardItem* subitem = new QStandardItem();
            subitem->setText(text);
            top->appendRow(subitem);
        }
    }
    return item;
}

void DeviceListController::DiscoverCallback(size_t buflen, const void* buffer, const DAVA::Net::Endpoint& endpoint)
{
    // This method is called when announce packet has arrived

    DAVA::String appName;
    bool extracted = DAVA::Net::PeerDescription::ExtractAppName(buffer, buflen, appName);

    // Check whether device+app has been already announced
    if (extracted && !AlreadyInModel(endpoint, appName))
    {
        DAVA::Net::PeerDescription peer;
        if (peer.Deserialize(buffer, buflen) > 0)
        {
            DAVA::Logger::FrameworkDebug("Endpoint %s will be added to device list", endpoint.ToString().c_str());
            QStandardItem* item = CreateDeviceItem(endpoint, peer);
            model->appendRow(item);
            if (view)
            {
                QTreeView* treeView = view->ItemView();
                treeView->expand(item->index());
            }
        }
    }
}

bool DeviceListController::AlreadyInModel(const DAVA::Net::Endpoint& endp, const DAVA::String& appName) const
{
    for (int i = 0, n = model->rowCount(); i < n; ++i)
    {
        QVariant v = model->item(i)->data(ROLE_SOURCE_ADDRESS);
        QVariant p = model->item(i)->data(ROLE_PEER_DESCRIPTION);
        if (endp == v.value<DAVA::Net::Endpoint>() && appName == p.value<DAVA::Net::PeerDescription>().GetAppName())
        {
            return true;
        }
    }
    return false;
}

void DeviceListController::DiscoverOnRange(const DAVA::Net::IPAddress& addr, const std::pair<DAVA::uint16, DAVA::uint16>& range)
{
    ipAddr = addr;
    portsRange = range;
    currentPort = portsRange.first;
    waitingForStartIterations = 0;
    closingPreviousDiscoverIterations = 0;
    DiscoverOnCurrentPort();
}

void DeviceListController::DiscoverOnCurrentPort()
{
    using namespace DAVA;
    using namespace DAVA::Net;

    Endpoint endp(ipAddr, currentPort);

    NetCore::DiscoverStartResult startResult = NetCore::Instance()->TryDiscoverDevice(endp);
    switch (startResult)
    {
    case NetCore::CONTROLLER_NOT_CREATED:
    {
        Logger::Error("Discoverer was not created");
        return;
    }
    case NetCore::CONTROLLER_NOT_STARTED_YET:
    {
        static uint32 MAX_WAITING_FOR_START_ITERATIONS = 2;
        ++waitingForStartIterations;

        if (waitingForStartIterations >= MAX_WAITING_FOR_START_ITERATIONS)
        {
            Logger::Error("Controller is not started");
            return;
        }
        else
        {
            DiscoverOnCurrentPortLater();
            return;
        }
    }
    case NetCore::DISCOVER_STARTED:
    {
        DiscoverNext(START_LATER);
        return;
    }
    case NetCore::CLOSING_PREVIOUS_DISCOVER:
    {
        static uint32 MAX_CLOSING_PREVIOUS_DISCOVER_ITERATIONS = 2;
        ++closingPreviousDiscoverIterations;
        if (closingPreviousDiscoverIterations >= MAX_CLOSING_PREVIOUS_DISCOVER_ITERATIONS)
        {
            DAVA::Logger::FrameworkDebug("Restarting discoverer");
            DestroyDiscovererController();
            CreateDiscovererController();
            closingPreviousDiscoverIterations = 0;
            DiscoverOnCurrentPortLater();
            return;
        }
        else
        {
            DiscoverOnCurrentPortLater();
            return;
        }
    }
    default:
    {
        DVASSERT(false, Format("Unknown type: %u", startResult).c_str());
        return;
    }
    }
}

void DeviceListController::DiscoverOnCurrentPortLater()
{
    const int REATTEMPT_PERIOD_MSEC = 1500;
    QTimer::singleShot(REATTEMPT_PERIOD_MSEC, this, &DeviceListController::DiscoverOnCurrentPort);
}

void DeviceListController::DiscoverNext(DeviceListController::DiscoverStartParam param)
{
    ++currentPort;
    waitingForStartIterations = 0;
    closingPreviousDiscoverIterations = 0;
    if (currentPort <= portsRange.second)
    {
        if (param == START_IMMEDIATELY)
        {
            DiscoverOnCurrentPort();
        }
        else
        {
            DiscoverOnCurrentPortLater();
        }
    }
}
