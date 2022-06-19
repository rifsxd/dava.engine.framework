#include "RemoteTool/RemoteToolModule.h"
#include "RemoteTool/Private/DeviceListController.h"
#include "RemoteTool/Private/DeviceList/DeviceListWidget.h"

#include <TArc/Core/FieldBinder.h>
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/WindowSubSystem/QtAction.h>

#include <Functional/Function.h>
#include <Reflection/ReflectionRegistrator.h>

#include <QMenu>

RemoteToolModule::~RemoteToolModule()
{
    DVASSERT(deviceListWidget == nullptr);
    if (deviceListWidget != nullptr)
    {
        delete deviceListWidget;
    }
}

void RemoteToolModule::PostInit()
{
    using namespace DAVA;

    // create menu bar action "Remote", insert before "Help"
    QMenu* menuRemote = new QMenu(QStringLiteral("Remote"), nullptr);
    ActionPlacementInfo menuRemotePlacement(CreateMenuPoint("", { DAVA::InsertionParams::eInsertionMethod::BeforeItem, MenuItems::menuHelp }));
    GetUI()->AddAction(mainWindowKey, menuRemotePlacement, menuRemote->menuAction());

    QtAction* toolAction = new QtAction(GetAccessor(), QString("Remote Inspect"));

    // create "Remote Inspect" action inside of "Remote"
    ActionPlacementInfo toolPlacement(CreateMenuPoint("Remote"));
    GetUI()->AddAction(DAVA::mainWindowKey, toolPlacement, toolAction);

    connections.AddConnection(toolAction, &QAction::triggered, DAVA::MakeFunction(this, &RemoteToolModule::Show));
}

void RemoteToolModule::Show()
{
    if (!deviceListController)
    {
        deviceListWidget = new DeviceListWidget();

        deviceListController = new DeviceListController(GetUI(), deviceListWidget);
        deviceListController->SetView(deviceListWidget);
    }
    deviceListController->ShowView();
}

void RemoteToolModule::OnWindowClosed(const DAVA::WindowKey& key)
{
    if (key == DAVA::mainWindowKey && deviceListWidget != nullptr)
    {
        delete deviceListWidget;
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(RemoteToolModule)
{
    DAVA::ReflectionRegistrator<RemoteToolModule>::Begin()
    .ConstructorByPointer()
    .End();
}
