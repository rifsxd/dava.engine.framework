#pragma once

#include "TArc/WindowSubSystem/UI.h"
#include "TArc/WindowSubSystem/ActionUtils.h"
#include "TArc/DataProcessing/Common.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/DataProcessing/PropertiesHolder.h"
#include "TArc/Qt/QtString.h"

#include <Base/Any.h>
#include <Base/BaseTypes.h>

#include <Base/FastName.h>
#include <Functional/Function.h>
#include <Functional/Signal.h>

#include <QList>

namespace DAVA
{
class KeyedArchive;
class ContextAccessor;

class RecentMenuItems
{
public:
    RecentMenuItems(RecentMenuItems& rmi) = delete;
    RecentMenuItems& operator=(RecentMenuItems& rmi) = delete;

    struct Params
    {
        Params(const WindowKey& windowKey_, ContextAccessor* accessor_, const String& propertiesItemKey);

        WindowKey windowKey;
        ContextAccessor* accessor = nullptr;
        UI* ui = nullptr;

        QString recentMenuName;
        ActionPlacementInfo recentMenuPlacementInfo;

        QList<QString> menuSubPath;
        Function<uint32()> getMaximumCount;
        const String propertiesItemKey;
        FieldDescriptor predicateFieldDescriptor;
        Function<Any(const Any&)> enablePredicate;
    };

    RecentMenuItems(Params&& params);

    void Add(const String& recent);
    Signal<String> actionTriggered;
    Vector<String> Get() const;
    void Truncate();

private:
    void InitMenuItems();
    void AddInternal(const String& recent);
    void RemoveMenuItems();

    QtConnections connections;

    Params params;
};
} // namespace DAVA
