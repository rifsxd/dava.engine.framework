#include "TArc/Models/RecentMenuItems.h"
#include "TArc/WindowSubSystem/QtAction.h"

#include <FileSystem/KeyedArchive.h>
#include <Utils/StringFormat.h>

#include <QMenu>
#include <QAction>

namespace DAVA
{
namespace RecentMenuItemsDetails
{
const String recentItemsKey = "recent items";
}

RecentMenuItems::RecentMenuItems(Params&& params_)
    : params(std::move(params_))
{
    InitMenuItems();
}

void RecentMenuItems::Add(const String& recent)
{
    RemoveMenuItems();
    AddInternal(recent);
    InitMenuItems();
}

void RecentMenuItems::RemoveMenuItems()
{
    PropertiesItem item = params.accessor->CreatePropertiesNode(params.propertiesItemKey);
    Vector<String> actions = item.Get<Vector<String>>(RecentMenuItemsDetails::recentItemsKey);

    for (const String& action : actions)
    {
        ActionPlacementInfo placement(CreateMenuPoint(params.menuSubPath));
        params.ui->RemoveAction(params.windowKey, placement, QString::fromStdString(action));
    }
}

void RecentMenuItems::InitMenuItems()
{
    Vector<String> pathList = Get();

    if (params.recentMenuName.isEmpty() == false)
    { // create menu for recent items
        QAction* recentMenu = new QAction(params.recentMenuName, nullptr);
        recentMenu->setEnabled(pathList.empty() == false);
        params.ui->AddAction(params.windowKey, params.recentMenuPlacementInfo, recentMenu);
    }

    for (const String& path : pathList)
    {
        if (path.empty())
        {
            continue;
        }

        QString pathQt = QString::fromStdString(path);
        QtAction* action = new QtAction(params.accessor, pathQt);
        if (params.enablePredicate)
        {
            action->SetStateUpdationFunction(QtAction::Enabled, params.predicateFieldDescriptor, params.enablePredicate);
        }

        connections.AddConnection(action, &QAction::triggered, [path, this]() {
            actionTriggered.Emit(path);
        },
                                  Qt::QueuedConnection);

        ActionPlacementInfo placement(CreateMenuPoint(params.menuSubPath));
        params.ui->AddAction(params.windowKey, placement, action);
    }
}

void RecentMenuItems::AddInternal(const String& recent)
{
    using namespace DAVA;
    Vector<String> vectorToSave = Get();

    FilePath filePath(recent);
    String stringToInsert = filePath.GetAbsolutePathname();

    //check present set to avoid duplicates
    vectorToSave.erase(std::remove(vectorToSave.begin(), vectorToSave.end(), stringToInsert), vectorToSave.end());
    vectorToSave.insert(vectorToSave.begin(), stringToInsert);

    uint32 recentFilesMaxCount = params.getMaximumCount();
    uint32 size = Min((uint32)vectorToSave.size(), recentFilesMaxCount);

    vectorToSave.resize(size);
    PropertiesItem item = params.accessor->CreatePropertiesNode(params.propertiesItemKey);
    item.Set(RecentMenuItemsDetails::recentItemsKey, vectorToSave);
}

Vector<String> RecentMenuItems::Get() const
{
    using namespace DAVA;
    PropertiesItem item = params.accessor->CreatePropertiesNode(params.propertiesItemKey);
    Vector<String> retVector = item.Get<Vector<String>>(RecentMenuItemsDetails::recentItemsKey);
    uint32 recentFilesMaxCount = params.getMaximumCount();
    uint32 size = Min(static_cast<uint32>(retVector.size()), recentFilesMaxCount);
    retVector.resize(size);
    return retVector;
}

void RecentMenuItems::Truncate()
{
    RemoveMenuItems();

    {
        Vector<String> truncatedRecentItems = Get();
        PropertiesItem item = params.accessor->CreatePropertiesNode(params.propertiesItemKey);
        item.Set(RecentMenuItemsDetails::recentItemsKey, truncatedRecentItems);
    }

    InitMenuItems();
}

RecentMenuItems::Params::Params(const WindowKey& windowKey_, ContextAccessor* accessor_, const String& propertiesItemKey_)
    : windowKey(windowKey_)
    , accessor(accessor_)
    , propertiesItemKey(propertiesItemKey_)
{
}
} // namespace DAVA
