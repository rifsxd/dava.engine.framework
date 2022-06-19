#pragma once

#include "Classes/Modules/PackageModule/Private/PackageWidget.h"

#include <TArc/DataProcessing/TArcDataNode.h>

class PackageNode;
class QAction;

class PackageData : public DAVA::TArcDataNode
{
public:
private:
    friend class PackageModule;

    QAction* importPackageAction = nullptr;

    QAction* copyAction = nullptr;
    QAction* pasteAction = nullptr;
    QAction* cutAction = nullptr;

    QAction* deleteAction = nullptr;
    QAction* duplicateAction = nullptr;

    QAction* moveUpAction = nullptr;
    QAction* moveDownAction = nullptr;
    QAction* moveLeftAction = nullptr;
    QAction* moveRightAction = nullptr;

    QAction* jumpToPrototypeAction = nullptr;
    QAction* findPrototypeInstancesAction = nullptr;

    PackageWidget* packageWidget = nullptr;
    DAVA::Map<PackageNode*, PackageContext> packageWidgetContexts;

    DAVA_VIRTUAL_REFLECTION(PackageData, DAVA::TArcDataNode);
};
